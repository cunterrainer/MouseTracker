#define _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <tchar.h>
#include <initguid.h>
#include <wmistr.h>
#include <iostream>
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


void GetMonitors()
{
    HRESULT hr = E_FAIL;
    LONG hWmiHandle;
    PWmiMonitorID MonitorID;
    HINSTANCE hDLL = LoadLibrary(L"Advapi32.dll");
    WmiOpenBlock = (WOB)GetProcAddress(hDLL, "WmiOpenBlock");
    WmiQueryAllData = (WQAD)GetProcAddress(hDLL, "WmiQueryAllDataW");
    WmiCloseBlock = (WCB)GetProcAddress(hDLL, "WmiCloseBlock");
    if (WmiOpenBlock != NULL && WmiQueryAllData && WmiCloseBlock)
    {
        WCHAR pszDeviceId[256] = L"";
        hr = WmiOpenBlock((LPGUID)&WmiMonitorID_GUID, GENERIC_READ, &hWmiHandle);
        if (hr == ERROR_SUCCESS)
        {
            ULONG nBufferSize = 0;
            UCHAR* pAllDataBuffer = 0;
            PWNODE_ALL_DATA pWmiAllData;
            hr = WmiQueryAllData(hWmiHandle, &nBufferSize, 0);
            if (hr == ERROR_INSUFFICIENT_BUFFER)
            {
                pAllDataBuffer = (UCHAR*)malloc(nBufferSize);
                hr = WmiQueryAllData(hWmiHandle, &nBufferSize, pAllDataBuffer);
                if (hr == ERROR_SUCCESS)
                {
                    while (1)
                    {
                        pWmiAllData = (PWNODE_ALL_DATA)pAllDataBuffer;
                        if (pWmiAllData->WnodeHeader.Flags & WNODE_FLAG_FIXED_INSTANCE_SIZE)
                            MonitorID = (PWmiMonitorID)&pAllDataBuffer[pWmiAllData->DataBlockOffset];
                        else
                            MonitorID = (PWmiMonitorID)&pAllDataBuffer[pWmiAllData->OffsetInstanceDataAndLength[0].OffsetInstanceData];

                        ULONG nOffset = 0;
                        WCHAR* pwsInstanceName = 0;
                        nOffset = (ULONG)pAllDataBuffer[pWmiAllData->OffsetInstanceNameOffsets];
                        pwsInstanceName = (WCHAR*)OFFSET_TO_PTR(pWmiAllData, nOffset + sizeof(USHORT));
                        WCHAR wsText[255] = L"";
                        swprintf(wsText, 255, L"Instance Name = %s\r\n", pwsInstanceName);
                        OutputDebugString(wsText);
                        std::wcout << wsText << std::endl;

                        WCHAR* pwsUserFriendlyName;
                        pwsUserFriendlyName = (WCHAR*)MonitorID->UserFriendlyName;
                        swprintf(wsText, 255, L"User Friendly Name = %s\r\n", pwsUserFriendlyName);
                        OutputDebugString(wsText);
                        std::wcout << wsText << std::endl;

                        WCHAR* pwsManufacturerName;
                        pwsManufacturerName = (WCHAR*)MonitorID->ManufacturerName;
                        swprintf(wsText, 255, L"Manufacturer Name = %s\r\n", pwsManufacturerName);
                        OutputDebugString(wsText);
                        std::wcout << wsText << std::endl;

                        WCHAR* pwsProductCodeID;
                        pwsProductCodeID = (WCHAR*)MonitorID->ProductCodeID;
                        swprintf(wsText, 255, L"Product Code ID = %s\r\n", pwsProductCodeID);
                        OutputDebugString(wsText);
                        std::wcout << wsText << std::endl;

                        WCHAR* pwsSerialNumberID;
                        pwsSerialNumberID = (WCHAR*)MonitorID->SerialNumberID;
                        swprintf(wsText, 255, L"Serial Number ID = %s\r\n", pwsSerialNumberID);
                        OutputDebugString(wsText);

                        if (!pWmiAllData->WnodeHeader.Linkage)
                            break;
                        pAllDataBuffer += pWmiAllData->WnodeHeader.Linkage;
                        std::wcout << wsText << std::endl;
                    }
                    //free(pAllDataBuffer);
                }
            }
            WmiCloseBlock(hWmiHandle);
        }
    }
}