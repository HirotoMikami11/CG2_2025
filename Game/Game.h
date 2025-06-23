#pragma once
#include <memory>
//最初から用意されているシーン
#include "Managers/Scene/DemoScene.h"
#include "Managers/Scene/SceneManager.h"

//ゲームで使うシーン
#include "Scenes/GameScene/GameScene.h"
#include "Scenes/TitleScene/TitleScene.h"


//ゲームシーン

class Game
{
public:
	Game();
	~Game();

	void Initialize();
	void Update();
	void Draw();
	void Finalize();

private:
	void InitializeScenes();

	// シーンマネージャー
	SceneManager* sceneManager_;  // 追加（シングルトンなのでポインタ）
	// リソース管理
	ModelManager* modelManager_;
	TextureManager* textureManager_;

};