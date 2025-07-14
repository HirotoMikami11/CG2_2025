#include "GameScene.h"
#include "Managers/ImGui/ImGuiManager.h" 
#include "Managers/Scene/SceneManager.h"

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

GameScene::~GameScene() {
	// 敵の解放
	enemies_.clear(); // unique_ptrなので自動的に解放される



	// MapChipFieldの解放
	if (mapChipField_) {
		delete mapChipField_;
		mapChipField_ = nullptr;
	}

}


void GameScene::LoadResources() {
	// リソースの読み込み
	Logger::Log(Logger::GetStream(), "TitleScene: Loading resources...\n");

	// リソースマネージャーの取得
	modelManager_ = ModelManager::GetInstance();
	textureManager_ = TextureManager::GetInstance();

	// モデルを事前読み込み
	modelManager_->LoadModel("resources/Model/Block", "block.obj", "block");
	modelManager_->LoadModel("resources/Model/Player", "player.obj", "player");
	modelManager_->LoadModel("resources/Model/Enemy", "enemy.obj", "enemy");
	modelManager_->LoadModel("resources/Model/Skydome", "skydome.obj", "skydome");
	modelManager_->LoadModel("resources/Model/DeathParticles", "deathParticles.obj", "deathParticle");

	Logger::Log(Logger::GetStream(), "TitleScene: Resources loaded successfully\n");



}


void GameScene::Initialize() {
	// ゲームプレイフェーズから開始
	phase_ = Phase::kPlay;


	// システム参照の取得
	directXCommon_ = Engine::GetInstance()->GetDirectXCommon();
	offscreenRenderer_ = Engine::GetInstance()->GetOffscreenRenderer();

	// ゲームオブジェクト初期化
	InitializeGameObjects();
	///*-----------------------------------------------------------------------*///
	///								カメラの初期化									///
	///*-----------------------------------------------------------------------*///
	cameraController_ = CameraController::GetInstance();
	cameraController_->Initialize({ 0.0f, 0.0f, -50.0f });

	///*-----------------------------------------------------------------------*///
	///								ゲームカメラ									///
	///*-----------------------------------------------------------------------*///
  // ゲームカメラの作成・設定
	auto gameCamera = std::make_unique<GameCamera>();
	gameCamera->Initialize({ 0.0f, 0.0f, -50.0f });

	// 移動範囲設定
	GameCamera::Rect movableArea = {
		15.0f,									// left（左端制限）
		mapChipField_->GetMapSize().x - 16.0f,	// right（右端制限）
		7.5f,									// bottom（下端制限）
		mapChipField_->GetMapSize().y - 7.5f	// top（上端制限）
	};
	gameCamera->SetMovableArea(movableArea);
	gameCamera->SetTarget(player_.get());

	// raw pointerで参照を保存（所有権移譲前に）
	gameCamera_ = gameCamera.get();

	// CameraControllerに登録（所有権を移譲）
	cameraController_->RegisterCamera("game", std::move(gameCamera));
	cameraController_->SetActiveCamera("game");


	// カメラを初期位置にリセット
	gameCamera_->Reset();
}

void GameScene::InitializeGameObjects() {

	///*-----------------------------------------------------------------------*///
	///									ブロック									///
	///*-----------------------------------------------------------------------*///
	// マップチップフィールドの生成・初期化
	mapChipField_ = new MapChipField();
	mapChipField_->Initialize();
	mapChipField_->LoadMapChipCSV("resources/Mapchips/blocks_short.csv");

	// マップチップに合わせたブロックの生成
	GenerateBlocks();


	///*-----------------------------------------------------------------------*///
	///									プレイヤー								///
	///*-----------------------------------------------------------------------*///


	//初期化
	player_ = std::make_unique<Player>();
	player_->Initialize();
	player_->SetMapChipField(mapChipField_);
	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(2, 18);
	player_->SetPosition(playerPosition);

	///*-----------------------------------------------------------------------*///
	///									敵										///
	///*-----------------------------------------------------------------------*///
	// 敵の生成・初期化(関数)
	InitializeEnemies();

	///*-----------------------------------------------------------------------*///
	///										天球									///
	///*-----------------------------------------------------------------------*///


	//初期化
	skydome_ = std::make_unique<Skydome>();
	skydome_->Initialize();


	///*-----------------------------------------------------------------------*///
	///									ライト									///
	///*-----------------------------------------------------------------------*///
	directionalLight_.Initialize(directXCommon_, Light::Type::DIRECTIONAL);
	directionalLight_.SetDirection({ 0.0f,-0.7f,0.5f });
}

