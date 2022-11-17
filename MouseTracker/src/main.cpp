#include <iostream>
#include <cstring>

#include "ImGui/imgui.h"

#include "Window.h"
#include "Monitor.h"
#include "Image.h"


void ImageWindow(ImVec2 wSize, GLuint glImage, const Image& image)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Image", (bool*)0, IMGUI_WINDOW_FLAGS);
    ImGui::SetWindowPos({ 0, wSize.y * (1.f / 4.f) });
    ImGui::SetWindowSize({ wSize.x, wSize.y * (3.f / 4.f)});

    const float y = image.Resolution().y / (wSize.y * (3.f / 4.f));
    const float xPos = (wSize.x - image.Resolution().x / y) / 2.f;

    const ImVec2 p = ImGui::GetCursorScreenPos();
    ImGui::SetCursorScreenPos(ImVec2(p.x + xPos, p.y));
    ImGui::Image((void*)(intptr_t)glImage, { image.Resolution().x / y, image.Resolution().y / y });
    ImGui::End();
    ImGui::PopStyleVar();
}


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

        ImageWindow(w.GetSize(), image, i);

        w.ImGuiRender();
        w.WaitEvents();
        w.Swap();
    }
}