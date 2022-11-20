#include <filesystem>
#include <iostream>
#include <optional>
#include <cstring>
#include <string>
#include <thread>
#include <chrono>
#include <Windows.h>

#include "ImGui/imgui.h"
#include "nfd/nfd.h"

#include "Window.h"
#include "Monitor.h"
#include "Clang.h"
#include "Image.h"
#include "Log.h"

#define CURSOR_POS(cPos, mPos) (cPos - mPos)

// not using glfw since it doesn't record key presses if window isn't focused
inline bool KeyPressed(int key)
{
    return GetAsyncKeyState(key) & 0x01;
}

void ImageWindow(ImVec2 wSize, const Image& image)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Image", (bool*)0, IMGUI_WINDOW_FLAGS);
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


std::optional<std::filesystem::path> GetSavePath()
{
    nfdchar_t* savePath = NULL;
    nfdresult_t result = NFD_SaveDialog("png", NULL, &savePath);
    if (result == NFD_OKAY)
    {
        const std::filesystem::path path = savePath;
        free(savePath);
        Log << "GetSavePath() path: [" << path << ']' << std::endl;
        return { path };
    }
    else if (result == NFD_CANCEL)
        Log << "GetSavePath() user pressed cancel" << std::endl;
    else
    {
        const std::string errorMsg = "Failed to open file [" + std::string(savePath) + "]";
        Err << "GetSavePath() " << errorMsg << std::endl;
        MsgBoxError(errorMsg.c_str());
    }
    return std::nullopt;
}


std::optional<std::string> GetImagePath()
{
    nfdchar_t* outPath = NULL;
    nfdresult_t result = NFD_OpenDialog("png,jpeg,jpg", NULL, &outPath);
    if (result == NFD_OKAY)
    {
        const std::string filePath = outPath;
        free(outPath);
        Log << "GetImagePath() path: [" << filePath << ']' << std::endl;
        return { filePath };
    }
    else if (result == NFD_CANCEL)
        Log << "GetImagePath() user pressed cancel" << std::endl;
    else
    {
        const std::string errorMsg = "Failed to open file [" + std::string(outPath) + "]";
        Err << "GetImagePath() " << errorMsg << std::endl;
        MsgBoxError(errorMsg.c_str());
    }
    return std::nullopt;
}


inline void SaveImage(const Image& img)
{
    const std::optional<std::filesystem::path> path = GetSavePath();
    if (!path.has_value())
        return;
    if (!img.WriteToFile(path.value()))
    {
        const std::string errorMsg = "Failed to write image [" + path.value().string() + "]";
        MsgBoxError(errorMsg.c_str());
    }
}


inline void LoadImg(Image& img)
{
    std::optional<std::string> path = GetImagePath();
    if (!path.has_value())
        return;
    img.LoadFromFile(path.value());
}


std::string ConcatSelection(const std::vector<MonitorInfo>& mInfo)
{
    std::string selection;
    for (const MonitorInfo& i : mInfo)
    {
        std::string tmp;
        std::transform(i.userFriendlyName.begin(), i.userFriendlyName.end(), std::back_inserter(tmp), [](wchar_t c) { return (char)c; });
        selection += tmp + '\0';
    }
    return selection;
}


