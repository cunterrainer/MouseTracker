#include <Windows.h>

#include "GLFW/glfw3.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"

#include "Window.h"
#include "Arial.h"
#include "Log.h"

//public
Window::Window(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share)
{
    glfwSetErrorCallback([](int error, const char* description){ Err << "Glfw Error " << error << ": " << description << std::endl; });

    if (!glfwInit())
    {
        Err << "glfwInit() failed!" << std::endl;
        return;
    }
    Log << "glfwInit() succeeded!" << std::endl;

    // Create window with graphics context
    m_Window = glfwCreateWindow(width, height, title, monitor, share);
    if (m_Window == NULL)
    {
        Err << "glfwCreateWindow() failed! w: " << width << " h : " << height << " t : " << title << std::endl;
        return;
    }
    Log << "Created window w: " << width << " h: " << height << " t: " << title << std::endl;
        
    // center the window
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    glfwSetWindowPos(m_Window, (mode->width - width) / 2, (mode->height - height) / 2);

    glfwMakeContextCurrent(m_Window);
    glfwSwapInterval(1);
    glClearColor(0.27f, 0.27f, 0.27f, 1.0f);
    ImGuiInit();
}


Window::~Window()
{
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    Log << "Shutdown ImGui" << std::endl;

    glfwDestroyWindow(m_Window);
    glfwTerminate();
    Log << "Shutdown glfw" << std::endl;
}


ImVec2 Window::GetSize() const
{
    int width, height;
    glfwGetWindowSize(m_Window, &width, &height);
    return { static_cast<float>(width), static_cast<float>(height) };
}


void Window::ImGuiInit(const char* iniFileName) const
{
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromMemoryCompressedTTF(sg_ArialCompressedData, sg_ArialCompressedSize, 19);
    io.IniFilename = iniFileName;

    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    ImGui::StyleColorsDark();
    Log << "Initialised ImGui" << std::endl;
}


void Window::ImGuiStartFrame() const
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}


void Window::ImGuiRender() const
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}