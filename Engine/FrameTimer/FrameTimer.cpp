#include "FrameTimer.h"
#include "Managers/ImGui/ImGuiManager.h" 

FrameTimer::FrameTimer()
	: deltaTime_(0.0f)
	, totalTime_(0.0f)
	, fps_(0.0f)
	, fpsUpdateInterval_(1.0f)  // 1秒間隔でFPS更新
	, fpsTimer_(0.0f)
	, fpsFrameCount_(0)
{
	// 開始時間を記録
	startTime_ = std::chrono::high_resolution_clock::now();
	lastFrameTime_ = startTime_;
}

void FrameTimer::BeginFrame() {
	// 現在時刻を取得
	auto currentTime = std::chrono::high_resolution_clock::now();

	// デルタタイムを秒で計算
	//
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastFrameTime_);
	deltaTime_ = duration.count() / 1000000.0f;  // マイクロ秒から秒に変換


	// 総経過時間を更新
	totalTime_ += deltaTime_;

	// FPS計算
	fpsTimer_ += deltaTime_;
	fpsFrameCount_++;

	// 指定間隔でFPSを更新
	if (fpsTimer_ >= fpsUpdateInterval_) {
		//frame / secondの計算
		fps_ = static_cast<float>(fpsFrameCount_) / fpsTimer_;
		fpsTimer_ = 0.0f;
		fpsFrameCount_ = 0;
	}

	// 次フレーム用に現在時刻を保存
	lastFrameTime_ = currentTime;
}

void FrameTimer::ImGui()
{


	//FPS関連の情報を描画
	ImGui::Text("Frame Timer Info");

	//FPS
	ImGui::Text("FPS: %.1f", fps_);
	ImGui::SameLine();
	DrawFPSStats();

	//ImGui::Text("Delta Time: %.6f s", deltaTime_);
	//ImGui::Text("Total Time: %.2f s", totalTime_);

	ImGui::Separator();	//線

}


//FPSの状態を色と文字で表示
void FrameTimer::DrawFPSStats()
{

	// FPS品質表示
	ImVec4 color;
	const char* status;
	///最初の1フレと少しの間は計算されていないので早期リターン
	if (totalTime_ <= 1.00f) { return; }

	if (fps_ >= 55.0f) {
		color = ImVec4(0, 1, 0, 1);  // 緑
		status = "Excellent";
	} else if (fps_ >= 30.0f) {
		color = ImVec4(1, 1, 0, 1);  // 黄
		status = "Good";
	} else if (fps_ >= 20.0f) {
		color = ImVec4(1, 0.5f, 0, 1);  // オレンジ
		status = "Fair";
	} else {
		color = ImVec4(1, 0, 0, 1);  // 赤
		status = "Poor";
	}
	ImGui::TextColored(color, "Status: %s", status);


}
