#include "CameraController.h"


CameraController* CameraController::GetInstance() {
	static CameraController instance;
	return &instance;
}

void CameraController::Initialize()
{
	//カメラの初期化
	camera_.Initialize();
	// カメラの初期位置を設定
	camera_.SetTranslate({ 0.0f, 0.0f, -10.0f });

	//デバッグカメラの初期化
	debugCamera_.Initialize();
	// カメラの初期位置を設定
	debugCamera_.SetTranslate({ 0.0f, 0.0f, -10.0f });

	useDebugCamera_ = true;
}

void CameraController::Update()
{

	///
	///	カメラの更新
	/// 

	if (!useDebugCamera_) {
		camera_.Update();
		viewProjectionMatrix_ = camera_.GetViewProjectionMatrix();

	} else {
		debugCamera_.Update();
		viewProjectionMatrix_ = debugCamera_.GetViewProjectionMatrix();

	}


	///スプライトの行列も更新
	viewProjectionMatrixSprite_ = camera_.GetSpriteViewProjectionMatrix();

	ImGui();

}

void CameraController::ImGui()
{

	ImGui::Begin("CameraController");

	// カメラモード切り替え
	ImGui::Text("CameraMode: %s", useDebugCamera_ ? "DebugCamera" : "MainCamera");

	if (ImGui::Button(useDebugCamera_ ? "Use DebugCamera" : "Use MainCamera")) {
		useDebugCamera_ = !useDebugCamera_;
	}

	ImGui::Separator(); // 区切り線

	// デバッグカメラが有効な場合のみ、デバッグカメラのImGuiを表示
	if (useDebugCamera_) {
		debugCamera_.ImGui(); // ここでデバッグカメラのImGuiを呼び出す
	} else {
		// メインカメラの情報表示（必要に応じて）
		ImGui::Text("MainCamera is active");
		// メインカメラの設定があればここに追加
	}
	ImGui::End();


}
