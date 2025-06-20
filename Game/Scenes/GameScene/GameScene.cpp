#include "GameScene.h"

GameScene::GameScene()
	: BaseScene("GameScene") // シーン名を設定
	, cameraController_(nullptr)
	, directXCommon_(nullptr)
	, offscreenRenderer_(nullptr) {
}

GameScene::~GameScene() = default;

void GameScene::Initialize() {
	// システム参照の取得
	directXCommon_ = Engine::GetInstance()->GetDirectXCommon();
	offscreenRenderer_ = Engine::GetInstance()->GetOffscreenRenderer();

	///*-----------------------------------------------------------------------*///
	///								カメラの初期化									///
	///*-----------------------------------------------------------------------*///
	cameraController_ = CameraController::GetInstance();
	cameraController_->Initialize();


	// ゲームオブジェクト初期化
	InitializeGameObjects();
}

void GameScene::InitializeGameObjects() {
	///*-----------------------------------------------------------------------*///
	///									プレイヤー								///
	///*-----------------------------------------------------------------------*///


	//初期化
	player_ = std::make_unique<Player>();
	player_->Initialize();


	///*-----------------------------------------------------------------------*///
	///									ライト									///
	///*-----------------------------------------------------------------------*///
	directionalLight_.Initialize(directXCommon_, Light::Type::DIRECTIONAL);

}

void GameScene::Update() {
	// カメラ更新
	cameraController_->Update();
	cameraController_->SetTransform({ 0.0f, 0.0f, -50.0f });

	// ゲームオブジェクト更新
	UpdateGameObjects();
}

void GameScene::UpdateGameObjects() {


	// 行列更新
	viewProjectionMatrix = cameraController_->GetViewProjectionMatrix();
	viewProjectionMatrixSprite = cameraController_->GetViewProjectionMatrixSprite();


	player_->Update(viewProjectionMatrix);


}

void GameScene::Draw() {
	// ゲームオブジェクトの描画（オフスクリーンに描画）
	DrawGameObjects();
}

void GameScene::DrawGameObjects() {
	player_->Draw(directionalLight_);
}

void GameScene::OnEnter() {
	// ゲームシーンに入る時の処理
	cameraController_->SetTransform({ 0.0f, 0.0f, -50.0f });
}

void GameScene::OnExit() {
	// ゲームシーンから出る時の処理
}

void GameScene::ImGui() {


	ImGui::Spacing();

	player_->ImGui();
	// ライトのImGui
	ImGui::Text("Lighting");
	directionalLight_.ImGui("DirectionalLight");
}

void GameScene::Finalize() {
	// unique_ptrで自動的に解放される
}