void GameScene::Update() {
	///カメラの更新
	cameraController_->Update();
	// 行列更新
	viewProjectionMatrix = cameraController_->GetViewProjectionMatrix();
	viewProjectionMatrixSprite = cameraController_->GetViewProjectionMatrixSprite();

	// フェーズの切り替えチェック
	ChangePhase();

	// フェーズ別の更新処理
	switch (phase_) {
	case Phase::kPlay:
		///天球の更新
		skydome_->Update(viewProjectionMatrix);

		///自キャラの更新
		player_->Update(viewProjectionMatrix);

		///敵の更新
		// 敵の更新を追加
		for (auto& enemy : enemies_) {
			enemy->Update(viewProjectionMatrix);
		}

		///ブロックの更新
		for (auto& blockLine : blocks_) {
			for (auto& block : blockLine) {
				if (!block) continue; // nullptrの場合はスキップ
				block->Update(viewProjectionMatrix);
			}
		}

		///全ての当たり判定
		CheckAllCollision();

		break;

	case Phase::kDeath:
		///天球の更新
		skydome_->Update(viewProjectionMatrix);

		///敵の更新（複数）
		for (auto& enemy : enemies_) {
			enemy->Update(viewProjectionMatrix);
		}

		///ブロックの更新
		for (auto& blockLine : blocks_) {
			for (auto& block : blockLine) {
				if (!block) continue; // nullptrの場合はスキップ
				block->Update(viewProjectionMatrix);
			}
		}

		// デスパーティクルの更新
		if (deathParticles_ != nullptr) {
			deathParticles_->Update(viewProjectionMatrix);
		}


		// フェーズの切り替えチェック
		ChangePhase();
		break;

	default:
		break;
	}
}









void GameScene::Draw() {
	// ゲームオブジェクトの描画（オフスクリーンに描画）
	DrawGameObjects();
}

void GameScene::DrawGameObjects() {

	skydome_->Draw(directionalLight_);
	player_->Draw(directionalLight_);
	for (auto& enemy : enemies_) {
		enemy->Draw(directionalLight_);
	}

	// ブロックの描画
	for (auto& blockLine : blocks_) {
		for (auto& block : blockLine) {
			if (!block) continue; // nullptrの場合はスキップ
			block->Draw(directionalLight_);
		}
	}

	// デスパーティクルの描画（kDeathフェーズでのみ）
	if (phase_ == Phase::kDeath && deathParticles_ != nullptr) {
		deathParticles_->Draw(directionalLight_);
	}
}

void GameScene::InitializeEnemies()
{
	// 敵の生成・初期化
	for (int32_t i = 0; i < kEnemyCount; i++) {
		auto newEnemy = std::make_unique<Enemy>();

		// マップチップがある場合の位置設定
		Vector3 enemyPosition;
		if (mapChipField_) {
			enemyPosition = mapChipField_->GetMapChipPositionByIndex(17 + i, 18);
		}

		newEnemy->Initialize(enemyPosition);
		enemies_.push_back(std::move(newEnemy));
	}
}

