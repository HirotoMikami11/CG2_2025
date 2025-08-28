#define NOMINMAX
#include "GameScene.h"
#include "Managers/ImGui/ImGuiManager.h"
#include "Managers/Scene/SceneManager.h" 
#include "Managers/Transition/SceneTransitionHelper.h"

#include "GameObjects/Enemy/Enemy.h"
#include "GameObjects/Enemy/RushingFishEnemy.h"
#include "GameObjects/Enemy/ShootingFishEnemy.h"

#include "Camera/RailCamera.h"		//レールカメラ

GameScene::GameScene()
	: BaseScene("GameScene") // シーン名を設定
	, cameraController_(nullptr)
	, directXCommon_(nullptr)
	, offscreenRenderer_(nullptr)
	, modelManager_(nullptr)
	, textureManager_(nullptr)
	, viewProjectionMatrix{ MakeIdentity4x4() }
	, viewProjectionMatrixSprite{ MakeIdentity4x4() }
	, isWait_(false)
	, waitTimer_(0)
{
}

GameScene::~GameScene() = default;

void GameScene::LoadResources() {
	// リソースの読み込み
	Logger::Log(Logger::GetStream(), "GameScene: Loading resources...\n");

	// リソースマネージャーの取得
	modelManager_ = ModelManager::GetInstance();
	textureManager_ = TextureManager::GetInstance();
	audioManager_ = AudioManager::GetInstance();
	// モデルファイルを読み込み
	modelManager_->LoadModel("resources/Model/Ground", "ground.obj", "ground");
	modelManager_->LoadModel("resources/Model/Skydome", "skydome.obj", "skydome");
	modelManager_->LoadModel("resources/Model/Rocks", "rock1.obj", "rock1");
	modelManager_->LoadModel("resources/Model/Rocks", "rock2.obj", "rock2");
	modelManager_->LoadModel("resources/Model/Rocks", "rock3.obj", "rock3");
	modelManager_->LoadModel("resources/Model/Camera", "camera.obj", "camera");
	// プレイヤー関連
	modelManager_->LoadModel("resources/Model/Player", "player.obj", "player");
	modelManager_->LoadModel("resources/Model/PlayerBullet", "playerBullet.obj", "playerBullet");
	modelManager_->LoadModel("resources/Model/PlayerHomingBullet", "playerHomingBullet.obj", "playerHomingBullet");
	// 敵関連
	modelManager_->LoadModel("resources/Model/EnemyBullet", "enemyBullet.obj", "enemyBullet");
	modelManager_->LoadModel("resources/Model/Squid", "Mesh_Squid.obj", "rushFish");
	modelManager_->LoadModel("resources/Model/Goldfish", "Mesh_Goldfish.obj", "shootingFish");

	// テクスチャ読み込み
	textureManager_->LoadTexture("resources/Texture/Reticle/reticle.png", "reticle");

	// 音声の読み込み
	audioManager_->LoadAudio("resources/Audio/GameBGM.mp3", "GameBGM");


	Logger::Log(Logger::GetStream(), "GameScene: Resources loaded successfully\n");
}

