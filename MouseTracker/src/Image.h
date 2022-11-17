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
private:
    constexpr int GetIndex(int x, int y) const
    {
        if (x < 0 || y < 0 || x >(m_Width - 1) || y >(m_Height - 1))
            return -1;
        return m_Width * y + x;
    }
public:
    Image(ImVec2 res) : m_Width(static_cast<int>(res.x)), m_Height(static_cast<int>(res.y)), m_Pixel(m_Width*m_Height)
    {
        m_Data = new unsigned char[m_Pixel];
        //int i;
        //m_Data = stbi_load("2.png", &m_Width, &m_Height, &i, 4);
        //assert(m_Data != NULL);
        std::memset(m_Data, 255, m_Pixel);
    }

    ~Image()
    {
        delete[] m_Data;
    }

    ImVec2 Resolution() const 
    { 
        return { static_cast<float>(m_Width), static_cast<float>(m_Height) }; 
    }


    void SetPixel(int x, int y, unsigned char c)
    {
        const int index = GetIndex(x, y);
        if(index != -1)
            m_Data[index] = c;
    }


    GLuint UploadTexture() const
    {
        // Create a OpenGL texture identifier
        GLuint image_texture;
        glGenTextures(1, &image_texture);
        glBindTexture(GL_TEXTURE_2D, image_texture);

        // Setup filtering parameters for display
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        //glTexImage2D(GL_TEXTURE_2D, 0, 4, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_Data);
        glTexImage2D(GL_TEXTURE_2D, 0, 1, m_Width, m_Height, 0, GL_RED, GL_UNSIGNED_BYTE, m_Data);
        return image_texture;
    }


    GLuint Update(GLuint image_texture, int x, int y, unsigned char c)
    {
        SetPixel(x, y, c);
        glBindTexture(GL_TEXTURE_2D, image_texture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Width, m_Height, GL_RED, GL_UNSIGNED_BYTE, m_Data);
        return image_texture;
    }
};