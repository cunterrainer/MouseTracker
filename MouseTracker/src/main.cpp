#include <iostream>
#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#include "ImGui/imgui.h"
#include "stb/stb_image.h"

#include "Window.h"
#include "Monitor.h"
#include "Image.h"


int main()
{
    Window w;
    Monitor m;
    Image i(m.Resolution());

    GLuint image = i.UploadTexture();

    while (w.IsOpen())
    {
        w.Clear();
        w.ImGuiStartFrame();

        ImGui::Begin("Data");
        ImGui::Image((void*)(intptr_t)image, i.Resolution());
        ImGui::End();

        w.ImGuiRender();
        w.WaitEvents();
        w.Swap();
    }
}