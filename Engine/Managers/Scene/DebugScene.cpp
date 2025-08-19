#include "DebugScene.h"
#include "Managers/ImGui/ImGuiManager.h" 


DebugScene::DebugScene()
	: BaseScene("DebugScene") // シーン名を設定
	, cameraController_(nullptr)
	, directXCommon_(nullptr)
	, offscreenRenderer_(nullptr)
	, textureManager_(nullptr)
	, viewProjectionMatrix{ MakeIdentity4x4() }
{
}

DebugScene::~DebugScene() = default;


void DebugScene::LoadResources() {
	// リソースの読み込み
	Logger::Log(Logger::GetStream(), "DebugScene: Loading resources...\n");

	// リソースマネージャーの取得
	textureManager_ = TextureManager::GetInstance();

	// テクスチャの事前読み込み（必要に応じて）
	// textureManager_->LoadTexture("resources/textures/uvChecker.png", "uvChecker");

	Logger::Log(Logger::GetStream(), "DebugScene: Resources loaded successfully\n");
}

void DebugScene::ConfigureOffscreenEffects()
{
	/// オフスクリーンレンダラーのエフェクト設定

	// 全てのエフェクトを無効化
	offscreenRenderer_->DisableAllEffects();

	// デバッグ用なので基本的にエフェクトは無効のまま
	// 必要に応じてここでエフェクトを有効化
}

void DebugScene::Initialize() {
	// システム参照の取得
	directXCommon_ = Engine::GetInstance()->GetDirectXCommon();
	offscreenRenderer_ = Engine::GetInstance()->GetOffscreenRenderer();

	///*-----------------------------------------------------------------------*///
	///								カメラの初期化									///
	///*-----------------------------------------------------------------------*///
	cameraController_ = CameraController::GetInstance();
	// 座標と回転を指定して初期化
	Vector3 initialPosition = { 0.0f, 6.8f, -18.0f };
	Vector3 initialRotation = { 0.4f, 0.0f, 0.0f };
	cameraController_->Initialize(initialPosition, initialRotation);
	cameraController_->SetActiveCamera("normal");

	// ゲームオブジェクト初期化
	InitializeGameObjects();
	//ポストエフェクトの初期設定
	ConfigureOffscreenEffects();
}

void DebugScene::InitializeGameObjects() {

	///*-----------------------------------------------------------------------*///
	///									平面									///
	///*-----------------------------------------------------------------------*///
	Vector3Transform transformPlane{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f}
	};

	plane_ = std::make_unique<Plane>();
	plane_->Initialize(directXCommon_, "plane", "uvChecker");
	plane_->SetTransform(transformPlane);

	///*-----------------------------------------------------------------------*///
	///								グリッド線									///
	///*-----------------------------------------------------------------------*///

	// グリッド
	gridLine_ = std::make_unique<GridLine>();
	// 100m、1m間隔、10mごとに黒
	gridLine_->Initialize(directXCommon_,
		GridLineType::XZ,			// グリッドタイプ：XZ平面
		100.0f,						// サイズ
		1.0f,						// 間隔
		10.0f,						// 主要線間隔
		{ 0.5f, 0.5f, 0.5f, 1.0f },	// 通常線：灰色
		{ 0.0f, 0.0f, 0.0f, 1.0f }	// 主要線：黒
	);
	gridLine_->SetName("Main Grid");

	///*-----------------------------------------------------------------------*///
	///									ライト									///
	///*-----------------------------------------------------------------------*///
	directionalLight_.Initialize(directXCommon_, Light::Type::DIRECTIONAL);
}

void DebugScene::Update() {
	// カメラ更新
	cameraController_->Update();

	// ゲームオブジェクト更新
	UpdateGameObjects();
}

void DebugScene::UpdateGameObjects() {
	// 行列更新
	viewProjectionMatrix = cameraController_->GetViewProjectionMatrix();

	// 平面の更新
	plane_->Update(viewProjectionMatrix);

	// グリッド線更新
	gridLine_->Update(viewProjectionMatrix);
}

void DebugScene::DrawOffscreen() {
	// グリッド線を描画（3D要素）
	gridLine_->Draw(viewProjectionMatrix);

	// 3Dゲームオブジェクトの描画（オフスクリーンに描画）
	// 平面の描画
	plane_->Draw(directionalLight_);
}

void DebugScene::DrawBackBuffer() {
	// UI要素があればここに描画
	// 現在は何も描画しない
}

void DebugScene::ImGui() {
#ifdef _DEBUG

	ImGui::Text("Debug Scene");
	ImGui::Separator();

	ImGui::Spacing();
	ImGui::Text("Plane");
	plane_->ImGui();

	ImGui::Spacing();
	ImGui::Text("Grid Line");
	gridLine_->ImGui();

	ImGui::Spacing();
	// ライトのImGui
	ImGui::Text("Lighting");
	directionalLight_.ImGui("DirectionalLight");

#endif
}

void DebugScene::Finalize() {
	// unique_ptrで自動的に解放される
}