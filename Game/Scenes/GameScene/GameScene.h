#pragma once
#include <memory>
#include <array>
#include <list>
#include <fstream>
#include <sstream>
#include <string>

#include "Engine.h"
#include "Managers/Scene/BaseScene.h"

#include "Objects/Line/GridLine.h"
#include "GameObjects/Player/Player.h"
#include "GameObjects/Enemy/BaseEnemy.h"

#include "GameObjects/EnemyBullet/EnemyBullet.h"
#include "CollisionManager/CollisionManager.h"	//衝突判定マネージャー
#include "GameObjects/Ground/Ground.h"			//地面
#include "GameObjects/Skydome/Skydome.h"		//天球
#include "GameObjects/LockOn/LockOn.h"

#include "EnemyPopCommand/EnemyPopCommand.h"
#include "EnemyPlacementEditor/EnemyPlacementEditor.h" // 敵配置エディタ

#include "FieldEditor/FieldEditor.h"		  // フィールドエディタ
#include "FieldEditor/FieldLoader.h"		 // フィールドローダー

#include "CameraController/CameraController.h"	//カメラコントローラー
#include "Camera/RailCameraEditor.h" 

// 前方宣言
class RailCamera;

/// <summary>
/// ゲームシーン
/// </summary>
class GameScene : public BaseScene {
public:
	GameScene();
	~GameScene() override;

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

	/// <summary>
	/// 敵弾を追加する(敵で使用する)
	/// </summary>
	/// <param name="enemyBullet">敵弾</param>
	void AddEnemyBullet(std::unique_ptr<EnemyBullet> enemyBullet);

private:
	void InitializeGameObjects();
	void UpdateGameObjects();

	// ゲームオブジェクト
	std::unique_ptr<Player> player_;
	// ロックオンシステム
	std::unique_ptr<LockOn> lockOn_;

	std::list<std::unique_ptr<BaseEnemy>> enemies_;

	// 敵の弾丸
	std::list<std::unique_ptr<EnemyBullet>> enemyBullets_;

	// 背景オブジェクト
	std::unique_ptr<Ground> ground_;
	std::unique_ptr<Skydome> skydome_;

	// ライティング
	Light directionalLight_;

	// カメラ
	CameraController* cameraController_;
	RailCamera* railCamera_ = nullptr;  // レールカメラへの参照
	Matrix4x4 viewProjectionMatrix;
	Matrix4x4 viewProjectionMatrixSprite;

	// システム参照
	DirectXCommon* directXCommon_;
	OffscreenRenderer* offscreenRenderer_;

	// フィールド関連
	std::unique_ptr<FieldEditor> fieldEditor_;     // デバッグ用フィールドエディタ
	std::unique_ptr<FieldLoader> fieldLoader_;     // フィールドローダー

	// リソース管理
	ModelManager* modelManager_;
	TextureManager* textureManager_;
	// 音声管理
	AudioManager* audioManager_;
	//					衝突マネージャー					//
	std::unique_ptr<CollisionManager> collisionManager_;

	//					レールカメラエディタ						//
	std::unique_ptr<RailCameraEditor> railCameraEditor_;

	//					敵配置エディタ					//
	std::unique_ptr<EnemyPlacementEditor> enemyPlacementEditor_;

	//					敵発生関連					//
	// 敵発生コマンド
	std::unique_ptr<EnemyPopCommand> enemyPopCommand_;
	// 待機中フラグ
	bool isWait_ = false;
	// 待機タイマー
	int waitTimer_ = 0;

	/// <summary>
	/// 敵発生データの読み込み
	/// </summary>
	void LoadEnemyPopData();

	/// <summary>
	/// 敵発生コマンドの更新
	/// </summary>
	void UpdateEnemyPopCommands();

	/// <summary>
	/// 敵を生成する
	/// </summary>
	/// <param name="position">生成位置</param>
	/// <param name="enemyType">敵の種類</param>
	/// <param name="pattern">敵のパターン</param>
	void CreateEnemy(const Vector3& position, EnemyType enemyType, EnemyPattern pattern);

	/// <summary>
	/// 死んでいる敵を削除する
	/// </summary>
	void DeleteDeadEnemies();

	/// <summary>
	/// 死んでいる敵弾を削除する
	/// </summary>
	void DeleteDeadEnemyBullets();

	/// <summary>
	/// 全ての敵弾を削除する
	/// </summary>
	void ClearAllEnemyBullets();

	/// <summary>
	/// 全ての敵を削除する
	/// </summary>
	void ClearAllEnemies();

	/// <summary>
	/// デバッグでゲーム開始処理(imguiでリスタートするときにしよう)
	/// </summary>
	void DebugStartGame();

};