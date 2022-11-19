#include <filesystem>
#include <iostream>
#include <optional>
#include <cstring>
#include <string>
#include <Windows.h>

#include "ImGui/imgui.h"
#include "nfd/nfd.h"

#include "Window.h"
#include "Monitor.h"
#include "Image.h"
#include "Log.h"

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

    const ImVec2 p = ImGui::GetCursorScreenPos();
    ImGui::SetCursorScreenPos(ImVec2(p.x + xPos, p.y));
    ImGui::Image((void*)(intptr_t)image.GetGpuImage(), {image.Resolution().x / y, image.Resolution().y / y});
    ImGui::End();
    ImGui::PopStyleVar();
}


std::optional<std::filesystem::path> GetSavePath(HWND wHandle)
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
        MessageBoxA(wHandle, errorMsg.c_str(), "Error", MB_OK | MB_ICONERROR | MB_APPLMODAL);
    }
    return std::nullopt;
}


std::optional<std::string> GetImagePath(HWND wHandle)
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
        MessageBoxA(wHandle, errorMsg.c_str(), "Error", MB_OK | MB_ICONERROR | MB_APPLMODAL);
    }
    return std::nullopt;
}


inline void SaveImage(const Image& img, HWND wHandle)
{
    const std::optional<std::filesystem::path> path = GetSavePath(wHandle);
    if (!path.has_value())
        return;
    if (!img.WriteToFile(path.value()))
    {
        const std::string errorMsg = "Failed to write image [" + path.value().string() + "]";
        MessageBoxA(wHandle, errorMsg.c_str(), "Error", MB_OK | MB_ICONERROR | MB_APPLMODAL);
    }
}


inline void LoadImg(Image& img, HWND wHandle)
{
    std::optional<std::string> path = GetImagePath(wHandle);
    if (!path.has_value())
        return;
    if (!img.LoadFromFile(path.value()))
    {
        const std::string errorMsg = "Failed to load image [" + path.value() + "]";
        MessageBoxA(wHandle, errorMsg.c_str(), "Error", MB_OK | MB_ICONERROR | MB_APPLMODAL);
    }
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


void SettingsWindow(ImVec2 wSize, POINT pos, const std::vector<MonitorInfo>& mInfo, bool& tracking, bool& bigPixelMode, Image& img, HWND wHandle)
{
    ImGui::Begin("Settings", (bool*)0, IMGUI_WINDOW_FLAGS);
    ImGui::SetWindowPos({ 0, 0 });
    ImGui::SetWindowSize({ wSize.x, wSize.y * (1.f / 4.f) });

    static size_t selectedMonitor = 0;
    static std::string mSelection = ConcatSelection(mInfo);

    ImGui::LabelText("Resolution", "%dx%d", mInfo[selectedMonitor].w, mInfo[selectedMonitor].h);
    ImGui::LabelText("Cursor position", "x=%ld y=%ld", pos.x, pos.y);
    ImGui::Combo("##Monitor", (int*)&selectedMonitor, mSelection.c_str());
    
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
        SaveImage(img, wHandle);
    ImGui::SameLine(ImGui::GetItemRectSize().x + 20);
    if (ImGui::Button("Load image"))
        LoadImg(img, wHandle);

    if (ImGui::Button("Reset image") && MessageBoxW(wHandle, L"Do you really want to reset the tracking image? This change can't be undone!", L"Warning", MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2 | MB_APPLMODAL) == IDYES)
        img.Reset();
    ImGui::End();
}


int main()
{
    Window w;
    std::vector<MonitorInfo> mInfo = GetMonitors(); // mInfo[0] primary monitor
    if (mInfo.empty())
    {
        MessageBoxA(w.GetNativeHandle(), "Failed to load monitor data", "Error", MB_OK | MB_ICONERROR | MB_APPLMODAL);
        return 1;
    }
    Image i(mInfo[0].w, mInfo[0].h, w.GetNativeHandle());


    POINT pos{ 0,0 };
    POINT prevPos{ 1,1 };
    bool tracking = false;
    bool bigPixelMode = false;
    while (w.IsOpen())
    {
        w.Clear();
        w.ImGuiStartFrame();
        if (KeyPressed(VK_F9))
            tracking = !tracking;
        else if (KeyPressed(VK_F8))
            bigPixelMode = !bigPixelMode;

        prevPos = pos;
        if (GetCursorPos(&pos) == 0)
        {
            Err << "GetCursorPos() error: " << GetLastError() << std::endl;
        }
        else if ((pos.x != prevPos.x || pos.y != prevPos.y) && tracking)
        {
            i.Update(pos.x, pos.y, bigPixelMode);
        }
        ImageWindow(w.GetSize(), i);
        SettingsWindow(w.GetSize(), pos, mInfo, tracking, bigPixelMode, i, w.GetNativeHandle());

        w.ImGuiRender();
        w.PollEvents();
        w.Swap();
    }
}