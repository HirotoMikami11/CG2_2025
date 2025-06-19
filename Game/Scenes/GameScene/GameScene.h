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

class GameScene {
public:
	GameScene();
	~GameScene();

	void Initialize();
	void Update();
	void Draw();
	void Finalize();

private:
	void InitializeGameObjects();
	void UpdateGameObjects();
	void DrawGameObjects();

	// ゲームオブジェクト
	static const int kMaxTriangleIndex = 2;
	std::array<std::unique_ptr<Triangle>, kMaxTriangleIndex> triangles_;
	std::unique_ptr<Sphere> sphere_;
	std::unique_ptr<Model3D> model_;

	// UI
	std::unique_ptr<Sprite> sprite_;

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