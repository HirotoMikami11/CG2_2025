#include "Game.h"



Game::Game() :
	textureManager_(nullptr),
	modelManager_(nullptr),
	sceneManager_(nullptr) {
}

Game::~Game() = default;

void Game::Initialize() {
	// シーンマネージャーの初期化
	sceneManager_ = SceneManager::GetInstance();
	sceneManager_->Initialize();

	// トランジションマネージャーの初期化（エフェクト管理）
	transitionManager_ = TransitionManager::GetInstance();
	transitionManager_->Initialize();

	// リソースマネージャーの初期化
	modelManager_ = ModelManager::GetInstance();
	textureManager_ = TextureManager::GetInstance();

	// カスタムトランジションエフェクトを登録
	RegisterTransitionEffects();

	// シーンの初期化
	InitializeScenes();


}

void Game::InitializeScenes() {
	// Sceneの登録
	auto demoScene = std::make_unique<DemoScene>();
	sceneManager_->RegisterScene("DemoScene", std::move(demoScene));

	// 将来的に追加するシーン
	// auto debugScene = std::make_unique<DebugScene>();
	// sceneManager_->RegisterScene("DebugScene", std::move(debugScene));

	// (初期化時に一度だけ)既に登録されているシーンのリソースを読み込み
	sceneManager_->LoadAllSceneResources();
	// デフォルトシーンを設定（最初に表示するシーン）
	sceneManager_->ChangeScene("DemoScene");
}

void Game::RegisterTransitionEffects()
{
	// スライドエフェクト(左から)を登録
	auto slideLeft = std::make_unique<SlideEffect>(SlideEffect::Direction::Left);
	slideLeft->Initialize(Engine::GetInstance()->GetDirectXCommon());
	transitionManager_->RegisterEffect("slide_left", std::move(slideLeft));

	// スライドエフェクト(右から)を登録
	auto slideRight = std::make_unique<SlideEffect>(SlideEffect::Direction::Right);
	slideRight->Initialize(Engine::GetInstance()->GetDirectXCommon());
	transitionManager_->RegisterEffect("slide_right", std::move(slideRight));

}

void Game::Update() {

	// トランジションマネージャーの更新（エフェクトの更新）
	if (transitionManager_) {
		transitionManager_->Update(1.0f / 60.0f); //時間
	}

	// シーンマネージャーの更新
	if (sceneManager_) {
		sceneManager_->Update();
	}



	// シーンマネージャーのImGui更新
	if (sceneManager_) {
		sceneManager_->ImGui();
	}
	// トランジションマネージャーのImGui
	if (transitionManager_) {
	//　TODO:imgui必要に応じて作成
	// 	transitionManager_->ImGui();
	}
}

void Game::DrawOffscreen() {
	// シーンマネージャーの3D描画（オフスクリーン内）
	if (sceneManager_) {
		sceneManager_->DrawOffscreen();
	}
}

void Game::DrawBackBuffer() {
	// シーンマネージャーのUI描画（オフスクリーン外）
	if (sceneManager_) {
		sceneManager_->DrawBackBuffer();
	}

	// トランジションエフェクトの描画（最前面）
	if (transitionManager_) {
		transitionManager_->Draw();
	}
}

void Game::Finalize() {
	// トランジションマネージャーの終了処理
	if (transitionManager_) {
		transitionManager_->Finalize();
	}

	// シーンマネージャーの終了処理
	if (sceneManager_) {
		sceneManager_->Finalize();
	}
}