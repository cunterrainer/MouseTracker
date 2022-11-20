#pragma once
#include <string>
#include <utility>
#include <Windows.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

#define IMGUI_WINDOW_FLAGS ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar

class Window
{
private:
	GLFWwindow* m_Window = nullptr;
	mutable bool m_Resized = true;
public:
	Window(int width = 1600, int height = 920, const char* title = "Mouse Tracker", GLFWmonitor* monitor = NULL, GLFWwindow* share = NULL);
	~Window();

	// loop
	inline bool IsOpen()     const { return !glfwWindowShouldClose(m_Window); }
	inline void Swap()       const { glfwSwapBuffers(m_Window);               }
	inline void Clear()      const { glClear(GL_COLOR_BUFFER_BIT);            }
	inline void PollEvents() const { glfwPollEvents();                        }
	inline void WaitEvents() const { glfwWaitEvents();                        }
	inline void StartFrame() const { Clear(); ImGuiStartFrame();              }
	inline void EndFrame()   const { ImGuiRender(); PollEvents(); Swap();     }

	ImVec2 GetSize() const;
	inline HWND GetNativeHandle() const { return glfwGetWin32Window(m_Window); }
	inline void SetResized()      const { m_Resized = true;                    }

	// ImGui
	void ImGuiInit(const char* iniFileName = nullptr) const;
	void ImGuiStartFrame() const;
	void ImGuiRender() const;
};

inline const Window& GetWindow()
{
	static const Window w;
	return w;
}

inline int MsgBoxError(LPCSTR message)   { return MessageBoxA(GetWindow().GetNativeHandle(), message, "Error", MB_OK | MB_ICONERROR | MB_APPLMODAL); }
inline int MsgBoxWarning(LPCSTR message) { return MessageBoxA(GetWindow().GetNativeHandle(), message, "Warning", MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2 | MB_APPLMODAL); }