void GameScene::ConfigureOffscreenEffects() {
	/// オフスクリーンレンダラーのエフェクト設定

	// 全てのエフェクトを無効化
	offscreenRenderer_->DisableAllEffects();

	// 必要に応じてエフェクトを有効化

	auto* depthFogEffect = offscreenRenderer_->GetDepthFogEffect();
	if (depthFogEffect) {
		depthFogEffect->SetEnabled(true);
		depthFogEffect->SetFogColor({ 0.02f, 0.08f, 0.25f, 1.0f });
		depthFogEffect->SetFogDistance(0.2f, 260.0f);//レールカメラの時は180程度
	}
	auto* depthOfFieldEffect = offscreenRenderer_->GetDepthOfFieldEffect();
	if (depthOfFieldEffect) {
		depthOfFieldEffect->SetEnabled(true);
		depthOfFieldEffect->SetFocusDistance(2.0f); // フォーカス距離
		depthOfFieldEffect->SetFocusRange(30.0f); // フォーカス範囲

	}

	auto* vignetteEffect = offscreenRenderer_->GetVignetteEffect();
	if (vignetteEffect) {
		vignetteEffect->SetEnabled(true);
		vignetteEffect->SetVignetteStrength(0.6f);
		vignetteEffect->SetVignetteRadius(0.40f);
	}

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

	// レールカメラを作成してカメラコントローラーに登録
	auto railCamera = std::make_unique<RailCamera>();
	railCamera->Initialize(Vector3{ 0.0f, 0.0f, 0.0f }, Vector3{ 0.0f, 0.0f, 0.0f });
	railCamera_ = railCamera.get(); // 参照を保存
	cameraController_->RegisterCamera("rail", std::move(railCamera));

	// デフォルトでレールカメラをアクティブに設定
	cameraController_->SetActiveCamera("rail");
	//railCamera_->StopMovement(); // レールカメラの動きを停止
	railCamera_->SetLoopEnabled(false);//ループを停止

	// レールカメラエディタの初期化
	railCameraEditor_ = std::make_unique<RailCameraEditor>();
	railCameraEditor_->Initialize(railCamera_);

	///*-----------------------------------------------------------------------*///
	///								敵配置エディタの初期化							///
	///*-----------------------------------------------------------------------*///
	enemyPlacementEditor_ = std::make_unique<EnemyPlacementEditor>();
	enemyPlacementEditor_->Initialize(directXCommon_, cameraController_);

	///*-----------------------------------------------------------------------*///
	///								衝突マネージャー								///
	///*-----------------------------------------------------------------------*///
	// 衝突マネージャーの生成
	collisionManager_ = std::make_unique<CollisionManager>();
	// 衝突マネージャーの初期化
	collisionManager_->Initialize();

	// ゲームオブジェクト初期化
	InitializeGameObjects();

	///*-----------------------------------------------------------------------*///	
	///							敵のコマンド読込									///
	///*-----------------------------------------------------------------------*///
	// 敵発生コマンドの初期化
	enemyPopCommand_ = std::make_unique<EnemyPopCommand>();
	// 敵発生データの読み込み
	LoadEnemyPopData();

	///*-----------------------------------------------------------------------*///
	///                         フィールドエディタの初期化                        ///
	///*-----------------------------------------------------------------------*///
#ifdef _DEBUG
	fieldEditor_ = std::make_unique<FieldEditor>();
	fieldEditor_->Initialize(directXCommon_, cameraController_);
#endif

	///*-----------------------------------------------------------------------*///
	///                         フィールドローダーの初期化                        ///
	///*-----------------------------------------------------------------------*///
	fieldLoader_ = std::make_unique<FieldLoader>();
	fieldLoader_->Initialize(directXCommon_);

	// デフォルトのフィールドを読み込み
	fieldLoader_->LoadField("resources/CSV_Data/Field/field.csv");

	// ポストエフェクトの初期設定
	ConfigureOffscreenEffects();

	// BGMをループ再生で開始
	audioManager_->PlayLoop("GameBGM");
	audioManager_->SetVolume("GameBGM", 0.2f);
}

void GameScene::InitializeGameObjects() {
	///*-----------------------------------------------------------------------*///
	///								ライティング								///
	///*-----------------------------------------------------------------------*///
	directionalLight_.Initialize(directXCommon_, Light::Type::DIRECTIONAL);
	directionalLight_.SetDirection({ 0.98f,-1.0f,0.0f });
	directionalLight_.SetIntensity(0.65f);
	///*-----------------------------------------------------------------------*///
	///								プレイヤー									///
	///*-----------------------------------------------------------------------*///
	// ロックオンシステムの初期化
	lockOn_ = std::make_unique<LockOn>();
	lockOn_->Initialize(directXCommon_);

	player_ = std::make_unique<Player>();
	Vector3 playerPosition(0.0f, 0.0f, 30.0f); // 前にずらす
	player_->Initialize(directXCommon_, playerPosition);

	// プレイヤーの親をレールカメラのTransformに設定
	if (railCamera_) {
		player_->SetParent(&railCamera_->GetTransform());
	}

	// プレイヤーにロックオンシステムを設定
	player_->SetLockOn(lockOn_.get());
	///*-----------------------------------------------------------------------*///
	///								天球									///
	///*-----------------------------------------------------------------------*///
	// 天球の生成
	skydome_ = std::make_unique<Skydome>();
	skydome_->Initialize(directXCommon_);

	///*-----------------------------------------------------------------------*///
	///								地面									///
	///*-----------------------------------------------------------------------*///
	// 地面の生成
	ground_ = std::make_unique<Ground>();
	ground_->Initialize(directXCommon_, { 0.0f, -70.0f, 0.0f });
}

