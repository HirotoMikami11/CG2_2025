#pragma once
#include <memory>

#include "Managers/Scene/SceneManager.h"
#include "Managers/Transition/TransitionManager.h"
/// 				Scene							///
//最初から用意されているシーン
#include "Managers/Scene/DemoScene.h"
//必要なシーン郡
#include "Scenes/GameScene/GameScene.h"
#include "Scenes/TitleScene/TitleScene.h"
#include "Scenes/GameoverScene/GameoverScene.h"
#include "Scenes/GameclearScene/GameclearScene.h"

/// 				Transitions							///
#include "Managers/Transition/SceneTransitionHelper.h"
#include "Managers/Transition/TransitionEffect/SlideEffect.h"

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

	/// <summary>
	/// 3D描画（オフスクリーン内）
	/// </summary>
	void DrawOffscreen();

	/// <summary>
	/// UI描画（オフスクリーン外）
	/// </summary>
	void DrawBackBuffer();

	void Finalize();

private:
	/// <summary>
	/// シーンの初期化
	/// </summary>
	void InitializeScenes();

	/// <summary>
	/// トランジションエフェクトの登録
	/// </summary>
	void RegisterTransitionEffects();

	// シーンマネージャー
	SceneManager* sceneManager_;

	// トランジションマネージャー
	TransitionManager* transitionManager_;

	// リソース管理
	ModelManager* modelManager_;
	TextureManager* textureManager_;
};