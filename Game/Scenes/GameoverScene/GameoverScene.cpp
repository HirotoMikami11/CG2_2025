#include "GameoverScene.h"
#include "Managers/ImGui/ImGuiManager.h" 
#include "Managers/Scene/SceneManager.h" 
#include "Managers/Transition/SceneTransitionHelper.h"

GameoverScene::GameoverScene()
	: BaseScene("GameoverScene") // シーン名を設定
	, cameraController_(nullptr)
	, directXCommon_(nullptr)
	, offscreenRenderer_(nullptr)
	, modelManager_(nullptr)
	, textureManager_(nullptr)
	, viewProjectionMatrix{ MakeIdentity4x4() }
	, viewProjectionMatrixSprite{ MakeIdentity4x4() }
{
}

GameoverScene::~GameoverScene() = default;

void GameoverScene::LoadResources() {
	// リソースの読み込み
	Logger::Log(Logger::GetStream(), "GameoverScene: Loading resources...\n");

	// リソースマネージャーの取得
	modelManager_ = ModelManager::GetInstance();
	textureManager_ = TextureManager::GetInstance();

	// モデルを事前読み込み
	modelManager_->LoadModel("resources/Model/FontModel", "gameoverFont.obj", "gameoverFont");
	modelManager_->LoadModel("resources/Model/Player", "player.obj", "player");


	Logger::Log(Logger::GetStream(), "GameoverScene: Resources loaded successfully\n");
}

void GameoverScene::ConfigureOffscreenEffects()
{
	/// オフスクリーンレンダラーのエフェクト設定

	// 全てのエフェクトを無効化
	offscreenRenderer_->DisableAllEffects();


	auto* depthFogEffect = offscreenRenderer_->GetDepthFogEffect();
	if (depthFogEffect) {
		depthFogEffect->SetEnabled(true); // 深度フォグを有効化
		depthFogEffect->SetFogColor({ 0.000f, 0.048f, 0.275f, 1.000f});
		depthFogEffect->SetFogDistance(depthFogEffect->GetFogNear(), 24.0f);
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
		vignetteEffect->SetVignetteStrength(0.9f);
		vignetteEffect->SetVignetteRadius(0.38f);
	}

}

void GameoverScene::Initialize() {
	// システム参照の取得
	directXCommon_ = Engine::GetInstance()->GetDirectXCommon();
	offscreenRenderer_ = Engine::GetInstance()->GetOffscreenRenderer();


	///*-----------------------------------------------------------------------*///
	///								カメラの初期化									///
	///*-----------------------------------------------------------------------*///

	cameraController_ = CameraController::GetInstance();
	// 座標と回転を指定して初期化
	Vector3 initialPosition = { 5.569f, 7.390f, -10.685f };
	Vector3 initialRotation = { 0.615f,-0.460f,0.0f };
	cameraController_->Initialize(initialPosition, initialRotation);
	cameraController_->SetActiveCamera("normal");

	// ゲームオブジェクト初期化
	InitializeGameObjects();
	//ポストエフェクトの初期設定
	ConfigureOffscreenEffects();
}

