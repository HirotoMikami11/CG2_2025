#include "Game.h"

Game::Game() = default;
Game::~Game() = default;

void Game::Initialize() {
	// ゲームシーンの初期化
	gameScene_ = std::make_unique<GameScene>();
	gameScene_->Initialize();
}

void Game::Update() {
	// ゲームシーンの更新
	if (gameScene_) {
		gameScene_->Update();
	}
}

void Game::Draw() {
	// ゲームシーンの描画
	if (gameScene_) {
		gameScene_->Draw();
	}
}

void Game::Finalize() {
	// ゲームシーンの終了処理
	if (gameScene_) {
		gameScene_->Finalize();
		gameScene_.reset();
	}
}