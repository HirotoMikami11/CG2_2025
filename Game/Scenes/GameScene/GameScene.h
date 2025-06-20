#pragma once
#include <memory>
#include <array>

#include "../../../Sprite.h"
#include "../../../Light.h"
#include "../../../GameObject.h"

#include "../../../Engine.h"
#include "../../../DirectXCommon.h"
#include "../../../CameraController.h"
#include "../../../ImGuiManager.h"
#include "../../../BaseScene.h"

class GameScene : public BaseScene {
public:
	GameScene();
	~GameScene() override;

	// BaseSceneの実装
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;

	// シーンの出入り時の処理
	void OnEnter() override;
	void OnExit() override;

	// ImGui描画
	void ImGui() override;

private:
	void InitializeGameObjects();
	void UpdateGameObjects();
	void DrawGameObjects();

	// ゲームオブジェクト


	// ライティング
	Light directionalLight_;

	// カメラ
	CameraController* cameraController_;
	Matrix4x4 viewProjectionMatrix;
	Matrix4x4 viewProjectionMatrixSprite;

	// システム参照
	DirectXCommon* directXCommon_;
	OffscreenRenderer* offscreenRenderer_;
};