#include <iostream>
#include <vector>
#include <Windows.h>
#include <tchar.h>
#include <initguid.h>
#include <wmistr.h>

#include "GLFW/glfw3.h"

#include "Monitor.h"
#include "Log.h"

DEFINE_GUID(WmiMonitorID_GUID, 0x671a8285, 0x4edb, 0x4cae, 0x99, 0xfe, 0x69, 0xa1, 0x5c, 0x48, 0xc0, 0xbc);
typedef struct WmiMonitorID {
    USHORT ProductCodeID[16];
    USHORT SerialNumberID[16];
    USHORT ManufacturerName[16];
    UCHAR WeekOfManufacture;
    USHORT YearOfManufacture;
    USHORT UserFriendlyNameLength;
    USHORT UserFriendlyName[1];
} WmiMonitorID, * PWmiMonitorID;
#define OFFSET_TO_PTR(Base, Offset) ((PBYTE)((PBYTE)Base + Offset))

typedef HRESULT(WINAPI* WOB) (IN LPGUID lpGUID, IN DWORD nAccess, OUT LONG*);
WOB WmiOpenBlock;
typedef HRESULT(WINAPI* WQAD) (IN LONG hWMIHandle, ULONG* nBufferSize, OUT UCHAR* pBuffer);
WQAD WmiQueryAllData;
typedef HRESULT(WINAPI* WCB) (IN LONG);
WCB WmiCloseBlock;


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
        const GLFWvidmode* mode = glfwGetVideoMode(m[i]);
        if (mode == NULL)
        {
            Err << "{SetVideoMode()} glfwGetVideoMode() failed!" << std::endl;
            mInfo.clear();
            return;
        }

        mInfo[i].w = mode->width;
        mInfo[i].h = mode->height;
        glfwGetMonitorPos(m[i], &mInfo[i].x, &mInfo[i].y);
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
    mInfo.push_back({ L"", L"All", L"", L"", L"", x, y, w-x, h-y });
}


std::vector<MonitorInfo> GetMonitors()
{
    // Load libraries
    HINSTANCE hDLL = LoadLibraryW(L"Advapi32.dll");
    if (hDLL == NULL)
    {
        Err << "Failed to load library Advapi32.dll" << std::endl;
        return {};
    }
    WmiOpenBlock = (WOB)GetProcAddress(hDLL, "WmiOpenBlock");
    WmiQueryAllData = (WQAD)GetProcAddress(hDLL, "WmiQueryAllDataW");
    WmiCloseBlock = (WCB)GetProcAddress(hDLL, "WmiCloseBlock");
    if (WmiOpenBlock == NULL || !WmiQueryAllData || !WmiCloseBlock)
    {
        Err << "Failed to load either WmiOpenBlock or WmiQueryAllData or WmiCloseBlock: " << GetLastError() << std::endl;
        return {};
    }


    LONG hWmiHandle = 0;
    HRESULT hr = WmiOpenBlock(const_cast<LPGUID>(&WmiMonitorID_GUID), GENERIC_READ, &hWmiHandle);
    if (hr != ERROR_SUCCESS)
    {
        Err << "WmiOpenBlock() failed: " << GetLastError() << std::endl;
        return {};
    }


    ULONG nBufferSize = 0;
    hr = WmiQueryAllData(hWmiHandle, &nBufferSize, NULL);
    if (hr != ERROR_INSUFFICIENT_BUFFER)
    {
        Err << "1) WmiQueryAllData() failed: " << GetLastError() << std::endl;
        return {};
    }


    UCHAR* pAllDataBuffer = (UCHAR*)malloc(nBufferSize);
    if (pAllDataBuffer == NULL)
    {
        Err << "{Monitor} Failed to allocate memory for WmiQueryAllData buffer" << std::endl;
        return {};
    }

    UCHAR* pAllDataBufferOriginal = pAllDataBuffer;
    hr = WmiQueryAllData(hWmiHandle, &nBufferSize, pAllDataBuffer);
    if (hr != ERROR_SUCCESS)
    {
        Err << "2) WmiQueryAllData() failed: " << GetLastError() << std::endl;
        return {};
    }


    std::vector<MonitorInfo> info;
    while (true)
    {
        MonitorInfo mInfo;

        PWmiMonitorID MonitorID;
        PWNODE_ALL_DATA pWmiAllData = (PWNODE_ALL_DATA)pAllDataBuffer;
        if (pWmiAllData->WnodeHeader.Flags & WNODE_FLAG_FIXED_INSTANCE_SIZE)
            MonitorID = (PWmiMonitorID)&pAllDataBuffer[pWmiAllData->DataBlockOffset];
        else
            MonitorID = (PWmiMonitorID)&pAllDataBuffer[pWmiAllData->OffsetInstanceDataAndLength[0].OffsetInstanceData];

        ULONG nOffset = (ULONG)pAllDataBuffer[pWmiAllData->OffsetInstanceNameOffsets];
        mInfo.instanceName     = (WCHAR*)OFFSET_TO_PTR(pWmiAllData, nOffset + sizeof(USHORT));
        mInfo.userFriendlyName = (WCHAR*)MonitorID->UserFriendlyName;
        mInfo.manufacturerName = (WCHAR*)MonitorID->ManufacturerName;
        mInfo.productCodeId    = (WCHAR*)MonitorID->ProductCodeID;
        mInfo.serialNumberId   = (WCHAR*)MonitorID->SerialNumberID;
        info.push_back(mInfo);

        if (!pWmiAllData->WnodeHeader.Linkage)
            break;
        pAllDataBuffer += pWmiAllData->WnodeHeader.Linkage;
    }
    free(pAllDataBufferOriginal);
    WmiCloseBlock(hWmiHandle);
    SetVideoMode(info);
    SetAllMonitors(info);
    return info;
}