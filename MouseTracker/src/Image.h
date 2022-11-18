#pragma once
#include <filesystem>
#include <algorithm>
#include <cstring>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "GLFW/glfw3.h"
#include "ImGui/imgui.h"
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

class Image
{
private:
    int m_Width;
    int m_Height;
    unsigned char* m_Data;
    GLuint m_GpuImage = 0;
private:
    constexpr int GetIndex(int x, int y) const
    {
        if (x < 0 || y < 0 || x > (m_Width - 1) || y > (m_Height - 1))
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
        return img;
    }


    bool SetPixel(int x, int y, unsigned char c)
    {
        const int index = GetIndex(x, y);
        if (index != -1)
        {
            m_Data[index] = c;
            return true;
        }
        return false;
    }


    inline void DeleteTexture() const
    {
        delete[] m_Data;
        glDeleteTextures(1, &m_GpuImage);
    }


    inline void UpdateGpu() const
    {
        glBindTexture(GL_TEXTURE_2D, m_GpuImage);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Width, m_Height, GL_RED, GL_UNSIGNED_BYTE, m_Data);
    }
public:
    Image(ImVec2 res) : m_Width(static_cast<int>(res.x)), m_Height(static_cast<int>(res.y))
    {
        const size_t pixel = (size_t)(m_Height*m_Width);
        m_Data = new unsigned char[pixel];
        std::memset(m_Data, 255, pixel);
        m_GpuImage = GenerateTexture();
    }

    ~Image()
    {
        DeleteTexture();
    }

    ImVec2 Resolution() const 
    { 
        return { static_cast<float>(m_Width), static_cast<float>(m_Height) }; 
    }


    GLuint GetGpuImage() const 
    { 
        return m_GpuImage; 
    }


    void Update(int x, int y, unsigned char c)
    {
        if (SetPixel(x, y, c))
            UpdateGpu();
    }


    void WriteToFile(const std::filesystem::path& path) const
    {
        std::string pathStr = path.string();
        std::string extension = path.extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) { return std::tolower(c); });
        if (extension != ".png")
            pathStr += ".png";

        if (stbi_write_png(pathStr.c_str(), m_Width, m_Height, 1, m_Data, m_Width) == 0)
            std::cerr << "Failed to write image [" << path << "]\n";
    }


    void LoadFromFile(const std::string_view& path)
    {
        int cmp;
        unsigned char* data = stbi_load(path.data(), &m_Width, &m_Height, &cmp, 1);
        if (data == NULL)
        {
            std::cerr << "Failed to load image from file\n";
            return;
        }

        DeleteTexture();
        m_Data = data;
        m_GpuImage = GenerateTexture();
    }


    void Reset()
    {
        const size_t pixel = (size_t)(m_Height * m_Width);
        std::memset(m_Data, 255, pixel);
        UpdateGpu();
    }
};