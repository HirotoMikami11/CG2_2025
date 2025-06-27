#include "CameraController/CameraController.h"
#include "Managers/ImGuiManager.h" 

CameraController* CameraController::GetInstance() {
	static CameraController instance;
	return &instance;
}

void CameraController::Initialize(const Vector3& Position)
{
	//カメラの初期化
	camera_.Initialize(Position);

	//デバッグカメラの初期化
	debugCamera_.Initialize(Position);


	useDebugCamera_ = false; // デフォルトではメインカメラを使用する

#ifdef _DEBUG
	useDebugCamera_ = false;
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

void CameraController::SetPositon(const Vector3& Position)
{

	cameraTranslation_ = Position;

	camera_.SetPositon(cameraTranslation_);
	debugCamera_.SetPositon(cameraTranslation_);


}

Vector3 CameraController::GetPosition() const {
	// 現在アクティブなカメラの位置を返す
	if (!useDebugCamera_) {
		// メインカメラの位置を取得
		return camera_.GetPosition();
	} else {
		// デバッグカメラの位置を取得
		return debugCamera_.GetPosition();
	}
}
void CameraController::ImGui()
{
#ifdef _DEBUG

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

#endif
}

void CameraController::SwitchCamera()
{
#ifdef _DEBUG
	//Shift + TABでメインとデバッグカメラを切り替える
	if (InputManager::GetInstance()->IsKeyTrigger(DIK_TAB) &&
		InputManager::GetInstance()->IsKeyDown(DIK_LSHIFT)) {
		useDebugCamera_ = !useDebugCamera_;

	}
	#endif
}