void GameScene::Update() {
	// 敵出現コマンドの更新
	UpdateEnemyPopCommands();

	// カメラ更新（CameraControllerが全てのカメラを管理）
	cameraController_->Update();

	// レールカメラをデバッグ時にも更新
	if (cameraController_->GetActiveCameraId() != "rail") {
		railCamera_->Update();
	}
	// ビュープロジェクション行列を取得
	viewProjectionMatrix = cameraController_->GetViewProjectionMatrix();
	viewProjectionMatrixSprite = cameraController_->GetViewProjectionMatrixSprite();

	// ゲームオブジェクト更新
	UpdateGameObjects();

	// ロックオンシステムの更新
	if (player_->IsMultiLockOnMode()) {
		// マルチロックオンモードの場合
		lockOn_->UpdateMultiLockOn(player_.get(), enemies_, viewProjectionMatrix, player_->GetMultiLockOnTargets());
	} else {
		// 通常モードの場合
		lockOn_->Update(player_.get(), enemies_, viewProjectionMatrix);
	}

	// 敵と敵弾の削除
	DeleteDeadEnemies();
	DeleteDeadEnemyBullets();

	///*-----------------------------------------------------------------------*///
	///								衝突判定									///
	///*-----------------------------------------------------------------------*///
#pragma region 衝突判定

	// 衝突マネージャーの更新
	// プレイヤー・敵弾のリストを取得
	const std::list<std::unique_ptr<PlayerBullet>>& playerBullets = player_->GetBullets();
	const std::list<std::unique_ptr<PlayerHomingBullet>>& playerHomingBullets = player_->GetHomingBullets();

	// 衝突マネージャーのリストをクリアする
	collisionManager_->ClearColliderList();

	// Player, Enemyのコライダーを追加する
	collisionManager_->AddCollider(player_.get());
	for (const auto& enemy : enemies_) {
		collisionManager_->AddCollider(enemy.get());
	}

	// Bulletのコライダーを追加する
	for (const auto& bullet : playerBullets) {
		collisionManager_->AddCollider(bullet.get());
	}
	for (const auto& homingBullet : playerHomingBullets) {
		collisionManager_->AddCollider(homingBullet.get());
	}
	for (const auto& bullet : enemyBullets_) {
		collisionManager_->AddCollider(bullet.get());
	}

	// 衝突判定と応答
	collisionManager_->Update();

#pragma endregion

	// レールカメラエディタの更新
	if (railCameraEditor_) {
		railCameraEditor_->Update();
	}

	// 敵配置エディタの更新
	if (enemyPlacementEditor_) {
		enemyPlacementEditor_->Update(viewProjectionMatrix);
	}
	// フィールドエディタの更新（デバッグ時のみ）
#ifdef _DEBUG
	if (fieldEditor_) {
		fieldEditor_->Update(viewProjectionMatrix);
	}
#endif

	// フィールドローダーの更新
	if (fieldLoader_) {
		fieldLoader_->Update(viewProjectionMatrix);
	}

	///*-----------------------------------------------------------------------*///
	///								ゲーム状態判定								///
	///*-----------------------------------------------------------------------*///
#ifdef _DEBUG
	return;
#endif // _DEBUG

	// ゲームオーバー条件: プレイヤーのHPが0以下
	if (player_ && player_->GetCurrentHP() <= 0.0f) {
		// フェードを使った遷移（ヘルパークラスを使用）
		SceneTransitionHelper::FadeToScene("GameoverScene", 1.0f);
		audioManager_->Stop("GameBGM");
		return; // 以降の処理をスキップ

	}

	// ゲームクリア条件: 敵が全滅 かつ レールカメラが終点到達
	if (enemies_.empty() && railCamera_ &&
		railCamera_->GetProgress() >= 1.0f && !railCamera_->IsMoving()) {
		// フェードを使った遷移（ヘルパークラスを使用）
		SceneTransitionHelper::FadeToScene("GameclearScene", 1.0f);
		audioManager_->Stop("GameBGM");
		return; // 以降の処理をスキップ

	}



}

