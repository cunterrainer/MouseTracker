#pragma once
#include <filesystem>
#include <algorithm>
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
    int m_Width  = 0;
    int m_Height = 0;
    unsigned char* m_Data = nullptr;
    GLuint m_GpuImage = 0;
private:
    constexpr int GetIndex(int x, int y) const
    {
        if (x < 0 || y < 0 || x >= m_Width || y >= m_Height)
            return -1;
        return m_Width * y + x;
    }


    GLuint GenerateTexture() const
    {
        // Create a OpenGL texture identifier
        GLuint img;
        glGenTextures(1, &img);
        glBindTexture(GL_TEXTURE_2D, img);

        // Setup filtering parameters for display
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, 1, m_Width, m_Height, 0, GL_RED, GL_UNSIGNED_BYTE, m_Data);
        Log << "Generated opengl texture w: " << m_Width << " h: " << m_Height << " texture: " << img << std::endl;
        return img;
    }


    bool SetPixel(int x, int y, bool bigPixelMode)
    {
        auto SetDataAtIndex = [this](int a, int b)
        {
            const int index = GetIndex(a, b);
            if (index != -1)
            {
                m_Data[index] = 0;
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
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Width, m_Height, GL_RED, GL_UNSIGNED_BYTE, m_Data);
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


    inline void Resize(int width, int height)
    {
        DeleteTexture();
        const size_t pixel = (size_t)(width * height);
        m_Data = new (std::nothrow) unsigned char[pixel];
        if (m_Data == nullptr)
        {
            Err << "{Image} Failed to allocate memory for the internal array" << std::endl;
            MsgBoxError("Failed to allocate memory for the internal array!");
            return;
        }
        std::memset(m_Data, 255, pixel);
        m_Width = width;
        m_Height = height;
        m_GpuImage = GenerateTexture();
    }


    inline ImVec2 Resolution() const
    { 
        return { static_cast<float>(m_Width), static_cast<float>(m_Height) }; 
    }


    inline GLuint GetGpuImage() const
    { 
        return m_GpuImage; 
    }


    inline void Update(int x, int y, bool bpm)
    {
        if (m_Data != nullptr && SetPixel(x, y, bpm))
            UpdateGpu();
    }


    bool WriteToFile(const std::filesystem::path& path) const
    {
        std::string pathStr = path.string();
        std::string extension = path.extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) { return std::tolower(c); });
        if (extension != ".png")
            pathStr += ".png";

        if (stbi_write_png(pathStr.c_str(), m_Width, m_Height, 1, m_Data, m_Width) == 0)
        {
            Err << "Failed to write image w: " << m_Width << " h: " << m_Height << " [" << path << "]" << std::endl;
            return false;
        }
        Log << "Successfully wrote image w: " << m_Width << " h: " << m_Height << " [" << path << "]" << std::endl;
        return true;
    }


    void LoadFromFile(const std::string& path)
    {
        int width, height, cmp;
        unsigned char* data = stbi_load(path.data(), &width, &height, &cmp, 1);
        if (data == NULL)
        {
            const std::string errorMsg = "Failed to load image [" + path + "]";
            Err << errorMsg << std::endl;
            MsgBoxError(errorMsg.c_str());
            return;
        }
        if (width != m_Width || height != m_Height)
        {
            stbi_image_free(data);
            std::string msg = "Couldn't load image since it doesn't match the monitors resolution!\nMonitor: ";
            msg += std::to_string(m_Width) + 'x' + std::to_string(m_Height) + "\nImage: ";
            msg += std::to_string(width) + 'x' + std::to_string(height);

            MsgBoxError(msg.c_str());
            std::replace(msg.begin(), msg.end(), '\n', ' ');
            Err << msg << std::endl;
            return;
        }

        std::memcpy(m_Data, data, m_Width * m_Height);
        stbi_image_free(data);
        UpdateGpu();
        Log << "Successfully loaded image from file w: " << m_Width << " h: " << m_Height << " [" << path << "]" << std::endl;
    }


    inline void Reset()
    {
        const size_t pixel = (size_t)(m_Height * m_Width);
        std::memset(m_Data, 255, pixel);
        UpdateGpu();
        Log << "{Image} Reset image w: " << m_Width << " h: " << m_Height << std::endl;
    }
};