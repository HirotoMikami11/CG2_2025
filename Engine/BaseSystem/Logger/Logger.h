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

	static void Finalize();

	static void Log(const std::string& message);

	static void Log(std::ostream& os, const std::string& message);

	// std::format を使用したもの
	template<typename... Args>
	static void Log(const std::string& format, Args&&... args) {
		if constexpr (sizeof...(args) > 0) {
			// 引数がある場合はフォーマットを適用
			std::string formatted = std::format(format, std::forward<Args>(args)...);
			Log(formatted);
		} else {
			// 引数がない場合はそのまま出力
			Log(format);
		}
	}

	//printfを使用した者
	template<typename... Args>
	static void LogF(const char* format, Args&&... args) {
		constexpr size_t bufferSize = 1024;
		char buffer[bufferSize];

		if constexpr (sizeof...(args) > 0) {
			// 引数がある場合はsnprintfでフォーマット
			std::snprintf(buffer, bufferSize, format, args...);
		} else {
			// 引数がない場合はそのままコピー
			std::strncpy(buffer, format, bufferSize - 1);
			buffer[bufferSize - 1] = '\0';
		}

		Log(std::string(buffer));
	}

	static std::wstring ConvertString(const std::string& str);

	static std::string ConvertString(const std::wstring& str);

	static std::ofstream& GetStream() { return logFileStream_; };

	static void SetEnabled(bool enabled) { isEnabled_ = enabled; }
	static bool IsEnabled() { return isEnabled_; }

private:
	//ログの出力先
	static std::ofstream logFileStream_;

	// ログ有効フラグ
	static bool isEnabled_;
};