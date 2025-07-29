#include "GameScene.h"
#include "Managers/ImGui/ImGuiManager.h"
#include "Managers/Scene/SceneManager.h" 
#include "Managers/Transition/SceneTransitionHelper.h"

GameScene::GameScene()
	: BaseScene("GameScene") // シーン名を設定
	, cameraController_(nullptr)
	, directXCommon_(nullptr)
	, offscreenRenderer_(nullptr)
	, modelManager_(nullptr)
	, textureManager_(nullptr)
	, viewProjectionMatrix{ MakeIdentity4x4() }
	, viewProjectionMatrixSprite{ MakeIdentity4x4() }
{
}

GameScene::~GameScene() = default;

void GameScene::LoadResources() {
	// リソースの読み込み
	Logger::Log(Logger::GetStream(), "GameScene: Loading resources...\n");

	// リソースマネージャーの取得
	modelManager_ = ModelManager::GetInstance();
	textureManager_ = TextureManager::GetInstance();


	// 必要に応じて独自のモデルファイルを読み込み
	// modelManager_->LoadModel("resources/Model/Enemy", "enemy.obj", "model_Enemy");

	Logger::Log(Logger::GetStream(), "GameScene: Resources loaded successfully\n");
}

void GameScene::ConfigureOffscreenEffects() {
	/// オフスクリーンレンダラーのエフェクト設定

	// 全てのエフェクトを無効化
	offscreenRenderer_->DisableAllEffects();

	// 必要に応じてエフェクトを有効化
	/*
	auto* depthFogEffect = offscreenRenderer_->GetDepthFogEffect();
	if (depthFogEffect) {
		depthFogEffect->SetEnabled(true);
		depthFogEffect->SetFogDistance(0.2f, 40.0f);
	}
	*/
}

void GameScene::Initialize() {
	// システム参照の取得
	directXCommon_ = Engine::GetInstance()->GetDirectXCommon();
	offscreenRenderer_ = Engine::GetInstance()->GetOffscreenRenderer();

	///*-----------------------------------------------------------------------*///
	///								カメラの初期化									///
	///*-----------------------------------------------------------------------*///
	cameraController_ = CameraController::GetInstance();
	// 座標と回転を指定して初期化
	Vector3 initialPosition = { 0.0f, 0.0f, -50.0f };
	Vector3 initialRotation = { 0.0f, 0.0f, 0.0f };
	cameraController_->Initialize(initialPosition, initialRotation);
	cameraController_->SetActiveCamera("normal");

	///*-----------------------------------------------------------------------*///
	///								衝突マネージャー								///
	///*-----------------------------------------------------------------------*///
	// 衝突マネージャーの生成
	collisionManager_ = std::make_unique<CollisionManager>();
	// 衝突マネージャーの初期化
	collisionManager_->Initialize();

	// ゲームオブジェクト初期化
	InitializeGameObjects();

	// ポストエフェクトの初期設定
	ConfigureOffscreenEffects();
}

void GameScene::InitializeGameObjects() {
	///*-----------------------------------------------------------------------*///
	///								ライティング								///
	///*-----------------------------------------------------------------------*///
	directionalLight_.Initialize(directXCommon_, Light::Type::DIRECTIONAL);

	///*-----------------------------------------------------------------------*///
	///								プレイヤー									///
	///*-----------------------------------------------------------------------*///
	player_ = std::make_unique<Player>();
	player_->Initialize(directXCommon_);

	///*-----------------------------------------------------------------------*///
	///								敵キャラ									///
	///*-----------------------------------------------------------------------*///
	enemy_ = std::make_unique<Enemy>();
	enemy_->Initialize(directXCommon_, Vector3{ 50.0f, 10.0f, 200.0f });
	// 敵キャラにプレイヤーのアドレスを渡す
	enemy_->SetPlayer(player_.get());
}

void GameScene::Update() {

	// カメラ更新
	cameraController_->Update();

	// ゲームオブジェクト更新
	UpdateGameObjects();

	///*-----------------------------------------------------------------------*///
	///								衝突判定										///
	///*-----------------------------------------------------------------------*///
	// 衝突マネージャーの更新
	// プレイヤー・敵弾のリストを取得
	const std::list<std::unique_ptr<PlayerBullet>>& playerBullets = player_->GetBullets();
	const std::list<std::unique_ptr<EnemyBullet>>& enemyBullets = enemy_->GetBullets();

	// 衝突マネージャーのリストをクリアする
	collisionManager_->ClearColliderList();

	// Player, Enemyのコライダーを追加する
	collisionManager_->AddCollider(player_.get());
	collisionManager_->AddCollider(enemy_.get());

	// Bulletのコライダーを追加する
	for (const auto& bullet : playerBullets) {
		collisionManager_->AddCollider(bullet.get());
	}
	for (const auto& bullet : enemyBullets) {
		collisionManager_->AddCollider(bullet.get());
	}

	// 衝突判定と応答
	collisionManager_->Update();
}

void GameScene::UpdateGameObjects() {
	// 行列更新
	viewProjectionMatrix = cameraController_->GetViewProjectionMatrix();
	viewProjectionMatrixSprite = cameraController_->GetViewProjectionMatrixSprite();

	// プレイヤーの更新
	if (player_) {
		player_->Update(viewProjectionMatrix);
	}

	// 敵の更新
	if (enemy_) {
		enemy_->Update(viewProjectionMatrix);
	}
}

void GameScene::DrawOffscreen() {
	// 3Dゲームオブジェクトの描画（オフスクリーンに描画）
	DrawGameObjects();
}

void GameScene::DrawBackBuffer() {
	// UI(スプライトなど)の描画（オフスクリーン外に描画）
	// 現在はUI要素なし
}

void GameScene::DrawGameObjects() {
	// プレイヤーの描画
	if (player_) {
		player_->Draw(directionalLight_);
	}

	// 敵の描画
	if (enemy_) {
		enemy_->Draw(directionalLight_);
	}
}

void GameScene::OnEnter() {
	// ゲームシーンに入る時の処理
}

void GameScene::OnExit() {
	// ゲームシーンから出る時の処理
}

void GameScene::ImGui() {
#ifdef _DEBUG
	// プレイヤーのImGui

	ImGui::Text("Player");
	player_->ImGui();
	ImGui::Spacing();


	// 敵のImGui

	ImGui::Text("Enemy");
	enemy_->ImGui();
	ImGui::Spacing();


	// ライトのImGui
	ImGui::Text("Lighting");
	directionalLight_.ImGui("DirectionalLight");
	ImGui::Spacing();


#endif
}

void GameScene::Finalize() {
	// unique_ptrで自動的に解放される
}