#pragma once
#include <memory>
//最初から用意されているシーン
#include "Managers/Scene/DemoScene.h"
#include "Managers/Scene/SceneManager.h"



/// <summary>
/// ゲーム全体を管理するクラス
/// </summary>
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
	/// <summary>
	/// シーンの初期化
	/// </summary>
	void InitializeScenes();

	// シーンマネージャー
	SceneManager* sceneManager_;

	// リソース管理
	ModelManager* modelManager_;
	TextureManager* textureManager_;

};