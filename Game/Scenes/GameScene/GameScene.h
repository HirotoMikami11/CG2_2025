#pragma once
#include <memory>
#include <array>

#include "Engine.h"
#include "../../../BaseScene.h"		//シーン基底クラス

#include"Objects/Player/Player.h"	//プレイヤー

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
	//プレイヤー
	std::unique_ptr<Player> player_ = nullptr;


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