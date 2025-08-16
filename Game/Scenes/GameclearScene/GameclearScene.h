#pragma once
#include <memory>
#include <array>

#include "Engine.h"
#include "Managers/Scene/BaseScene.h"		//シーン基底クラス

#include "GameObjects/Ground/Ground.h"			//地面
#include "GameObjects/Rock/Rock.h"


class GameclearScene : public BaseScene
{
public:
	GameclearScene();
	~GameclearScene() override;

	/// <summary>
	/// リソース読み込み（1回のみ実行）
	/// </summary>
	void LoadResources() override;

	/// <summary>
	/// シーンに入った時のオフスクリーン設定
	/// </summary>
	void ConfigureOffscreenEffects() override;

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

	// ImGui描画
	void ImGui() override;

private:
	void InitializeGameObjects();
	void UpdateGameObjects();

	// ゲームオブジェクト

	//タイトルフォント
	std::unique_ptr<Model3D> gameclearFont_ = nullptr;

	//タイトル用のプレイヤー
	std::unique_ptr<Model3D> gameclearPlayer_ = nullptr;

	// 背景オブジェクト
	std::unique_ptr<Ground> ground_;

	//岩
	std::unique_ptr<Rock> rock_[3];


	// イージングパラメータ
	float t = 0.0f;
	const float tSpeed = 1.0f / 400.0f; // イージングの進行速度


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