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

	/// <summary>
	/// リソース読み込み（1回のみ実行）
	/// </summary>
	void LoadResources() override;

	/// <summary>
	/// オブジェクト初期化（シーン切り替え毎に実行）
	/// </summary>
	void Initialize() override;

	void Update() override;

	/// <summary>
	/// 3D描画処理（オフスクリーン内）
	/// </summary>
	void DrawOffscreen() override;

	/// <summary>
	/// UI描画処理（オフスクリーン外）
	/// </summary>
	void DrawBackBuffer() override;

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
	std::unique_ptr<Sphere> sphere_;
	std::unique_ptr<Plane> plane_;

	std::unique_ptr<Model3D> modelMultiMesh_;
	std::unique_ptr<Model3D> modelMultiMaterial_;


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