#include <iostream>
#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#include "ImGui/imgui.h"
#include "stb/stb_image.h"

#include "Window.h"

GLuint UploadTexture(const unsigned char* data, int width, int height)
{
    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, 1, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
    return image_texture;
}


int main()
{
    Window w;

    unsigned char* data = new unsigned char[500 * 500];
    std::memset(data, 0, 500 * 500);

    int width = 500;
    int height = 500;
    GLuint image = UploadTexture(data, width, height);

    while (w.IsOpen())
    {
        w.Clear();
        w.ImGuiStartFrame();

        ImGui::Begin("Data");
        ImGui::Image((void*)(intptr_t)image, ImVec2((float)width, (float)height));
        ImGui::End();

        w.ImGuiRender();
        w.WaitEvents();
        w.Swap();
    }

    delete[] data;
}