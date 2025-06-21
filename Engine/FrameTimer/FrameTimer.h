#pragma once
#include <chrono>


/// <summary>
/// フレーム管理クラス
/// デルタタイムとフレーム数を管理する
/// </summary>
class FrameTimer {
public:
	/// <summary>
	  /// シングルトンインスタンスを取得
	  /// </summary>
	  /// <returns>FrameTimerの唯一のインスタンス</returns>
	static FrameTimer& GetInstance() {
		static FrameTimer instance;
		return instance;
	}



	/// <summary>
	/// フレーム開始時に呼ぶ（毎フレーム最初に実行）
	/// </summary>
	void BeginFrame();


	/// <summary>
	/// ImGuiで数値を表示
	/// </summary>
	void ImGui();

	//アクセッサ


	/// <summary>
	/// デルタタイムを取得（秒単位）
	/// </summary>
	/// <returns>前フレームからの経過時間（秒）</returns>
	float GetDeltaTime() const { return deltaTime_; }


	/// <summary>
	/// 現在のFPSを取得
	/// </summary>
	/// <returns>1秒間のフレーム数</returns>
	float GetFPS() const { return fps_; }

	/// <summary>
	/// アプリケーション開始からの経過時間を取得（秒単位）
	/// </summary>
	/// <returns>総経過時間（秒）</returns>
	float GetTotalTime() const { return totalTime_; }

	/// <summary>
	/// FPS計算の更新間隔を設定
	/// </summary>
	/// <param name="interval">更新間隔（秒）</param>
	void SetFPSUpdateInterval(float interval) { fpsUpdateInterval_ = interval; }

private:

	FrameTimer();
	~FrameTimer() = default;

	FrameTimer(const FrameTimer&) = delete;
	FrameTimer& operator=(const FrameTimer&) = delete;

	// 時間計測用
	std::chrono::high_resolution_clock::time_point lastFrameTime_;
	std::chrono::high_resolution_clock::time_point startTime_;

	// フレーム情報
	float deltaTime_;			// デルタタイム（秒）
	float totalTime_;			// 総経過時間（秒）

	// FPS計算用
	float fps_;					// 現在のFPS
	float fpsUpdateInterval_;	// FPS更新間隔（秒）
	float fpsTimer_;			// FPS計算用タイマー
	uint32_t fpsFrameCount_;	// FPS計算用フレームカウント

	/// <summary>
	/// FPSの状態を色と文字で表示する関数(ImGui用)
	/// </summary>
	void DrawFPSStats();

};