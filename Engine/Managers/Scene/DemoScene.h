#pragma once
#include <memory>
#include <array>

#include "Objects/Sprite/Sprite.h"
#include "Objects/Light/Light.h"
#include "Objects/GameObject/GameObject.h"
#include "CameraController/CameraController.h"
#include "Objects/Line/GridLine.h"

#include "Engine.h"
#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "Managers/Scene/BaseScene.h"


/// <summary>
/// 絶対動くシーン(作り変えたりしない)
/// デバッグ用に使用するシーンは別途用意しておく(DebugSceneかSampleScene)
/// </summary>
class DemoScene : public BaseScene {
public:
	DemoScene();
	~DemoScene() override;

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
	static const int kMaxTriangleIndex = 2;
	std::array<std::unique_ptr<Triangle>, kMaxTriangleIndex> triangles_;
	std::unique_ptr<Sphere> sphere_;
	std::unique_ptr<Model3D> model_;

	std::unique_ptr<Sprite> sprite_;
	std::unique_ptr<GridLine> gridLine_;

	// ライティング
	Light directionalLight_;

	// カメラ
	CameraController* cameraController_;
	Matrix4x4 viewProjectionMatrix;
	Matrix4x4 viewProjectionMatrixSprite;

	// システム参照
	DirectXCommon* directXCommon_;
	OffscreenRenderer* offscreenRenderer_;

	// リソース管理
	ModelManager* modelManager_;
	TextureManager* textureManager_;

};