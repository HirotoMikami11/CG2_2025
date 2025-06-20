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

	useDebugCamera_ = false; // デフォルトではメインカメラを使用する

#ifdef _DEBUG
	useDebugCamera_ = true;
#endif // DEBUG


	///デバッグカメラで開始する場合、スプライトが表示されないので初期化したタイミングで一度だけ更新する
	camera_.Update();
	viewProjectionMatrix_ = camera_.GetViewProjectionMatrix();
	viewProjectionMatrixSprite_ = camera_.GetSpriteViewProjectionMatrix();
}

void CameraController::Update()
{

	///
	///	カメラの更新
	/// 
	
	//デバッグとゲームカメラの切り替え
	SwitchCamera();

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

void CameraController::SetTransform(const Vector3& newTransform)
{

	cameraTranslation_ = newTransform;

	camera_.SetTranslate(cameraTranslation_);

}

void CameraController::ImGui()
{

	ImGui::Begin("CameraController");

	// カメラモード切り替え
	ImGui::Text("CameraMode: %s", useDebugCamera_ ? "DebugCamera" : "MainCamera");

	if (ImGui::Button(useDebugCamera_ ? "Use MainCamera" : "Use DebugCamera")) {
		useDebugCamera_ = !useDebugCamera_;
	}

	ImGui::Separator(); // 区切り線

	// デバッグカメラが有効な場合のみ、デバッグカメラのImGuiを表示
	if (useDebugCamera_) {

		// デバッグカメラ
		debugCamera_.ImGui();

	} else {

		// メインカメラ
		camera_.ImGui();

	}

	ImGui::End();


}

void CameraController::SwitchCamera()
{
	//Shift + TABでメインとデバッグカメラを切り替える
	if (InputManager::GetInstance()->IsKeyTrigger(DIK_TAB) &&
		InputManager::GetInstance()->IsKeyDown(DIK_LSHIFT)) {
		useDebugCamera_ = !useDebugCamera_;

	}
}