void GameScene::UpdateGameObjects() {
	// プレイヤーにスプライト用ビュープロジェクション行列を設定
	if (player_) {
		player_->SetViewProjectionMatrixSprite(viewProjectionMatrixSprite);
		player_->Update(viewProjectionMatrix);
	}

	// 敵の更新
	for (auto& enemy : enemies_) {
		if (enemy) {
			enemy->Update(viewProjectionMatrix);
		}
	}

	// 敵弾の更新
	for (auto& bullet : enemyBullets_) {
		bullet->Update(viewProjectionMatrix);
	}

	// 背景オブジェクトの更新
	if (skydome_) {
		skydome_->Update(viewProjectionMatrix);
	}
	if (ground_) {
		ground_->Update(viewProjectionMatrix);
	}
}

void GameScene::DrawOffscreen() {
	// 3Dゲームオブジェクトの描画（オフスクリーンに描画）

	// 背景オブジェクトの描画（先に描画）
	if (skydome_) {
		skydome_->Draw(directionalLight_);
	}
	if (ground_) {
		ground_->Draw(directionalLight_);
	}

	// プレイヤーの描画
	if (player_) {
		player_->Draw(directionalLight_);
	}

	// 敵の描画
	for (auto& enemy : enemies_) {
		if (enemy) {
			enemy->Draw(directionalLight_);
		}
	}

	// 敵弾の描画
	for (auto& bullet : enemyBullets_) {
		bullet->Draw(directionalLight_);
	}

	// レールカメラの軌道描画（デバッグ用）
	if (railCamera_) {
		railCamera_->DrawRailTrack(viewProjectionMatrix, directionalLight_);
	}

	// 敵配置エディタの描画（プレビューモデル）
	if (enemyPlacementEditor_) {
		enemyPlacementEditor_->Draw(directionalLight_);
	}

	// フィールドオブジェクトの描画
	if (fieldLoader_) {
		fieldLoader_->Draw(directionalLight_);
	}

	// フィールドエディタの描画（デバッグ時のみ）
#ifdef _DEBUG
	if (fieldEditor_) {
		fieldEditor_->Draw(directionalLight_);
	}
#endif
}

void GameScene::DrawBackBuffer() {
	// UI(スプライトなど)の描画（オフスクリーン外に描画）

	// プレイヤーのUI描画（レティクル）
	if (player_) {
		player_->DrawUI();
	}

	// ロックオンUI描画
	if (player_->IsMultiLockOnMode()) {
		// マルチロックオンモードの場合
		lockOn_->DrawMultiLockOnUI(player_->GetMultiLockOnTargets(), viewProjectionMatrix, viewProjectionMatrixSprite);
	} else {
		// 通常モードの場合
		lockOn_->DrawUI(viewProjectionMatrixSprite);
	}
}

void GameScene::ClearAllEnemyBullets() {
	// 弾が持っているプレイヤーポインタを無効化してから削除
	for (auto& bullet : enemyBullets_) {
		if (bullet) {
			bullet->SetPlayer(nullptr); // プレイヤーポインタを無効化
		}
	}
	enemyBullets_.clear(); // すべて削除
}

void GameScene::ClearAllEnemies() {
	// 敵が持っているプレイヤーポインタを無効化してから削除
	for (auto& enemy : enemies_) {
		if (enemy) {
			enemy->SetPlayer(nullptr);   // プレイヤーポインタを無効化
			enemy->SetGameScene(nullptr); // ゲームシーンポインタも無効化
			enemy->ClearTimeCalls();      // 時限発動をクリア
		}
	}
	enemies_.clear(); // すべて削除
}

void GameScene::DebugStartGame() {
	// 1. 全ての敵と敵弾をクリア
	ClearAllEnemies();
	ClearAllEnemyBullets();

	// 2. 敵発生コマンドをリセット
	if (enemyPopCommand_) {
		enemyPopCommand_->Reset();
	}
	isWait_ = false;
	waitTimer_ = 0;

	// 3. CSVから敵配置データを再読み込み
	LoadEnemyPopData();

	// 4. レールカメラを初期位置にリセット
	if (railCamera_) {
		railCamera_->ResetPosition();
		railCamera_->SetProgress(0.0f); // 進行度を0%に設定

		// レールカメラの設定確認・調整
		if (!railCamera_->IsLoopEnabled()) {
			railCamera_->SetLoopEnabled(true); // ループを有効化
		}
	}

	// 5. アクティブカメラをレールカメラに切り替え
	if (cameraController_) {
		cameraController_->SetActiveCamera("rail");
	}

	// 6. レールカメラの動きを開始
	if (railCamera_) {
		railCamera_->StartMovement();
	}

	// 7. ログ出力
	Logger::Log(Logger::GetStream(), "GameScene: Game Started! Rail camera activated and moving.\n");
	Logger::Log(Logger::GetStream(), "GameScene: Enemies cleared, CSV reloaded, camera reset to start position.\n");
}