void SettingsWindow(ImVec2 wSize, POINT pos, const std::vector<MonitorInfo>& mInfo, bool& tracking, bool& bigPixelMode, bool& sleepWhileIdle, Image& img, size_t& selectedMonitor)
{
    ImGui::Begin("Settings", (bool*)0, IMGUI_WINDOW_FLAGS);
    ImGui::PushStyleColor(ImGuiCol_Button, { 0.27f, 0.27f, 0.27f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.57f, 0.57f, 0.57f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.57f, 0.57f, 0.57f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_FrameBg, { 0.27f, 0.27f, 0.27f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, { 0.57f, 0.57f, 0.57f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_Header, { 0.27f, 0.27f, 0.27f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, { 0.57f, 0.57f, 0.57f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, { 0.27f, 0.27f, 0.27f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_CheckMark, { 0.7f,   0.7f,   0.7f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, { 0.57f, 0.57f, 0.57f, 1.0f });
    ImGui::SetWindowPos({ 0, 0 });
    ImGui::SetWindowSize({ wSize.x, wSize.y * (1.f / 4.f) });

    ImGui::LabelText("Resolution", "%dx%d", mInfo[selectedMonitor].w, mInfo[selectedMonitor].h);
    ImGui::LabelText("Cursor position", "x=%ld y=%ld", CURSOR_POS(pos.x, mInfo[selectedMonitor].x), CURSOR_POS(pos.y, mInfo[selectedMonitor].y));
    
    const size_t prevMonitor = selectedMonitor;
    static const std::string mSelection = ConcatSelection(mInfo);
    if (ImGui::Combo("##Monitor", (int*)&selectedMonitor, mSelection.c_str()))
    {
        const MonitorInfo& pm = mInfo[prevMonitor];
        const MonitorInfo& sm = mInfo[selectedMonitor];
        if (pm.w != sm.w || pm.h != sm.h)
        {
            if (MsgBoxWarning("Changing the monitor will cause the image to be lost because the monitors have different resolutions! Do you want to continue?") == IDYES)
            {
                img.Resize(sm.w, sm.h);
                if (selectedMonitor == mInfo.size() - 1)
                {
                    img.SetAllPixel(80);
                    for (size_t i = 0; i < mInfo.size() - 1; ++i)
                    {
                        img.SetPixelRange(CURSOR_POS(mInfo[i].x, mInfo.back().x), CURSOR_POS(mInfo[i].y, mInfo.back().y), mInfo[i].w, mInfo[i].h, 255);
                    }
                }
            }
            else
                selectedMonitor = prevMonitor;
        }
    }
    
    if (ImGui::RadioButton("Sleep while idle [F7]", sleepWhileIdle))
        sleepWhileIdle = !sleepWhileIdle;

    if (ImGui::RadioButton("Big pixel mode [F8]", bigPixelMode))
        bigPixelMode = !bigPixelMode;

    if(tracking)
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 230, 0, 255));
    else
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(230, 0, 0, 255));
    if (ImGui::RadioButton("Tracking [F9]", tracking))
        tracking = !tracking;
    ImGui::PopStyleColor();

    if (ImGui::Button("Save image", {104,0}))
        SaveImage(img);

    const float loadImageX = ImGui::GetItemRectSize().x + 20.f;
    ImGui::SameLine(loadImageX);
    if (ImGui::Button("Load image"))
        LoadImg(img);

    ImGui::SameLine(loadImageX + ImGui::GetItemRectSize().x + 10);
    if (ImGui::Button("Reset image") && MsgBoxWarning("Do you really want to reset the tracking image? This change can't be undone!") == IDYES)
        img.Reset();
    ImGui::PopStyleColor(10);
    ImGui::End();
}


int main()
{
    const Window& window = GetWindow();
    std::vector<MonitorInfo> mInfo = GetMonitors(); // mInfo[0] primary monitor
    if (mInfo.empty())
    {
        MessageBoxA(window.GetNativeHandle(), "Failed to load monitor data", "Error", MB_OK | MB_ICONERROR | MB_APPLMODAL);
        return 1;
    }
    Image i(mInfo[0].w, mInfo[0].h);


    POINT pos{ 0,0 };
    POINT prevPos{ 1,1 };
    bool tracking = false;
    bool bigPixelMode = false;
    bool sleepWhileIdle = true;
    size_t selectedMonitor = 0;
    auto startTime = std::chrono::high_resolution_clock::now();
    while (window.IsOpen())
    {
        window.Clear();
        window.ImGuiStartFrame();
        if (KeyPressed(VK_F9))
            tracking = !tracking;
        else if (KeyPressed(VK_F8))
            bigPixelMode = !bigPixelMode;
        else if (KeyPressed(VK_F7))
            sleepWhileIdle = !sleepWhileIdle;

        prevPos = pos;
        if (GetCursorPos(&pos) == 0)
        {
            Err << "GetCursorPos() error: " << GetLastError() << std::endl;
        }
        else if ((pos.x != prevPos.x || pos.y != prevPos.y) && tracking)
        {
            i.Update(CURSOR_POS(pos.x, mInfo[selectedMonitor].x), CURSOR_POS(pos.y, mInfo[selectedMonitor].y), bigPixelMode);
            startTime = std::chrono::high_resolution_clock::now();
        }
        else if(sleepWhileIdle && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count() > 200)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        ImageWindow(window.GetSize(), i);
        SettingsWindow(window.GetSize(), pos, mInfo, tracking, bigPixelMode, sleepWhileIdle, i, selectedMonitor);

        window.ImGuiRender();
        window.PollEvents();
        window.Swap();
    }
}