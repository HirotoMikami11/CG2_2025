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
#include "Objects/DeathParticles.h"	//死亡演出のパーティクル

class GameScene : public BaseScene {
public:
	GameScene();
	~GameScene() override;

	/// <summary>
	/// リソース読み込み（1回のみ実行）
	/// </summary>
	void LoadResources() override;

	/// <summary>
	/// オブジェクト初期化（シーン切り替え毎に実行）
	/// </summary>
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

	/// <summary>
	/// ゲームのフェーズ型
	/// </summary>
	enum class Phase {
		kPlay,  // ゲームプレイ
		kDeath, // デス演出
	};


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

	/// <summary>
	/// フェーズの切り替え
	/// </summary>
	void ChangePhase();

	// ゲームの現在フェーズ
	Phase phase_ = Phase::kPlay;

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

	//死亡パーティクル
	std::unique_ptr<DeathParticles> deathParticles_ = nullptr;

	// ライティング
	Light directionalLight_;

	// カメラ
	CameraController* cameraController_;
	GameCamera* gameCamera_ = nullptr;
	Matrix4x4 viewProjectionMatrix;
	Matrix4x4 viewProjectionMatrixSprite;

	// システム参照
	DirectXCommon* directXCommon_;
	OffscreenRenderer* offscreenRenderer_;
	
	// リソース管理
	ModelManager* modelManager_;
	TextureManager* textureManager_;


};