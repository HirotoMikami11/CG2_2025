#include "TitleScene.h"
#include "Managers/ImGui/ImGuiManager.h" 
#include "Managers/Scene/SceneManager.h" 
#include "Managers/Transition/SceneTransitionHelper.h"

TitleScene::TitleScene()
	: BaseScene("TitleScene") // シーン名を設定
	, cameraController_(nullptr)
	, directXCommon_(nullptr)
	, offscreenRenderer_(nullptr)
	, modelManager_(nullptr)
	, textureManager_(nullptr)
	, audioManager_(nullptr)
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
	audioManager_ = AudioManager::GetInstance();

	// モデルを事前読み込み
	modelManager_->LoadModel("resources/Model/FontModel", "titleFont.obj", "titleFont");
	modelManager_->LoadModel("resources/Model/Player", "player.obj", "player");

	// Aボタンのテクスチャを読み込み
	textureManager_->LoadTexture("resources/Texture/AButton.png", "AButton");

	// 音声の読み込み
	audioManager_->LoadAudio("resources/Audio/TitleBGM.mp3", "TitleBGM");
	audioManager_->LoadAudio("resources/Audio/Select.mp3", "Select");

	Logger::Log(Logger::GetStream(), "TitleScene: Resources loaded successfully\n");
}

void TitleScene::ConfigureOffscreenEffects()
{
	/// オフスクリーンレンダラーのエフェクト設定

	// 全てのエフェクトを無効化
	offscreenRenderer_->DisableAllEffects();


	auto* depthFogEffect = offscreenRenderer_->GetDepthFogEffect();
	if (depthFogEffect) {
		depthFogEffect->SetEnabled(true);
		depthFogEffect->SetFogColor({ 0.02f, 0.08f, 0.25f, 1.0f });
		depthFogEffect->SetFogDistance(depthFogEffect->GetFogNear(), 50.0f);
	}
	auto* depthOfFieldEffect = offscreenRenderer_->GetDepthOfFieldEffect();
	if (depthOfFieldEffect) {
		depthOfFieldEffect->SetEnabled(true);
		depthOfFieldEffect->SetFocusDistance(2.0f);
		depthOfFieldEffect->SetFocusRange(5.4f);
	}
	auto* lineGlitchPostEffect = offscreenRenderer_->GetLineGlitchEffect();
	if (lineGlitchPostEffect) {
		lineGlitchPostEffect->SetEnabled(true);
		lineGlitchPostEffect->SetNoiseIntensity(0.02f);
		lineGlitchPostEffect->SetNoiseInterval(0.0f);
	}
	auto* vignetteEffect = offscreenRenderer_->GetVignetteEffect();
	if (vignetteEffect) {
		vignetteEffect->SetEnabled(true);
		vignetteEffect->SetVignetteStrength(0.6f);
		vignetteEffect->SetVignetteRadius(0.40f);
	}

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
	//ポストエフェクトの初期設定
	ConfigureOffscreenEffects();

	// BGMをループ再生で開始
	audioManager_->PlayLoop("TitleBGM");
	audioManager_->SetVolume("TitleBGM", 1.0f);

}

void TitleScene::InitializeGameObjects() {
	///*-----------------------------------------------------------------------*///
	///									タイトルフォント							///
	///*-----------------------------------------------------------------------*///

	Vector3 titleFontPos = { 0.0f, 1.0f, 1.6f };
	//初期化、座標設定（リソースは既に読み込み済みなので軽量）
	titleFont_ = std::make_unique<Model3D>();
	titleFont_->Initialize(directXCommon_, "titleFont");  // 軽量（リソース参照のみ）
	titleFont_->SetPosition(titleFontPos);

	///*-----------------------------------------------------------------------*///
	///									プレイヤー(置物)							///
	///*-----------------------------------------------------------------------*///

	//初期化
	titlePlayer_ = std::make_unique<TitlePlayer>();
	titlePlayer_->Initialize();

	///*-----------------------------------------------------------------------*///
	///									Aボタン									///
	///*-----------------------------------------------------------------------*///

	// 初期位置を画面中央、Y座標620に設定（ImGuiで調整可能）
	Vector2 buttonPosition = { 640.0f, 620.0f };  // Y座標を620に設定
	Vector2 buttonSize = { 64.0f, 64.0f };

	aButton_ = std::make_unique<Button>();
	aButton_->Initialize(directXCommon_, "AButton", buttonPosition, buttonSize);
	aButton_->SetName("A Button");

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
	// スペースキーでゲームシーンへ遷移
	if (InputManager::GetInstance()->IsKeyTrigger(DIK_SPACE) ||
		InputManager::GetInstance()->IsGamePadButtonTrigger(InputManager::GamePadButton::A)) {
		if (!SceneTransitionHelper::IsTransitioning()) {
			audioManager_->Stop("TitleBGM");
			audioManager_->Play("Select");
			audioManager_->SetVolume("Select", 0.3f);

			// フェードを使った遷移（ヘルパークラスを使用）
			SceneTransitionHelper::FadeToScene("GameScene", 1.0f);
		}
	}

	// 行列更新
	viewProjectionMatrix = cameraController_->GetViewProjectionMatrix();
	viewProjectionMatrixSprite = cameraController_->GetViewProjectionMatrixSprite();

	// モデルの更新
	titleFont_->Update(viewProjectionMatrix);

	//プレイヤー(置物)の更新
	titlePlayer_->AddRotation(rotate_);
	titlePlayer_->Update(viewProjectionMatrix);

	// Aボタンの更新
	aButton_->Update(viewProjectionMatrixSprite);
}

void TitleScene::DrawOffscreen() {
	// ゲームオブジェクトの描画（オフスクリーンに描画）
	//タイトル文字
	titleFont_->Draw(directionalLight_);
	//タイトルプレイヤー(置物)
	titlePlayer_->Draw(directionalLight_);
}

void TitleScene::DrawBackBuffer()
{
	//一応3Dのものも外に置けるようにした

	// Aボタンの描画
	aButton_->Draw();
}

void TitleScene::ImGui() {
#ifdef _DEBUG
	ImGui::Text("TitleFont");
	titleFont_->ImGui();
	ImGui::Spacing();

	//プレイヤー(置物)
	titlePlayer_->ImGui();
	ImGui::Spacing();

	// AボタンのImGui
	ImGui::Text("A Button");
	aButton_->ImGui();
	ImGui::Spacing();

	// ライトのImGui
	ImGui::Text("Lighting");
	directionalLight_.ImGui("DirectionalLight");
#endif
}

void TitleScene::Finalize() {
	// unique_ptrで自動的に解放される
	Logger::Log(Logger::GetStream(), "TitleScene: Finalize\n");
}