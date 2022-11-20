#pragma once
#include <string>
#include <vector>

struct MonitorInfo
{
    std::wstring adapter;
    std::wstring name;
    int x, y, w, h;
};
std::vector<MonitorInfo> GetMonitors();