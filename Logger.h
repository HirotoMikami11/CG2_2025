#pragma once
#include <string>
#include <ostream>

class Logger
{
public:

    static void Log(const std::string& message);

    static void Log(std::ostream& os, const std::string& message);

    static std::wstring ConvertString(const std::string& str);

    static std::string ConvertString(const std::wstring& str);
};

