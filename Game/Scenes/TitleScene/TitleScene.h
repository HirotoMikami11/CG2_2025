#pragma once
#include <memory>
#include <array>

#include "Engine.h"
#include "Managers/Scene/BaseScene.h"		//シーン基底クラス

//タイトルプレイヤー
#include "Objects/TitlePlayer.h"

class TitleScene : public BaseScene
{
public:
	TitleScene();
	~TitleScene() override;

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

	//タイトルフォント
	std::unique_ptr<Model3D> titleFont_ = nullptr;

	//タイトル用のプレイヤー
	std::unique_ptr<TitlePlayer> TitlePlayer_ = nullptr;

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