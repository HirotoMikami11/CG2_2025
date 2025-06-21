#include "BaseSystem/Logger/Logger.h"
#include <Windows.h>
#include <strsafe.h>

///*-----------------------------------------------------------------------*///
//																			//
///									ログ関連の関数							   ///
//																			//
///*-----------------------------------------------------------------------*///

//変数の定義
std::ofstream Logger::logFileStream_;
bool Logger::isEnabled_ = false;  // デフォルトで有効

void Logger::Initalize()
{
	// ログが無効の場合は何もしない
	if (!isEnabled_) {
		return;
	}

	//ログにディレクトリを用意する
	std::filesystem::create_directory("logs");
	//現在時刻を取得（UTC時刻）
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	//ログファイルの名前にコンマ何秒はいらないので、削って秒にする
	std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>
		nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
	//日本時間（PCの設定時間）に変換
	std::chrono::zoned_time localTime{ std::chrono::current_zone(),nowSeconds };
	//formatを使って年月日_時分秒の文字列に変換
	std::string dateString = std::format("{:%Y%m%d_%H%M%S}", localTime);
	//時刻を使ってファイル名を決定
	std::string logFilePath = std::string("logs/") + dateString + ".log";

	// ログファイルを開く（一度だけ開いて保持）
	logFileStream_.open(logFilePath);

	// ファイルが正常に開かれたかチェック
	if (!logFileStream_.is_open()) {
		OutputDebugStringA("Error: Could not open log file\n");
		return;
	}

	//初期化完了のログ出力
	Logger::Log("Hello,DirectX!\n");
}

/// プログラム終了時にファイルを閉じる関数
void Logger::Finalize()
{
	if (logFileStream_.is_open()) {
		Log("Logger finalized.\n");
		logFileStream_.close();
	}
}

/// 出力ウィンドウとログファイルに文字を出す関数
void Logger::Log(const std::string& message) {
	// ログが無効の場合は何もしない
	if (!isEnabled_) {
		return;
	}

	// 出力ウィンドウに出力
	OutputDebugStringA(message.c_str());

	// ログファイルに出力（ファイルが開いている場合のみ）
	if (logFileStream_.is_open()) {
		logFileStream_ << message;
		logFileStream_.flush(); // 即座にファイルに書き込む
	}
}

/// カスタムストリームと出力ウィンドウに文字を出す関数（Logger::GetStream()を使う場合との互換性用）
void Logger::Log(std::ostream& os, const std::string& message) {
	// ログが無効の場合は何もしない
	if (!isEnabled_) {
		return;
	}

	// カスタムストリームに出力
	os << message;
	if (&os == &logFileStream_) {
		// ログファイルストリームの場合は即座にフラッシュ
		os.flush();
	}

	// 出力ウィンドウにも出力
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