#include "TitleScene.h"
#include "Managers/ImGuiManager.h" 
#include "Managers/Scene/SceneManager.h" 

TitleScene::TitleScene()
	: BaseScene("TitleScene") // シーン名を設定
	, cameraController_(nullptr)
	, directXCommon_(nullptr)
	, offscreenRenderer_(nullptr)
	, modelManager_(nullptr)
	, textureManager_(nullptr)
	, viewProjectionMatrix{ MakeIdentity4x4() }
	, viewProjectionMatrixSprite{ MakeIdentity4x4() }
{
}

TitleScene::~TitleScene() = default;

void TitleScene::LoadResources() {
	// リソースの読み込み
	Logger::Log(Logger::GetStream(), "TitleScene: Loading resources...\n");

	// リソースマネージャーの取得
	modelManager_ = ModelManager::GetInstance();
	textureManager_ = TextureManager::GetInstance();

	// モデルを事前読み込み
	modelManager_->LoadModel("resources/Model/TitleFont", "titleFont.obj", "titleFont");
	modelManager_->LoadModel("resources/Model/Player", "player.obj", "player");

	Logger::Log(Logger::GetStream(), "TitleScene: Resources loaded successfully\n");
}

void TitleScene::Initialize() {
	// システム参照の取得
	directXCommon_ = Engine::GetInstance()->GetDirectXCommon();
	offscreenRenderer_ = Engine::GetInstance()->GetOffscreenRenderer();


	///*-----------------------------------------------------------------------*///
	///								カメラの初期化									///
	///*-----------------------------------------------------------------------*///
	cameraController_ = CameraController::GetInstance();
	cameraController_->Initialize({ 0.0f, 0.0f, -10.0f });
	cameraController_->SetActiveCamera("normal");

	// ゲームオブジェクト初期化
	InitializeGameObjects();
}

void TitleScene::InitializeGameObjects() {
	///*-----------------------------------------------------------------------*///
	///									タイトルフォント							///
	///*-----------------------------------------------------------------------*///


	Vector3 titleFontPos = { -0.2f, 1.12f, 0.0f };
	//初期化、座標設定（リソースは既に読み込み済みなので軽量）
	titleFont_ = std::make_unique<Model3D>();
	titleFont_->Initialize(directXCommon_, "titleFont");  // 軽量（リソース参照のみ）
	titleFont_->SetPosition(titleFontPos);

	///*-----------------------------------------------------------------------*///
	///									プレイヤー(置物)							///
	///*-----------------------------------------------------------------------*///

	//初期化（軽量）
	TitlePlayer_ = std::make_unique<TitlePlayer>();
	TitlePlayer_->Initialize();

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

	// タイトルシーンに戻る
	if (InputManager::GetInstance()->IsKeyTrigger(DIK_SPACE)) {
		// SceneManagerを取得してタイトルシーンに切り替え
		//SceneManager::GetInstance()->SetNextScene("GameScene");
		SceneManager::GetInstance()->FadeOutToScene("GameScene", 1.0f);


	}

	// 行列更新
	viewProjectionMatrix = cameraController_->GetViewProjectionMatrix();
	viewProjectionMatrixSprite = cameraController_->GetViewProjectionMatrixSprite();


	// モデルの更新
	titleFont_->Update(viewProjectionMatrix);

	//プレイヤー(置物)の更新
	TitlePlayer_->Update(viewProjectionMatrix);

}

void TitleScene::Draw() {
	// ゲームオブジェクトの描画（オフスクリーンに描画）
	DrawGameObjects();
}

void TitleScene::DrawGameObjects() {
	//タイトル文字
	titleFont_->Draw(directionalLight_);
	//タイトルプレイヤー(置物)
	TitlePlayer_->Draw(directionalLight_);
}

void TitleScene::OnEnter() {
	// ゲームシーンに入る時の処理
	Logger::Log(Logger::GetStream(), "TitleScene: OnEnter\n");
}

void TitleScene::OnExit() {
	// ゲームシーンから出る時の処理
	Logger::Log(Logger::GetStream(), "TitleScene: OnExit\n");
}

void TitleScene::ImGui() {

#ifdef _DEBUG

	ImGui::Text("TitleFont");
	titleFont_->ImGui();
	ImGui::Spacing();

	//プレイヤー(置物)
	TitlePlayer_->ImGui();

	// ライトのImGui
	ImGui::Text("Lighting");
	directionalLight_.ImGui("DirectionalLight");
#endif
}

void TitleScene::Finalize() {
	// unique_ptrで自動的に解放される
	Logger::Log(Logger::GetStream(), "TitleScene: Finalize\n");
}