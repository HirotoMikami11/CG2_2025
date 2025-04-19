#include "Logger.h"
#include <Windows.h>
#include <strsafe.h>
///*-----------------------------------------------------------------------*///
//																			//
///									ログ関連の関数							   ///
//																			//
///*-----------------------------------------------------------------------*///

/// 出力ウィンドウに文字を出す関数
void Logger::Log(const std::string& message) {
	OutputDebugStringA(message.c_str());
}


/// 出力ウィンドウに文字を出し、ログファイルに書き込む関数
void Logger::Log(std::ostream& os, const std::string& message) {
	os << message << std::endl;
	OutputDebugStringA(message.c_str());
}

/// string -> wstringに変換する関数
 std::wstring Logger::ConvertString(const std::string& str) {
	if (str.empty()) {
		return std::wstring();
	}

	auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
	if (sizeNeeded == 0) {
		return std::wstring();
	}
	std::wstring result(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
	return result;
}


/// wstring -> stringに変換する関数
std::string Logger::ConvertString(const std::wstring& str) {
	if (str.empty()) {
		return std::string();
	}

	auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
	if (sizeNeeded == 0) {
		return std::string();
	}
	std::string result(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
	return result;
}