void GameoverScene::InitializeGameObjects() {
	///*-----------------------------------------------------------------------*///
	///									gameocerフォント							///
	///*-----------------------------------------------------------------------*///

	Vector3 gameoverFontPos = { -0.09f, 1.06f, -0.59f };
	Vector3 gameoverFontRote = { 0.12f, -0.058f, -0.47f };

	//初期化、座標設定
	gameoverFont_ = std::make_unique<Model3D>();
	gameoverFont_->Initialize(directXCommon_, "gameoverFont");
	gameoverFont_->SetPosition(gameoverFontPos);
	gameoverFont_->SetRotation(gameoverFontRote);

	///*-----------------------------------------------------------------------*///
	///									プレイヤー(置物)							///
	///*-----------------------------------------------------------------------*///
	Vector3 gameoverPlayerPos = { 1.03f, 0.1f, -2.67f };
	Vector3 gameoverPlayerRote = { 0.42f, -2.83f, -0.47f };
	//初期化
	gameoverPlayer_ = std::make_unique<Model3D>();
	gameoverPlayer_->Initialize(directXCommon_, "player");
	gameoverPlayer_->SetPosition(gameoverPlayerPos);
	gameoverPlayer_->SetRotation(gameoverPlayerRote);

	///*-----------------------------------------------------------------------*///
	///								地面									///
	///*-----------------------------------------------------------------------*///
	// 地面の生成
	ground_ = std::make_unique<Ground>();
	ground_->Initialize(directXCommon_, { 0.0f, 0.0f, 0.0f });

	///*-----------------------------------------------------------------------*///
	///								wiwa 								///
	///*-----------------------------------------------------------------------*///
// 地面の生成

		rock_[0] = std::make_unique<Rock>();
		rock_[0]->Initialize(directXCommon_, RockType::Rock1,
			{-4.7f,-0.07f,0.0f},
			{0.0f,0.0f,0.0f},
			{1.44f,1.0f,1.0f}
			);
	
		rock_[1] = std::make_unique<Rock>();
		rock_[1]->Initialize(directXCommon_, RockType::Rock2,
			{ 2.8f,0.73f,1.65f },
			{ -0.9f,0.11f,0.0f },
			{ 3.3f,1.8f,1.6f }
			);

		rock_[2] = std::make_unique<Rock>();
		rock_[2]->Initialize(directXCommon_, RockType::Rock3,
			{ 4.42f,0.4f,-4.2f },
			{ 0.0f,0.0f,0.0f },
			{ 1.0f,1.0f,1.0f }
		);

	///*-----------------------------------------------------------------------*///
	///									ライト									///
	///*-----------------------------------------------------------------------*///
	directionalLight_.Initialize(directXCommon_, Light::Type::DIRECTIONAL);
	directionalLight_.SetIntensity(0.2f);
}

void GameoverScene::Update() {
	// カメラ更新
	cameraController_->Update();

	// ゲームオブジェクト更新
	UpdateGameObjects();
}

void GameoverScene::UpdateGameObjects() {

	// タイトルシーンに戻る
	// スペースキーでゲームシーンへ遷移
	// Aボタンでゲームシーンへ遷移
	if (InputManager::GetInstance()->IsKeyTrigger(DIK_SPACE)||
		InputManager::GetInstance()->IsGamePadButtonTrigger(InputManager::GamePadButton::A)) {
		// フェードを使った遷移（ヘルパークラスを使用）
		SceneTransitionHelper::FadeToScene("TitleScene", 1.0f);

		// スライドエフェクトを使った遷移
		// SceneTransitionHelper::TransitionToScene("GameScene", "slide_left", 0.5f, 0.5f);

		// エフェクトなしの即座の遷移
		// SceneTransitionHelper::ChangeSceneImmediate("GameScene");
	}

	// 行列更新
	viewProjectionMatrix = cameraController_->GetViewProjectionMatrix();
	viewProjectionMatrixSprite = cameraController_->GetViewProjectionMatrixSprite();

	// モデルの更新
	gameoverFont_->Update(viewProjectionMatrix);
	for (int i = 0; i < 3; i++)
	{
		rock_[i]->Update(viewProjectionMatrix);
	}
	//プレイヤー(置物)の更新
	gameoverPlayer_->Update(viewProjectionMatrix);
	ground_->Update(viewProjectionMatrix);
}

void GameoverScene::DrawOffscreen() {
	// ゲームオブジェクトの描画（オフスクリーンに描画）
	DrawGameObjects();
}
void GameoverScene::DrawBackBuffer()
{
	//一応3Dのものも外に置けるようにした


}

void GameoverScene::DrawGameObjects() {
	//タイトル文字
	gameoverFont_->Draw(directionalLight_);
	//タイトルプレイヤー(置物)
	gameoverPlayer_->Draw(directionalLight_);
	ground_->Draw(directionalLight_);
	for (int i = 0; i < 3; i++)
	{
		rock_[i]->Draw(directionalLight_);
	}
}

void GameoverScene::ImGui() {
#ifdef _DEBUG
	ImGui::Text("gameoverFont");
	gameoverFont_->ImGui();
	ImGui::Spacing();

	//プレイヤー(置物)
	gameoverPlayer_->ImGui();
	ground_->ImGui();

	for (int i = 0; i < 3; i++)
	{
		rock_[i]->ImGui();
	}
	// ライトのImGui
	ImGui::Text("Lighting");
	directionalLight_.ImGui("DirectionalLight");
#endif
}



void GameoverScene::Finalize() {
	// unique_ptrで自動的に解放される
	Logger::Log(Logger::GetStream(), "GameoverScene: Finalize\n");
}