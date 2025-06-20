#include "TitleScene.h"

TitleScene::TitleScene()
	: BaseScene("TitleScene") // シーン名を設定
	, cameraController_(nullptr)
	, directXCommon_(nullptr)
	, offscreenRenderer_(nullptr) {
}

TitleScene::~TitleScene() = default;

void TitleScene::Initialize() {
	// システム参照の取得
	directXCommon_ = Engine::GetInstance()->GetDirectXCommon();
	offscreenRenderer_ = Engine::GetInstance()->GetOffscreenRenderer();

	///*-----------------------------------------------------------------------*///
	///								カメラの初期化									///
	///*-----------------------------------------------------------------------*///
	cameraController_ = CameraController::GetInstance();
	cameraController_->Initialize();
	cameraController_->SetTransform({ 0.0f, 0.0f, -10.0f });

	// ゲームオブジェクト初期化
	InitializeGameObjects();
}

void TitleScene::InitializeGameObjects() {
	///*-----------------------------------------------------------------------*///
	///									タイトルフォント									///
	///*-----------------------------------------------------------------------*///


	Vector3 titleFontPos = { -0.2f, 1.12f, 0.0f };
	//初期化、座標設定
	titleFont_ = std::make_unique<Model3D>();
	titleFont_->Initialize(directXCommon_, "resources/TitleFont", "titleFont.obj");
	titleFont_->SetPosition(titleFontPos);


	///*-----------------------------------------------------------------------*///
	///									ライト									///
	///*-----------------------------------------------------------------------*///
	directionalLight_.Initialize(directXCommon_, Light::Type::DIRECTIONAL);

}

void TitleScene::Update() {
	// カメラ更新
	cameraController_->Update();

	// ゲームオブジェクト更新
	UpdateGameObjects();
}

void TitleScene::UpdateGameObjects() {


	// 行列更新
	viewProjectionMatrix = cameraController_->GetViewProjectionMatrix();
	viewProjectionMatrixSprite = cameraController_->GetViewProjectionMatrixSprite();

	// モデルの更新
	titleFont_->Update(viewProjectionMatrix);

}

void TitleScene::Draw() {
	// ゲームオブジェクトの描画（オフスクリーンに描画）
	DrawGameObjects();
}

void TitleScene::DrawGameObjects() {
	titleFont_->Draw(directionalLight_);
}

void TitleScene::OnEnter() {
	// ゲームシーンに入る時の処理

}

void TitleScene::OnExit() {
	// ゲームシーンから出る時の処理
}

void TitleScene::ImGui() {

	ImGui::Text("TitleFont");
	titleFont_->ImGui();
	ImGui::Spacing();
	// ライトのImGui
	ImGui::Text("Lighting");
	directionalLight_.ImGui("DirectionalLight");
}

void TitleScene::Finalize() {
	// unique_ptrで自動的に解放される
}