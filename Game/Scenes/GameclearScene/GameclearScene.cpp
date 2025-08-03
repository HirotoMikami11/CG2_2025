#include "GameclearScene.h"
#include "Managers/ImGui/ImGuiManager.h" 
#include "Managers/Scene/SceneManager.h" 
#include "Managers/Transition/SceneTransitionHelper.h"

GameclearScene::GameclearScene()
	: BaseScene("GameclearScene") // シーン名を設定
	, cameraController_(nullptr)
	, directXCommon_(nullptr)
	, offscreenRenderer_(nullptr)
	, modelManager_(nullptr)
	, textureManager_(nullptr)
	, viewProjectionMatrix{ MakeIdentity4x4() }
	, viewProjectionMatrixSprite{ MakeIdentity4x4() }
{
}

GameclearScene::~GameclearScene() = default;

void GameclearScene::LoadResources() {
	// リソースの読み込み
	Logger::Log(Logger::GetStream(), "GameclearScene: Loading resources...\n");

	// リソースマネージャーの取得
	modelManager_ = ModelManager::GetInstance();
	textureManager_ = TextureManager::GetInstance();

	// モデルを事前読み込み
	modelManager_->LoadModel("resources/Model/FontModel", "gameclearFont.obj", "gameclearFont");
	modelManager_->LoadModel("resources/Model/Player", "player.obj", "player");


	Logger::Log(Logger::GetStream(), "GameclearScene: Resources loaded successfully\n");
}

void GameclearScene::ConfigureOffscreenEffects()
{
	/// オフスクリーンレンダラーのエフェクト設定

	// 全てのエフェクトを無効化
	offscreenRenderer_->DisableAllEffects();

	auto* depthFogEffect = offscreenRenderer_->GetDepthFogEffect();
	if (depthFogEffect) {
		depthFogEffect->SetEnabled(true);
		depthFogEffect->SetFogColor({ 0.02f, 0.08f, 0.25f, 1.0f });
		depthFogEffect->SetFogDistance(0.2f, 26.0f);//レールカメラの時は180程度
	}
	auto* depthOfFieldEffect = offscreenRenderer_->GetDepthOfFieldEffect();
	if (depthOfFieldEffect) {
		depthOfFieldEffect->SetEnabled(true);
		depthOfFieldEffect->SetFocusDistance(2.0f); // フォーカス距離
		depthOfFieldEffect->SetFocusRange(30.0f); // フォーカス範囲

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

void GameclearScene::Initialize() {
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

void GameclearScene::InitializeGameObjects() {
	///*-----------------------------------------------------------------------*///
	///									gameocerフォント							///
	///*-----------------------------------------------------------------------*///

	Vector3 gameclearFontPos = { 2.2f, 3.63f, -4.36f };
	Vector3 gameclearFontRote = { 0.45f, -0.47f, 0.0f };

	//初期化、座標設定
	gameclearFont_ = std::make_unique<Model3D>();
	gameclearFont_->Initialize(directXCommon_, "gameclearFont");
	gameclearFont_->SetPosition(gameclearFontPos);
	gameclearFont_->SetRotation(gameclearFontRote);
	gameclearFont_->SetColor({ 0.0f,1.0f,0.0f,1.0f });

	///*-----------------------------------------------------------------------*///
	///									プレイヤー(置物)							///
	///*-----------------------------------------------------------------------*///
	Vector3 gameoverPlayerPos = { 10.0f, 0.34f, -1.61f };
	Vector3 gameoverPlayerRote = { 0.0f, -1.37f, -0.0f };
	//初期化
	gameclearPlayer_ = std::make_unique<Model3D>();
	gameclearPlayer_->Initialize(directXCommon_, "player");
	gameclearPlayer_->SetPosition(gameoverPlayerPos);
	gameclearPlayer_->SetRotation(gameoverPlayerRote);

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
		{ 0.59f,-0.49f,-5.3f },
		{ 0.0f,-4.6f,0.0f },
		{ 1.82f,1.0f,1.1f }
	);

	rock_[1] = std::make_unique<Rock>();
	rock_[1]->Initialize(directXCommon_, RockType::Rock2,
		{ -3.17f,0.11f,2.7f },
		{ -3.07f,0.91f,-0.2f },
		{ 3.3f,1.8f,1.6f }
	);

	rock_[2] = std::make_unique<Rock>();
	rock_[2]->Initialize(directXCommon_, RockType::Rock3,
		{ 5.06f,-0.39f,2.4f },
		{ 0.0f,0.0f,0.0f },
		{ 1.7f,3.6f,1.0f }
	);
	///*-----------------------------------------------------------------------*///
	///									ライト									///
	///*-----------------------------------------------------------------------*///
	directionalLight_.Initialize(directXCommon_, Light::Type::DIRECTIONAL);
	directionalLight_.SetIntensity(0.2f);
}

void GameclearScene::Update() {
	// カメラ更新
	cameraController_->Update();

	// ゲームオブジェクト更新
	UpdateGameObjects();
}

void GameclearScene::UpdateGameObjects() {

	// タイトルシーンに戻る
	// スペースキーでゲームシーンへ遷移
	// Aボタンでゲームシーンへ遷移
	if (InputManager::GetInstance()->IsKeyTrigger(DIK_SPACE) ||
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




	// t を進める
	t += tSpeed;
	if (t >= 1.0f) {
		t = 0.0f; // ループ
	}

	// イージング値（0〜1）
	float easedT = EaseOutCubic(t);

	// 開始と終了位置
	float start = 8.0f;
	float end = -14.0f;

	float x = start + (end - start) * easedT;

	// イージング補間
	gameclearPlayer_->SetPosition({ x,0.0f,0.0f });


	// モデルの更新
	gameclearFont_->Update(viewProjectionMatrix);
	for (int i = 0; i < 3; i++)
	{
		rock_[i]->Update(viewProjectionMatrix);
	}
	//プレイヤー(置物)の更新
	gameclearPlayer_->Update(viewProjectionMatrix);
	ground_->Update(viewProjectionMatrix);
}

void GameclearScene::DrawOffscreen() {
	// ゲームオブジェクトの描画（オフスクリーンに描画）
	DrawGameObjects();
}
void GameclearScene::DrawBackBuffer()
{
	//一応3Dのものも外に置けるようにした


}

void GameclearScene::DrawGameObjects() {
	//タイトル文字
	gameclearFont_->Draw(directionalLight_);
	//タイトルプレイヤー(置物)
	gameclearPlayer_->Draw(directionalLight_);
	ground_->Draw(directionalLight_);
	for (int i = 0; i < 3; i++)
	{
		rock_[i]->Draw(directionalLight_);
	}
}

void GameclearScene::ImGui() {
#ifdef _DEBUG
	ImGui::Text("gameclearFont");
	gameclearFont_->ImGui();
	ImGui::Spacing();

	//プレイヤー(置物)
	gameclearPlayer_->ImGui();
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



void GameclearScene::Finalize() {
	// unique_ptrで自動的に解放される
	Logger::Log(Logger::GetStream(), "GameclearScene: Finalize\n");
}