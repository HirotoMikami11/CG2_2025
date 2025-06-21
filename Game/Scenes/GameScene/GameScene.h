#pragma once
#include <memory>
#include <array>
#include <list>

#include "Engine.h"
#include "Managers/Scene/BaseScene.h"		//シーン基底クラス

#include"Objects/Player.h"			//プレイヤー
#include "Objects/Enemy.h"			//敵
#include"Objects/Skydome.h"			//天球
#include"Objects/MapChipField.h"	//ブロック
#include"Objects/GameCamera.h"		//カメラをプレイヤー追従させる

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
	void DrawGameObjects();

	/// <summary>
	/// 敵の初期化
	/// </summary>
	void InitializeEnemies();
	/// <summary>
	/// マップチップデータに合わせたブロックの生成
	/// </summary>
	void GenerateBlocks();
	/// <summary>
	/// 全ての当たり判定を行う
	/// </summary>
	void CheckAllCollision();

	// ゲームオブジェクト
	//プレイヤー
	std::unique_ptr<Player> player_ = nullptr;
	//天球
	std::unique_ptr<Skydome> skydome_ = nullptr;

	// 敵
	std::list<std::unique_ptr<Enemy>> enemies_;

	// 敵の数
	static inline const int kEnemyCount = 3;

	// マップチップ
	MapChipField* mapChipField_;

	// ブロック（2次元配列でブロックを管理）
	std::vector<std::vector<std::unique_ptr<Model3D>>> blocks_;


	// ライティング
	Light directionalLight_;

	// カメラ
	CameraController* cameraController_;
	std::unique_ptr<GameCamera> gameCamera_ = nullptr;
	Matrix4x4 viewProjectionMatrix;
	Matrix4x4 viewProjectionMatrixSprite;

	// システム参照
	DirectXCommon* directXCommon_;
	OffscreenRenderer* offscreenRenderer_;

};