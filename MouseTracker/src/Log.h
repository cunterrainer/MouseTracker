#pragma once
#include <iostream>
#include <ostream>

struct Logger
{
private:
    const char* const m_LogInfo;
    mutable bool newLine = true;
public:
    inline explicit Logger(const char* info) noexcept : m_LogInfo(info) {}

    inline const Logger& operator<<(std::ostream& (*osmanip)(std::ostream&)) const noexcept
    {
        std::cout << *osmanip;
        newLine = true;
        return *this;
    }

    template <class T>
    inline const Logger& operator<<(const T& mess) const noexcept
    {
        if (newLine)
        {
            std::cout << m_LogInfo;
            newLine = false;
        }
        std::cout << mess;
        return *this;
    }
};
static inline const Logger Log("[LOG] ");
static inline const Logger Err("[ERR] ");