void GameScene::ImGui() {
#ifdef _DEBUG

	// レールカメラエディタのImGui（統合されたUI）
	if (railCameraEditor_) {
		railCameraEditor_->ImGui();
	}

	// 敵配置エディタのImGui
	if (enemyPlacementEditor_) {
		enemyPlacementEditor_->ImGui();
	}
	//タイマーの初期化
	if (ImGui::Button("Reset Enemy Pop Commands")) {
		// 敵発生コマンドをリセット
		enemyPopCommand_->Reset();
		LoadEnemyPopData(); // 再読み込み
		// 待機中フラグ
		isWait_ = false;
		// 待機タイマー
		waitTimer_ = 0;
	}

	// エディタからCSVを再読み込み
	if (ImGui::Button("Reload CSV from Editor")) {
		LoadEnemyPopData();
		if (enemyPlacementEditor_) {
			enemyPlacementEditor_->LoadFromEnemyPopCommand(enemyPopCommand_.get());
		}
	}

	// フィールドエディタのImGui
	if (fieldEditor_) {
		fieldEditor_->ImGui();
	}

	// フィールドローダーの情報表示
	if (fieldLoader_) {
		ImGui::Text("Field Loader");
		ImGui::Text("Field Loaded: %s", fieldLoader_->IsFieldLoaded() ? "YES" : "NO");
		if (ImGui::Button("Reload Field")) {
			fieldLoader_->LoadField("resources/CSV_Data/Field/field.csv");
		}

		if (ImGui::Button("Clear Field")) {
			fieldLoader_->ClearField();
		}

		ImGui::Spacing();
	}
	// プレイヤーのImGui
	ImGui::Text("Player");
	player_->ImGui();
	ImGui::Spacing();

	// 敵のImGui
	ImGui::Text("Enemies Count: %zu", enemies_.size());
	int enemyIndex = 0;
	for (auto& enemy : enemies_) {
		ImGui::PushID(enemyIndex);
		enemy->ImGui();
		ImGui::PopID();
		enemyIndex++;
	}
	ImGui::Spacing();

	// 敵弾のImGui
	ImGui::Text("Enemy Bullets Count: %zu", enemyBullets_.size());
	ImGui::Spacing();

	// 地面のImGui
	ImGui::Text("Ground");
	ground_->ImGui();
	ImGui::Spacing();

	// 天球のImGui
	ImGui::Text("Skydome");
	skydome_->ImGui();
	ImGui::Spacing();

	// ライトのImGui
	ImGui::Text("Lighting");
	directionalLight_.ImGui("DirectionalLight");
	ImGui::Spacing();

	// ゲーム制御セクション
	if (ImGui::CollapsingHeader("Game Control")) {
		// Start Gameボタン（目立つように大きく表示）
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 0.8f));        // 緑色
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));  // ホバー時
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.6f, 0.1f, 1.0f));   // 押下時

		if (ImGui::Button("START GAME", ImVec2(200, 50))) {
			DebugStartGame();
		}

		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::Text("Clear enemies, load CSV, reset rail camera and start");

		ImGui::Separator();
	}

	// 敵生成関連のImGui
	ImGui::Text("Enemy Spawn System");
	ImGui::Text("Is Wait: %s", isWait_ ? "YES" : "NO");
	ImGui::Text("Wait Timer: %d", waitTimer_);
	if (ImGui::Button("Create Test Normal Enemy")) {
		CreateEnemy(Vector3{ 50.0f, 10.0f, 200.0f }, EnemyType::Normal, EnemyPattern::Straight);
	}
	if (ImGui::Button("Create Test Rushing Fish")) {
		CreateEnemy(Vector3{ -30.0f, 15.0f, 150.0f }, EnemyType::RushingFish, EnemyPattern::Homing);
	}
	if (ImGui::Button("Create Test Shooting Fish")) {
		CreateEnemy(Vector3{ 30.0f, 20.0f, 180.0f }, EnemyType::ShootingFish, EnemyPattern::Shooting);
	}
	ImGui::SameLine();

#endif
}

void GameScene::AddEnemyBullet(std::unique_ptr<EnemyBullet> enemyBullet) {
	enemyBullets_.push_back(std::move(enemyBullet));
}

