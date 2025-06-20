#include "Game.h"


Game::Game() = default;
Game::~Game() = default;

void Game::Initialize() {
	// シーンマネージャーの初期化
	sceneManager_ = std::make_unique<SceneManager>();
	sceneManager_->Initialize();

	// シーンの初期化
	InitializeScenes();
}

void Game::InitializeScenes() {
	// GameSceneの登録
	auto demoScene = std::make_unique<DemoScene>();
	sceneManager_->RegisterScene("DemoScene", std::move(demoScene));

	auto gameScene = std::make_unique<GameScene>();
	sceneManager_->RegisterScene("GameScene", std::move(gameScene));

	auto titleScene = std::make_unique<TitleScene>();
	sceneManager_->RegisterScene("TitleScene", std::move(titleScene));


	// 将来的に追加するシーン
	// auto debugScene = std::make_unique<DebugScene>();
	// sceneManager_->RegisterScene("DebugScene", std::move(debugScene));

	// デフォルトシーンを設定（最初に表示するシーン）
	sceneManager_->ChangeScene("DemoScene");
}

void Game::Update() {
	// シーンマネージャーの更新
	if (sceneManager_) {
		sceneManager_->Update();
	}

	// シーンマネージャーのImGui更新
	if (sceneManager_) {
		sceneManager_->ImGui();
	}
}

void Game::Draw() {
	// シーンマネージャーの描画
	if (sceneManager_) {
		sceneManager_->Draw();
	}
}

void Game::Finalize() {
	// シーンマネージャーの終了処理
	if (sceneManager_) {
		sceneManager_->Finalize();
		sceneManager_.reset();
	}
}