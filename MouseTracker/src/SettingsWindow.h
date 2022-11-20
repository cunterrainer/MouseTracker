#pragma once
#include <filesystem>
#include <algorithm>
#include <optional>
#include <cstdlib>
#include <vector>
#include <string>
#include <vector>
#include <Windows.h>

#include "ImGui/imgui.h"
#include "nfd/nfd.h"

#include "Monitor.h"
#include "Window.h"
#include "Image.h"

#define CURSOR_POS(cPos, mPos) (cPos - mPos)

class SettingsWindow
{
private:
    bool m_Tracking = false;
    bool m_BigPixelMode = false;
    bool m_SleepWhileIdle = true;
    size_t m_SelectedMonitor = 0;
    Image& m_rImage;
private:
    static inline void PushStyleColors()
    {
        static constexpr ImVec4 dark{ 0.27f, 0.27f, 0.27f, 1.0f };
        static constexpr ImVec4 grey{ 0.57f, 0.57f, 0.57f, 1.0f };
        static constexpr ImVec4 lightGrey{ 0.70f, 0.70f, 0.70f, 1.0f };
        ImGui::PushStyleColor(ImGuiCol_Button, dark);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, grey);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, grey);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, dark);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, grey);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, grey);
        ImGui::PushStyleColor(ImGuiCol_Header, dark);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, grey);
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, dark);
        ImGui::PushStyleColor(ImGuiCol_CheckMark, lightGrey);
    }

    // not using glfw since it doesn't record key presses if window isn't focused
    static inline bool KeyPressed(int key)
    {
        return GetAsyncKeyState(key) & 0x01;
    }

    inline std::optional<std::filesystem::path> GetSavePath() const
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


    inline std::optional<std::string> GetImagePath() const
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


    inline void SaveImage(const Image& img) const
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


    inline void LoadImg(Image& img) const
    {
        std::optional<std::string> path = GetImagePath();
        if (!path.has_value())
            return;
        img.LoadFromFile(path.value());
    }


    inline std::string ConcatSelection(const std::vector<MonitorInfo>& mInfo) const
    {
        std::string selection;
        for (const MonitorInfo& i : mInfo)
        {
            std::string tmp;
            std::transform(i.name.begin(), i.name.end(), std::back_inserter(tmp), [](wchar_t c) { return (char)c; });
            selection += tmp + '\0';
        }
        return selection;
    }


    inline void TextLabels(POINT pos, const std::vector<MonitorInfo>& mInfo) const
    {
        const MonitorInfo& sm = mInfo[m_SelectedMonitor];
        ImGui::LabelText("Resolution", "%dx%d", sm.w, sm.h);
        ImGui::LabelText("Cursor position", "x=%ld y=%ld", CURSOR_POS(pos.x, sm.x), CURSOR_POS(pos.y, sm.y));
    }


    inline void RadioButtons()
    {
        if (ImGui::RadioButton("Sleep while idle [F7]", m_SleepWhileIdle) || KeyPressed(VK_F7))
            m_SleepWhileIdle = !m_SleepWhileIdle;

        if (ImGui::RadioButton("Big pixel mode [F8]", m_BigPixelMode) || KeyPressed(VK_F8))
            m_BigPixelMode = !m_BigPixelMode;

        if (m_Tracking)
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 230, 0, 255));
        else
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(230, 0, 0, 255));
        if (ImGui::RadioButton("Tracking [F9]", m_Tracking) || KeyPressed(VK_F9))
            m_Tracking = !m_Tracking;
        ImGui::PopStyleColor();
    }


    inline void Buttons() const
    {
        constexpr float saveImageBtnW = 104.f;
        if (ImGui::Button("Save image", { saveImageBtnW, 0.f }))
            SaveImage(m_rImage);

        constexpr float loadImageX = saveImageBtnW + 20.f; // arbitrary offset
        ImGui::SameLine(loadImageX);
        if (ImGui::Button("Load image"))
            LoadImg(m_rImage);

        ImGui::SameLine(loadImageX + ImGui::GetItemRectSize().x + 10); // arbitrary offset
        if (ImGui::Button("Reset image") && MsgBoxWarning("Do you really want to reset the tracking image? This change can't be undone!") == IDYES)
            m_rImage.Reset();
    }


    inline void MonitorSelectionCombo(const std::vector<MonitorInfo>& mInfo)
    {
        static const std::string selectionText = ConcatSelection(mInfo);

        const size_t prevMonitor = m_SelectedMonitor;
        if (!ImGui::Combo("##Monitor", (int*)&m_SelectedMonitor, selectionText.c_str()))
            return;

        const MonitorInfo& pm = mInfo[prevMonitor];
        const MonitorInfo& sm = mInfo[m_SelectedMonitor];
        if (pm.w == sm.w && pm.h == sm.h)
            return;

        // Image has to resized because resolutions have changed
        if (MsgBoxWarning("Changing the monitor will cause the image to be lost because the monitors have different resolutions! Do you want to continue?") != IDYES)
        {
            m_SelectedMonitor = prevMonitor;
            return;
        }
        ResizeImage(sm, mInfo);
    }


    inline void ResizeImage(const MonitorInfo& sm, const std::vector<MonitorInfo>& mInfo)
    {
        m_rImage.Resize(sm.w, sm.h);
        const size_t numMonitors = mInfo.size() - 1;
        if (m_SelectedMonitor != numMonitors)
            return;

        // 'All' was selected
        m_rImage.SetAllPixel(80); // Not monitor area on the image
        for (size_t i = 0; i < numMonitors; ++i)
        {
            m_rImage.SetPixelRange(CURSOR_POS(mInfo[i].x, mInfo.back().x), CURSOR_POS(mInfo[i].y, mInfo.back().y), mInfo[i].w, mInfo[i].h, 255);
        }
    }
public:
    inline explicit SettingsWindow(Image& img) : m_rImage(img) {}

    inline void Show(ImVec2 wSize, POINT pos, const std::vector<MonitorInfo>& mInfo)
    {
        PushStyleColors();
        ImGui::Begin("Settings", NULL, IMGUI_WINDOW_FLAGS);
        ImGui::SetWindowPos({ 0, 0 });
        ImGui::SetWindowSize({ wSize.x, wSize.y * (1.f / 4.f) });
        TextLabels(pos, mInfo);
        MonitorSelectionCombo(mInfo);
        RadioButtons();
        Buttons();
        ImGui::PopStyleColor(10);
        ImGui::End();
    }

    constexpr bool Tracking()          const { return m_Tracking;        }
    constexpr bool BigPixelMode()      const { return m_BigPixelMode;    }
    constexpr bool SleepWhileIdle()    const { return m_SleepWhileIdle;  }
    constexpr size_t SelectedMonitor() const { return m_SelectedMonitor; }
};