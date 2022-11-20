#include <iostream>
#include <vector>
#include <Windows.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

#include "Monitor.h"
#include "Log.h"

void SetVideoMode(std::vector<MonitorInfo>& mInfo)
{
    size_t count = 0;
    GLFWmonitor** m = glfwGetMonitors((int*)&count);
    if (m == NULL || count != mInfo.size())
    {
        Err << "glfwGetMonitors() failed! count: " << count << " mInfo.size(): " << mInfo.size() << std::endl;
        mInfo.clear();
        return;
    }

    for (size_t i = 0; i < count; ++i)
    {
        for (MonitorInfo& info : mInfo)
        {
            const std::string glfwAdapterName(glfwGetWin32Adapter(m[i]));
            if (info.adapter != std::wstring(glfwAdapterName.begin(), glfwAdapterName.end()))
                continue;

            const GLFWvidmode* mode = glfwGetVideoMode(m[i]);
            if (mode == NULL)
            {
                Err << "{SetVideoMode()} glfwGetVideoMode() failed!" << std::endl;
                mInfo.clear();
                return;
            }
            info.w = mode->width;
            info.h = mode->height;
            glfwGetMonitorPos(m[i], &info.x, &info.y);
        }
    }
}


void SetAllMonitors(std::vector<MonitorInfo>& mInfo)
{
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    for (const MonitorInfo& i : mInfo)
    {
        x = i.x < x ? i.x : x;
        y = i.y < y ? i.y : y;

        const int gx = i.x + i.w;
        const int gy = i.y + i.h;
        w = gx > w ? gx : w;
        h = gy > h ? gy : h;
    }
    mInfo.push_back({ L"",L"All", x, y, w - x, h - y});
}


std::vector<MonitorInfo> QuerryMonitors()
{
    std::vector<DISPLAYCONFIG_PATH_INFO> paths;
    std::vector<DISPLAYCONFIG_MODE_INFO> modes;
    const UINT32 flags = QDC_ONLY_ACTIVE_PATHS;

    UINT32 pathCount;
    UINT32 modeCount;
    LONG isError = GetDisplayConfigBufferSizes(flags, &pathCount, &modeCount);
    if (isError != ERROR_SUCCESS)
    {
        Err << "{QuerryMonitors()} GetDisplayConfigBufferSizes() failed: " << isError << std::endl;
        return {};
    }

    // Allocate the path and mode arrays
    paths.resize(pathCount);
    modes.resize(modeCount);

    // Get all active paths and their modes
    isError = QueryDisplayConfig(flags, &pathCount, paths.data(), &modeCount, modes.data(), nullptr);
    if (isError != ERROR_SUCCESS)
    {
        Err << "{QuerryMonitors()} QueryDisplayConfig() failed: " << isError << std::endl;
        return {};
    }

    // The function may have returned fewer paths/modes than estimated
    paths.resize(pathCount);
    modes.resize(modeCount);

    // For each active path
    std::vector<MonitorInfo> monitors;
    for (size_t i = 0; i < paths.size(); i++)
    {
        MonitorInfo mInfo;

        DISPLAYCONFIG_TARGET_DEVICE_NAME targetName = {};
        targetName.header.adapterId = paths[i].targetInfo.adapterId;
        targetName.header.id = paths[i].targetInfo.id;
        targetName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
        targetName.header.size = sizeof(targetName);
        isError = DisplayConfigGetDeviceInfo(&targetName.header);
        if (isError != ERROR_SUCCESS)
        {
            Err << "{QuerryMonitors()} DisplayConfigGetDeviceInfo(targetName) failed: " << isError << std::endl;
            return {};
        }

        DISPLAYCONFIG_SOURCE_DEVICE_NAME sourceName = {};
        sourceName.header.adapterId = paths[i].sourceInfo.adapterId;
        sourceName.header.id = paths[i].sourceInfo.id;
        sourceName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
        sourceName.header.size = sizeof(sourceName);
        isError = DisplayConfigGetDeviceInfo(&sourceName.header);
        if (isError != ERROR_SUCCESS)
        {
            Err << "{QuerryMonitors()} DisplayConfigGetDeviceInfo(sourceName) failed: " << isError << std::endl;
            return {};
        }

        std::wstring monName = L"Generic PnP Monitor";
        if (targetName.flags.friendlyNameFromEdid)
            monName = targetName.monitorFriendlyDeviceName;

        mInfo.name = monName;
        mInfo.adapter = sourceName.viewGdiDeviceName;
        monitors.push_back(mInfo);
    }
    return monitors;
}


std::vector<MonitorInfo> GetMonitors()
{
    std::vector<MonitorInfo> mInfo = QuerryMonitors();
    SetVideoMode(mInfo);
    if(mInfo.size() > 1)
        SetAllMonitors(mInfo);
    return mInfo;
}