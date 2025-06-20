#pragma once
#include <memory>
#include "Scenes/GameScene/GameScene.h"
#include "Scenes/TitleScene/TitleScene.h"
#include "../DemoScene.h"
#include "../SceneManager.h"

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
	std::unique_ptr<SceneManager> sceneManager_;
};