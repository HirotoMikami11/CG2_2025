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

	// モデルファイルを読み込み
	modelManager_->LoadModel("resources/Model/Ground", "ground.obj", "ground");
	modelManager_->LoadModel("resources/Model/Skydome", "skydome.obj", "skydome");
	modelManager_->LoadModel("resources/Model/Camera", "camera.obj", "camera");
	// プレイヤー関連
	modelManager_->LoadModel("resources/Model/Player", "player.obj", "player");
	modelManager_->LoadModel("resources/Model/PlayerBullet", "playerBullet.obj", "playerBullet");
	modelManager_->LoadModel("resources/Model/PlayerHomingBullet", "playerHomingBullet.obj", "playerHomingBullet");
	// 敵関連
	modelManager_->LoadModel("resources/Model/EnemyBullet", "enemyBullet.obj", "enemyBullet");

	// テクスチャ読み込み
	textureManager_->LoadTexture("resources/Texture/Reticle/reticle.png", "reticle");

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
	//cameraController_->SetActiveCamera("rail");
	railCamera_->StopMovement(); // レールカメラの動きを停止

	// レールカメラエディタの初期化
	railCameraEditor_ = std::make_unique<RailCameraEditor>();
	railCameraEditor_->Initialize(railCamera_);

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
	// 敵発生データの読み込み
	LoadEnemyPopData();

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
	DrawGameObjects();
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

void GameScene::DrawGameObjects() {
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
	// デバッグカメラ時でも常に描画されるように修正
	if (railCamera_) {
		railCamera_->DrawRailTrack(viewProjectionMatrix, directionalLight_);
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

void GameScene::ImGui() {
#ifdef _DEBUG

	// レールカメラエディタのImGui
	if (railCameraEditor_) {
		railCameraEditor_->ImGui();
	}

	// プレイヤーのImGui
	ImGui::Text("Player");
	player_->ImGui();
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

	// 敵生成関連のImGui
	ImGui::Text("Enemy Spawn System");
	ImGui::Text("Is Wait: %s", isWait_ ? "YES" : "NO");
	ImGui::Text("Wait Timer: %d", waitTimer_);
	if (ImGui::Button("Create Test Enemy")) {
		CreateEnemy(Vector3{ 50.0f, 10.0f, 200.0f }, EnemyPattern::Straight);
	}

	ImGui::SameLine();
	//タイマーの初期化
	if (ImGui::Button("Reset Enemy Pop Commands")) {
		// 敵発生コマンドのストリームをリセット
		enemyPopCommands.clear();
		enemyPopCommands.seekg(0);
		LoadEnemyPopData(); // 再読み込み
		// 待機中フラグ
		isWait_ = false;
		// 待機タイマー
		waitTimer_ = 0;
	}

	ImGui::Spacing();

#endif
}

void GameScene::AddEnemyBullet(std::unique_ptr<EnemyBullet> enemyBullet) {
	enemyBullets_.push_back(std::move(enemyBullet));
}

void GameScene::LoadEnemyPopData() {
	// ファイルを開く
	std::ifstream file;
	file.open("resources/CSV_Data/Enemy_Pop/enemyPop.csv");

	if (!file.is_open()) {
		Logger::Log(Logger::GetStream(), "GameScene: Failed to open enemyPop.csv\n");
		return;
	}

	// ファイルの内容を丸ごと文字列ストリームにコピー
	enemyPopCommands << file.rdbuf();

	// ファイルを閉じる
	file.close();
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

	// 一行分の文字列を入れる変数
	std::string line;

	// コマンド実行ループ(コマンドは一行単位なので一行づつ取り出す)
	while (std::getline(enemyPopCommands, line)) {
		// 一行分の文字列をストリームに変換して解析しやすくする。
		std::istringstream line_stream(line);

		// 一行の中から[,]が現れるまでwordに入れる
		std::string word;
		// カンマ区切りで行の先頭文字列を取得
		std::getline(line_stream, word, ',');

		//"//"から始まる行はコメントなのでスキップ
		if (word.find("//") == 0) {
			// スキップする
			continue;
		}

		// POPコマンド
		if (word.find("POP") == 0) {
			// x座標
			std::getline(line_stream, word, ',');
			float x = (float)std::atof(word.c_str());

			// y座標
			std::getline(line_stream, word, ',');
			float y = (float)std::atof(word.c_str());

			// z座標
			std::getline(line_stream, word, ',');
			float z = (float)std::atof(word.c_str());

			// Patternの指定
			std::getline(line_stream, word, ',');
			int patternValue = atoi(word.c_str());
			// 値をPatternに変更
			//   パターン値を列挙型に変換
			EnemyPattern pattern = EnemyPattern::Straight; // デフォルト
			switch (patternValue) {
			case 0:
				pattern = EnemyPattern::Straight;
				break;
			case 1:
				pattern = EnemyPattern::LeaveLeft;
				break;
			case 2:
				pattern = EnemyPattern::LeaveRight;
				break;
			default:
				pattern = EnemyPattern::Straight; // 不正な値の場合はデフォルト
				break;
			}

			// 敵を発生させる
			CreateEnemy(Vector3(x, y, z), pattern);

			// WAITコマンド
		} else if (word.find("WAIT") == 0) {
			std::getline(line_stream, word, ',');
			// 待ち時間
			int32_t waitTime = atoi(word.c_str());
			// 待機開始
			isWait_ = true;
			waitTimer_ = waitTime;

			// コマンドループを抜ける
			break;
		}
	}
}

void GameScene::CreateEnemy(const Vector3& position, EnemyPattern pattern) {
	// 敵キャラの生成
	auto enemy = std::make_unique<Enemy>();
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
	enemies_.remove_if([](const std::unique_ptr<Enemy>& enemy) {
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

	// 敵発生コマンドのストリームをリセット
	enemyPopCommands.clear();
	enemyPopCommands.seekg(0);
	isWait_ = false;
	waitTimer_ = 0;

	// レールカメラエディタのリソース解放
	if (railCameraEditor_) {
		railCameraEditor_.reset();
	}


	// プレイヤーを明示的にリセット
	if (player_) {
		player_.reset();
	}

	////
	///悩まされたので備忘録TODO:カメラを後図家するときも簡単に消せるような仕組みにする
	////

	//CameraControllerがシングルトンのため、他のオブジェクトより後に破棄される
	//RailCameraがModel3D → MaterialGroup → Material → DirectX12リソースという深い階層でリソースを保持
	//アプリケーション終了時にDirectXCommonが先に破棄され、その後でDirectX12リソースを解放しようとしてエラーが発生

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