void GameScene::GenerateBlocks()
{
	// マップサイズを取得
	uint32_t numBlockVertical = mapChipField_->GetNumBlockkVirtical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockkHorizontal();

	// 列数を設定（縦方向のブロック数）
	blocks_.resize(numBlockVertical);
	for (uint32_t i = 0; i < numBlockVertical; ++i) {
		// 1列の要素数を設定（横方向のブロック数）
		blocks_[i].resize(numBlockHorizontal);
	}

	// マップチップに基づいてブロックを生成
	for (uint32_t i = 0; i < numBlockVertical; ++i) {
		for (uint32_t j = 0; j < numBlockHorizontal; ++j) {
			// マップチップがブロックタイプの場合のみ生成
			if (mapChipField_->GetMapChipTypeByIndex(j, i) == MapChipType::kBlock) {
				// ブロックを生成する
				blocks_[i][j] = std::make_unique<Model3D>();
				blocks_[i][j]->Initialize(directXCommon_, "block");

				// マップチップの位置に基づいて座標を設定
				Vector3 blockPosition = mapChipField_->GetMapChipPositionByIndex(j, i);
				blocks_[i][j]->SetPosition(blockPosition);
			} else {
				// ブロックでない場合はnullptrを設定
				blocks_[i][j] = nullptr;
			}
		}
	}

}

void GameScene::CheckAllCollision()
{

	// プレイヤーが存在しない場合は処理しない
	if (!player_) return;

	// 判定対象1,2の座標
	AABB aabb1, aabb2;

	// 自キャラの座標
	aabb1 = player_->GetAABB();

	// 自キャラとすべての敵の当たり判定
	for (auto& enemy : enemies_) {
		// 敵の座標
		aabb2 = enemy->GetAABB();

		// AABB同士の交差判定
		if (IsCollision(aabb1, aabb2)) {
			// 敵の衝突時コールバックを呼び出す
			enemy->OnCollision(player_.get());
			// 自キャラの衝突時コールバックを呼び出す
			player_->OnCollision(enemy.get());
		}
	}


}



void GameScene::ChangePhase() {
	switch (phase_) {
		// ゲームフェーズ内の処理
	case Phase::kPlay:
		if (player_->IsDead()) {
			// 死亡演出フェーズに切り替え
			phase_ = Phase::kDeath;
			// 自キャラの座標を取得
			Vector3 deathParticlesPosition = player_->GetPosition();
			// 自キャラの座標にデスパーティクルを生成、初期化
			deathParticles_ = std::make_unique<DeathParticles>();
			deathParticles_->Initialize(deathParticlesPosition);
		}
		break;

		// 死亡フェーズ内の処理
	case Phase::kDeath:
		// パーティクルの演出が終了したらタイトルシーンに戻る
		if (deathParticles_ && deathParticles_->IsFinished()) {
			// SceneManagerを取得してタイトルシーンに切り替え
			SceneManager::GetInstance()->FadeOutToScene("TitleScene", 1.0f);

		}
		break;

	default:
		break;
	}
}



void GameScene::OnEnter() {
	//// ゲームシーンに入る時の処理
	//cameraController_->Initialize({ 0.0f, 0.0f, -50.0f });
	//// ゲームカメラもリセット
	//if (gameCamera_) {
	//	gameCamera_->Reset();
	//}

}

void GameScene::OnExit() {
	// ゲームシーンから出る時の処理

}

void GameScene::ImGui() {
#ifdef _DEBUG

	ImGui::Spacing();

	player_->ImGui();
	// 敵のデバッグ表示を追加
	if (ImGui::TreeNode("Enemies")) {
		ImGui::Text("Enemy Count: %zu", enemies_.size());
		int enemyIndex = 0;
		for (auto& enemy : enemies_) {
			if (ImGui::TreeNode(("Enemy " + std::to_string(enemyIndex)).c_str())) {
				enemy->ImGui();
				ImGui::TreePop();
			}
			enemyIndex++;
		}
		ImGui::TreePop();
	}
	skydome_->ImGui();

	// ライトのImGui
	ImGui::Text("Lighting");
	directionalLight_.ImGui("DirectionalLight");
#endif
}

void GameScene::Finalize() {
	cameraController_->UnregisterCamera("game");
	gameCamera_ = nullptr; // raw pointerをクリア
	// unique_ptrで自動的に解放される
	enemies_.clear();
	// MapChipFieldの解放
	if (mapChipField_) {
		delete mapChipField_;
		mapChipField_ = nullptr;
	}
}