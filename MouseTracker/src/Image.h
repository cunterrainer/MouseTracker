#pragma once
#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#include "GLFW/glfw3.h"
#include "ImGui/imgui.h"
#include "stb/stb_image.h"

class Image
{
private:
    int m_Width;
    int m_Height;
    int m_Pixel;
    unsigned char* m_Data;
    GLuint m_GpuImage = 0;
private:
    constexpr int GetIndex(int x, int y) const
    {
        if (x < 0 || y < 0 || x >(m_Width - 1) || y >(m_Height - 1))
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
public:
    Image(ImVec2 res) : m_Width(static_cast<int>(res.x)), m_Height(static_cast<int>(res.y)), m_Pixel(m_Width*m_Height)
    {
        m_Data = new unsigned char[m_Pixel];
        std::memset(m_Data, 255, m_Pixel);
        m_GpuImage = GenerateTexture();
    }

    ~Image()
    {
        delete[] m_Data;
        glDeleteTextures(1, &m_GpuImage);
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
        {
            glBindTexture(GL_TEXTURE_2D, m_GpuImage);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Width, m_Height, GL_RED, GL_UNSIGNED_BYTE, m_Data);
        }
    }
};