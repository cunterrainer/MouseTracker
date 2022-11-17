#pragma once
#include <iostream>

#include "GLFW/glfw3.h"
#include "ImGui/imgui.h"

class Monitor
{
private:
    int m_Width = 0;
    int m_Height = 0;
public:
    Monitor()
    {
        const GLFWvidmode* const mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        if (mode == NULL)
        {
            std::cerr << "Failed to retreive resolution\n";
            return;
        }
        m_Width = mode->width;
        m_Height = mode->height;
    }

    constexpr ImVec2 Resolution() const { return { static_cast<float>(m_Width), static_cast<float>(m_Height) }; }
    constexpr unsigned int Pixel() const { return m_Width * m_Height; }
    constexpr int Width() const { return m_Width; }
    constexpr int Height() const { return m_Height; }
    const char* Name() const { return glfwGetMonitorName(glfwGetPrimaryMonitor()); }
};

void GetMonitors();