#pragma once
#include <iostream>
#include <string>
#include <vector>

#include "GLFW/glfw3.h"
#include "ImGui/imgui.h"
#include "Log.h"

class Monitor
{
private:
    unsigned int m_Width = 0;
    unsigned int m_Height = 0;
public:
    Monitor()
    {
        const GLFWvidmode* const mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        if (mode == NULL)
        {
            Err << "{Monitor} glfwGetVideoMode() failed!" << std::endl;
            return;
        }
        Log << "{Monitor} glfwGetVideoMode() succeeded" << std::endl;
        m_Width = static_cast<unsigned int>(mode->width);
        m_Height = static_cast<unsigned int>(mode->height);
    }

    constexpr ImVec2 Resolution()   const { return { static_cast<float>(m_Width), static_cast<float>(m_Height) }; }
    constexpr unsigned int Pixel()  const { return m_Width * m_Height; }
    constexpr unsigned int Width()  const { return m_Width; }
    constexpr unsigned int Height() const { return m_Height; }
};


struct MonitorInfo
{
    std::wstring instanceName;
    std::wstring userFriendlyName;
    std::wstring manufacturerName;
    std::wstring productCodeId;
    std::wstring serialNumberId;
};
std::vector<MonitorInfo> GetMonitors();