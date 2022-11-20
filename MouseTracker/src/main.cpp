#include <cstdlib>
#include <string>
#include <thread>
#include <chrono>
#include <Windows.h>

#include "ImGui/imgui.h"

#include "SettingsWindow.h"
#include "Window.h"
#include "Monitor.h"
#include "Clang.h"
#include "Image.h"
#include "Log.h"

inline void ImageWindow(ImVec2 wSize, const Image& image)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Image", NULL, IMGUI_WINDOW_FLAGS);
    ImGui::SetWindowPos({ 0, wSize.y * (1.f / 4.f) });
    ImGui::SetWindowSize({ wSize.x, wSize.y * (3.f / 4.f)});

    const float y = image.Resolution().y / (wSize.y * (3.f / 4.f));
    const float xPos = (wSize.x - image.Resolution().x / y) / 2.f;
    if (xPos >= 0.f)
    {
        const ImVec2 p = ImGui::GetCursorScreenPos();
        ImGui::SetCursorScreenPos(ImVec2(p.x + xPos, p.y));
        ImGui::Image((void*)(intptr_t)image.GetGpuImage(), { image.Resolution().x / y, image.Resolution().y / y });
    }
    else
    {
        const float x = image.Resolution().x / wSize.x;
        const float yPos = (wSize.y * (3.f / 4.f) - image.Resolution().y / x) / 2.f;

        const ImVec2 p = ImGui::GetCursorScreenPos();
        ImGui::SetCursorScreenPos(ImVec2(p.x, p.y + yPos));
        ImGui::Image((void*)(intptr_t)image.GetGpuImage(), { image.Resolution().x / x, image.Resolution().y / x });
    }

    ImGui::End();
    ImGui::PopStyleVar();
}


int main()
{
    const Window& window = GetWindow();
    std::vector<MonitorInfo> mInfo = GetMonitors(); // mInfo[0] primary monitor
    if (mInfo.empty())
        return MsgBoxError("Failed to load monitor data");
    Image i(mInfo[0].w, mInfo[0].h);

    POINT pos{0, 0};
    POINT prevPos{1, 1};
    auto startTime = std::chrono::high_resolution_clock::now();
    SettingsWindow sw(i);
    while (window.IsOpen())
    {
        window.StartFrame();
        prevPos = pos;
        if (GetCursorPos(&pos) == 0)
        {
            Err << "GetCursorPos() error: " << GetLastError() << std::endl;
        }
        else if (sw.Tracking() && (pos.x != prevPos.x || pos.y != prevPos.y))
        {
            i.Update(CURSOR_POS(pos.x, mInfo[sw.SelectedMonitor()].x), CURSOR_POS(pos.y, mInfo[sw.SelectedMonitor()].y), sw.BigPixelMode());
            startTime = std::chrono::high_resolution_clock::now();
        }
        else if(sw.SleepWhileIdle() && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count() > 200)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        ImageWindow(window.GetSize(), i);
        sw.Show(window.GetSize(), pos, mInfo);
        window.EndFrame();
    }
}