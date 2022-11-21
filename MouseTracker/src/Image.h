#pragma once
#include <filesystem>
#include <algorithm>
#include <optional>
#include <cstring>
#include <string>
#include <new>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "GLFW/glfw3.h"
#include "ImGui/imgui.h"
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

#include "Log.h"

class Image
{
private:
    static constexpr int Channel = 4;
    int m_Width  = 0;
    int m_Height = 0;
    unsigned char* m_Data = nullptr;
    GLuint m_GpuImage = 0;
private:
    constexpr int GetIndex(int x, int y) const
    {
        if (x < 0 || y < 0 || x >= m_Width || y >= m_Height)
            return -1;
        return (m_Width * y + x) * Channel;
    }


    inline GLuint GenerateTexture() const
    {
        // Create a OpenGL texture identifier
        GLuint img;
        glGenTextures(1, &img);
        glBindTexture(GL_TEXTURE_2D, img);

        // Setup filtering parameters for display
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, Channel, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_Data);
        Log << "Generated opengl texture w: " << m_Width << " h: " << m_Height << " texture: " << img << std::endl;
        return img;
    }


    inline bool SetPixel(int x, int y, bool bigPixelMode)
    {
        auto SetDataAtIndex = [this](int a, int b)
        {
            const int index = GetIndex(a, b);
            if (index != -1)
            {
                m_Data[index] = 0;   // R
                m_Data[index+1] = 0; // G
                m_Data[index+2] = 0; // B
                return true;
            }
            return false;
        };

        const bool inRange = SetDataAtIndex(x, y);
        if (bigPixelMode && inRange)
        {
            SetDataAtIndex(x+1, y);
            SetDataAtIndex(x-1, y);
            SetDataAtIndex(x, y+1);
            SetDataAtIndex(x, y-1);
            SetDataAtIndex(x+1, y+1);
            SetDataAtIndex(x+1, y-1);
            SetDataAtIndex(x-1, y+1);
            SetDataAtIndex(x-1, y-1);
        }
        return inRange;
    }


    inline void DeleteTexture() const
    {
        if (m_GpuImage == 0 || m_Data == nullptr)
            return;

        delete[] m_Data;
        glDeleteTextures(1, &m_GpuImage);
        Log << "Deleted image texture: " << m_GpuImage << std::endl;
    }


    inline void UpdateGpu() const
    {
        glBindTexture(GL_TEXTURE_2D, m_GpuImage);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Width, m_Height, GL_RGBA, GL_UNSIGNED_BYTE, m_Data);
    }
public:
    inline Image(int width, int height)
    {
        Resize(width, height);
    }


    inline ~Image()
    {
        DeleteTexture();
    }


    inline std::optional<std::string> Resize(int width, int height)
    {
        DeleteTexture();
        const size_t pixel = (size_t)(width * height * Channel);
        m_Data = new (std::nothrow) unsigned char[pixel];
        if (m_Data == nullptr)
        {
            const std::string errorMsg = "Failed to allocate memory for the internal array!";
            Err << "{Image} " << errorMsg << std::endl;
            return { errorMsg };
        }
        std::memset(m_Data, 255, pixel);
        m_Width = width;
        m_Height = height;
        m_GpuImage = GenerateTexture();
        return std::nullopt;
    }


    inline ImVec2 Resolution() const
    { 
        return { static_cast<float>(m_Width), static_cast<float>(m_Height) }; 
    }


    constexpr GLuint GetGpuImage() const
    { 
        return m_GpuImage; 
    }


    inline void Update(int x, int y, bool bpm)
    {
        if (SetPixel(x, y, bpm))
            UpdateGpu();
    }


    inline int AlphaIsNeeded() const
    {
        for (size_t i = 3; i < (size_t)(m_Width * m_Height * Channel); i += 4)
        {
            if (m_Data[i] == 0)
                return true;
        }
        return false;
    }


    inline void RemoveGBAFromData(unsigned char* data) const
    {
        size_t mainDataIndex = 0;
        for (size_t i = 0; i < (size_t)(m_Width * m_Height); ++i)
        {
            data[i] = (m_Data[mainDataIndex] + m_Data[mainDataIndex + 1] + m_Data[mainDataIndex + 2]) / 3;
            mainDataIndex += 4;
        }
    }


    inline int SaveToFile(const char* path) const
    {
        const auto stbiWriteWAlphaFunc = [&](){ return stbi_write_png(path, m_Width, m_Height, Channel, m_Data, m_Width * Channel); };
        if (AlphaIsNeeded())
            return stbiWriteWAlphaFunc();

        unsigned char* data = new (std::nothrow) unsigned char[m_Width * m_Height];
        if (data == nullptr)
            return stbiWriteWAlphaFunc();

        RemoveGBAFromData(data);
        const int error = stbi_write_png(path, m_Width, m_Height, 1, data, m_Width);
        delete[] data;
        return error;
    }


    inline bool WriteToFile(const std::filesystem::path& path) const
    {
        std::string pathStr = path.string();
        std::string extension = path.extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) { return std::tolower(c); });
        if (extension != ".png")
            pathStr += ".png";

        if(SaveToFile(pathStr.c_str()) == 0)
        {
            Err << "Failed to write image w: " << m_Width << " h: " << m_Height << " [" << path << "]" << std::endl;
            return false;
        }
        Log << "Successfully wrote image w: " << m_Width << " h: " << m_Height << " [" << path << "]" << std::endl;
        return true;
    }


    inline std::optional<std::string> LoadFromFile(const std::string& path)
    {
        int width, height, cmp;
        unsigned char* data = stbi_load(path.data(), &width, &height, &cmp, Channel);
        if (data == NULL)
        {
            const std::string errorMsg = "Failed to load image [" + path + "]";
            Err << errorMsg << std::endl;
            return { errorMsg };
        }
        if (width != m_Width || height != m_Height)
        {
            stbi_image_free(data);
            std::string msg = "Couldn't load image since it doesn't match the monitors resolution!\nMonitor: ";
            msg += std::to_string(m_Width) + 'x' + std::to_string(m_Height) + "\nImage: ";
            msg += std::to_string(width) + 'x' + std::to_string(height);

            const std::string msgNl = msg;
            std::replace(msg.begin(), msg.end(), '\n', ' ');
            Err << msg << std::endl;
            return { msgNl };
        }

        std::memcpy(m_Data, data, (size_t)(m_Width * m_Height * Channel));
        stbi_image_free(data);
        UpdateGpu();
        Log << "Successfully loaded image from file w: " << m_Width << " h: " << m_Height << " [" << path << "]" << std::endl;
        return std::nullopt;
    }


    inline void Reset()
    {
        std::memset(m_Data, 255, (size_t)(m_Width * m_Height * Channel));
        UpdateGpu();
        Log << "{Image} Reset image w: " << m_Width << " h: " << m_Height << std::endl;
    }


    inline void SetAllPixel(int c)
    {
        std::memset(m_Data, c, (size_t)(m_Width * m_Height * Channel));
        UpdateGpu();
    }


    inline void SetPixelRange(int x, int y, int w, int h, unsigned char c)
    {
        for (int i = x; i < x+w; ++i)
        {
            for (int k = y; k < y+h; ++k)
            {
                const int index = GetIndex(i, k);
                if (index != -1)
                {
                    m_Data[index] = c;
                    m_Data[index+1] = c;
                    m_Data[index+2] = c;
                    m_Data[index+3] = c;
                }
            }
        }
        UpdateGpu();
    }
};