#pragma once
#include <string>
#include <vector>

struct MonitorInfo
{
    std::wstring instanceName;
    std::wstring userFriendlyName;
    std::wstring manufacturerName;
    std::wstring productCodeId;
    std::wstring serialNumberId;
    int x, y, w, h;
};
std::vector<MonitorInfo> GetMonitors();