#include <iostream>
#include <cstring>
#include <Windows.h>

#include "ImGui/imgui.h"

#include "Window.h"
#include "Monitor.h"
#include "Image.h"


void ImageWindow(ImVec2 wSize, const Image& image)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Image", (bool*)0, IMGUI_WINDOW_FLAGS);
    ImGui::SetWindowPos({ 0, wSize.y * (1.f / 4.f) });
    ImGui::SetWindowSize({ wSize.x, wSize.y * (3.f / 4.f)});

    const float y = image.Resolution().y / (wSize.y * (3.f / 4.f));
    const float xPos = (wSize.x - image.Resolution().x / y) / 2.f;

    const ImVec2 p = ImGui::GetCursorScreenPos();
    ImGui::SetCursorScreenPos(ImVec2(p.x + xPos, p.y));
    ImGui::Image((void*)(intptr_t)image.GetGpuImage(), {image.Resolution().x / y, image.Resolution().y / y});
    ImGui::End();
    ImGui::PopStyleVar();
}


void SettingsWindow(ImVec2 wSize, ImVec2 mRes, POINT pos, const char* mName)
{
    ImGui::Begin("Settings", (bool*)0, IMGUI_WINDOW_FLAGS);
    ImGui::SetWindowPos({ 0, 0 });
    ImGui::SetWindowSize({ wSize.x, wSize.y * (1.f / 4.f) });

    ImGui::LabelText("Resolution", "%ux%u", (unsigned int)mRes.x, (unsigned int)mRes.y);
    ImGui::LabelText("Cursor position", "x=%ld y=%ld", pos.x, pos.y);
    ImGui::LabelText("Monitor", "%s", mName);
    
    ImGui::End();
}


int main()
{
    Window w;
    Monitor m;
    Image i(m.Resolution());

    std::vector<MonitorInfo> mInfo = GetMonitors();
    for (auto c : mInfo)
    {
        std::wcout << c.instanceName << "\n" << c.manufacturerName << "\n" << c.productCodeId << "\n";
        std::wcout << c.serialNumberId << "\n" << c.userFriendlyName << "\n\n";
    }


    POINT pos;
    while (w.IsOpen())
    {
        w.Clear();
        w.ImGuiStartFrame();

        if (GetCursorPos(&pos) == 0)
        {
            std::cerr << "GetCursorPos() error: " << GetLastError() << std::endl;
        }
        else
        {
            i.Update(pos.x, pos.y, 0);
        }
        ImageWindow(w.GetSize(), i);
        SettingsWindow(w.GetSize(), m.Resolution(), pos, m.Name());

        w.ImGuiRender();
        w.PollEvents();
        w.Swap();
    }
}