void GameScene::LoadEnemyPopData() {
	// CSVファイルから敵発生データを読み込み
	if (!enemyPopCommand_->LoadFromCSV("resources/CSV_Data/Enemy_Pop/enemyPop.csv")) {
		Logger::Log(Logger::GetStream(), "GameScene: Failed to load enemy pop data\n");
	} else {
		// 敵配置エディタにも読み込んだデータを反映 
		if (enemyPlacementEditor_) {
			enemyPlacementEditor_->LoadFromEnemyPopCommand(enemyPopCommand_.get());
		}
	}
}

void GameScene::UpdateEnemyPopCommands() {
	/// 待機処理
	if (isWait_) {
		// 待機時間を減らす
		waitTimer_--;
		if (waitTimer_ <= 0) {
			// 待機完了
			isWait_ = false;
		}
		return;
	}

	// コマンド実行ループ
	EnemyPopCommandType commandType;
	EnemyPopData popData;
	WaitData waitData;

	while (enemyPopCommand_->GetNextCommand(commandType, popData, waitData)) {
		switch (commandType) {
		case EnemyPopCommandType::POP:
			// 敵を発生させる
			CreateEnemy(popData.position, popData.enemyType, popData.pattern);
			break;

		case EnemyPopCommandType::WAIT:
			// 待機開始
			isWait_ = true;
			waitTimer_ = waitData.waitTime;
			// コマンドループを抜ける
			return;
		}
	}
}

void GameScene::CreateEnemy(const Vector3& position, EnemyType enemyType, EnemyPattern pattern) {
	std::unique_ptr<BaseEnemy> enemy;

	// 敵の種類に応じて生成
	switch (enemyType) {
	case EnemyType::Normal:
		enemy = std::make_unique<Enemy>();
		break;
	case EnemyType::RushingFish:
		enemy = std::make_unique<RushingFishEnemy>();
		break;
	case EnemyType::ShootingFish:
		enemy = std::make_unique<ShootingFishEnemy>();
		break;
	default:
		// デフォルトは通常の敵
		enemy = std::make_unique<Enemy>();
		break;
	}

	// 敵キャラの初期化
	enemy->Initialize(directXCommon_, position, pattern);
	// 敵キャラにプレイヤーのアドレスを渡す
	enemy->SetPlayer(player_.get());
	// 敵キャラにGameSceneのアドレスを渡す
	enemy->SetGameScene(this);
	enemies_.push_back(std::move(enemy));
}

void GameScene::DeleteDeadEnemies() {
	// デスフラグが立っている敵を削除する
	enemies_.remove_if([](const std::unique_ptr<BaseEnemy>& enemy) {
		return enemy->IsDead();
		});
}

void GameScene::DeleteDeadEnemyBullets() {
	// デスフラグが立っている弾を削除する
	enemyBullets_.remove_if([](const std::unique_ptr<EnemyBullet>& bullet) {
		return bullet->IsDead();
		});
}

void GameScene::Finalize() {

	// 安全のため、再度削除処理を実行
	ClearAllEnemyBullets();
	ClearAllEnemies();

	// 敵発生コマンドのリセット
	if (enemyPopCommand_) {
		enemyPopCommand_->Reset();
	}
	isWait_ = false;
	waitTimer_ = 0;

	// レールカメラエディタのリソース解放
	if (railCameraEditor_) {
		railCameraEditor_.reset();
	}

	// 敵配置エディタのリソース解放
	if (enemyPlacementEditor_) {
		enemyPlacementEditor_.reset();
	}

	// フィールド関連のリソース解放
#ifdef _DEBUG
	if (fieldEditor_) {
		fieldEditor_.reset();
	}
#endif

	if (fieldLoader_) {
		fieldLoader_->ClearField();
		fieldLoader_.reset();
	}

	// プレイヤーを明示的にリセット
	if (player_) {
		player_.reset();
	}

	// レールカメラのリソースを明示的に解放
	if (railCamera_) {
		railCamera_->ReleaseResources();
		railCamera_ = nullptr;
	}

	// CameraControllerからRailCameraを明示的に削除
	if (cameraController_) {
		// デバッグカメラでない場合は通常カメラに切り替えてからRailCameraを削除
		if (cameraController_->GetActiveCameraId() == "rail") {
			cameraController_->SetActiveCamera("normal");
		}
		cameraController_->UnregisterCamera("rail");
	}
}