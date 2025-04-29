#pragma once
#include <string>
#include <ostream>
#include<format>
//ファイルやディレクトリに関する操作を行うライブラリ
#include<filesystem>
//ファイルに書いたり読んだりするライブラリ
#include<fstream>
//時間を扱うライブラリ
#include<chrono>

class Logger
{
public:
    static void Initalize();

    static void Log(const std::string& message);

    static void Log(std::ostream& os, const std::string& message);

    static std::wstring ConvertString(const std::string& str);

    static std::string ConvertString(const std::wstring& str);

    static std::ofstream& GetStream() { return logFileStream_; };
private:
//ログの出力先
    static std::ofstream logFileStream_; 
};

