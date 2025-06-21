#include "GameScene.h"
#include "Managers/ImGuiManager.h" 

GameScene::GameScene()
	: BaseScene("GameScene") // シーン名を設定
	, cameraController_(nullptr)
	, directXCommon_(nullptr)
	, offscreenRenderer_(nullptr) {
}

GameScene::~GameScene() {
	// MapChipFieldの解放
	if (mapChipField_) {
		delete mapChipField_;
		mapChipField_ = nullptr;
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
	cameraController_->Initialize({ 0.0f, 0.0f, -50.0f });

	// ゲームオブジェクト初期化
	InitializeGameObjects();
}

void GameScene::InitializeGameObjects() {
	///*-----------------------------------------------------------------------*///
	///									プレイヤー								///
	///*-----------------------------------------------------------------------*///


	//初期化
	player_ = std::make_unique<Player>();
	player_->Initialize();

	///*-----------------------------------------------------------------------*///
	///										天球									///
	///*-----------------------------------------------------------------------*///


	//初期化
	skydome_ = std::make_unique<Skydome>();
	skydome_->Initialize();

	///*-----------------------------------------------------------------------*///
	///									ブロック									///
	///*-----------------------------------------------------------------------*///
	// マップチップフィールドの生成・初期化
	mapChipField_ = new MapChipField();
	mapChipField_->Initialize();
	mapChipField_->LoadMapChipCSV("resources/Mapchips/blocks.csv");

	// マップチップに合わせたブロックの生成
	GenerateBlocks();


	///*-----------------------------------------------------------------------*///
	///									ライト									///
	///*-----------------------------------------------------------------------*///
	directionalLight_.Initialize(directXCommon_, Light::Type::DIRECTIONAL);
	directionalLight_.SetDirection({ 0.0f,-0.7f,0.5f });
}

void GameScene::Update() {

	///天球の更新
	skydome_->Update(viewProjectionMatrix);
	///自キャラの更新
	player_->Update(viewProjectionMatrix);
	///敵の更新

	///カメラの更新
	cameraController_->Update();
	// 行列更新
	viewProjectionMatrix = cameraController_->GetViewProjectionMatrix();
	viewProjectionMatrixSprite = cameraController_->GetViewProjectionMatrixSprite();


	///追従設定など？

	///ブロックの更新
	for (auto& blockLine : blocks_) {
		for (auto& block : blockLine) {
			if (!block) continue; // nullptrの場合はスキップ
			block->Update(viewProjectionMatrix);
		}
	}
	///全ての当たり判定

	///フェードの終了


}









void GameScene::Draw() {
	// ゲームオブジェクトの描画（オフスクリーンに描画）
	DrawGameObjects();
}

void GameScene::DrawGameObjects() {
	player_->Draw(directionalLight_);
	skydome_->Draw(directionalLight_);


	// ブロックの描画
	for (auto& blockLine : blocks_) {
		for (auto& block : blockLine) {
			if (!block) continue; // nullptrの場合はスキップ
			block->Draw(directionalLight_);
		}
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
				blocks_[i][j]->Initialize(directXCommon_, "resources/Model/Block", "block.obj");

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






void GameScene::OnEnter() {
	// ゲームシーンに入る時の処理
	cameraController_->Initialize({ 0.0f, 0.0f, -50.0f });
}

void GameScene::OnExit() {
	// ゲームシーンから出る時の処理
}

void GameScene::ImGui() {
#ifdef _DEBUG

	ImGui::Spacing();

	player_->ImGui();
	skydome_->ImGui();
	// ライトのImGui
	ImGui::Text("Lighting");
	directionalLight_.ImGui("DirectionalLight");
#endif
}

void GameScene::Finalize() {
	// unique_ptrで自動的に解放される
}