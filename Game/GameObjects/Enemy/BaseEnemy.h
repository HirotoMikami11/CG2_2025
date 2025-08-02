#pragma once
#include <memory>
#include <list>

#include "Objects/GameObject/GameObject.h"
#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "Objects/Light/Light.h"
#include "Engine.h"

#include "State/BaseEnemyState.h"
#include "GameObjects/EnemyBullet/EnemyBullet.h"
#include "MyMath/TimedCall.h"
#include "GameObjects/Collider.h"
#include "CollisionManager/CollisionConfig.h"

// プレイヤークラスの前方宣言
class Player;
// ゲームシーンの前方宣言
class GameScene;

/// <summary>
/// 敵の種類
/// </summary>
enum class EnemyType {
	Normal = 0,        // 通常の敵（現在の敵）
	RushingFish = 1,   // 突進してくる魚
	ShootingFish = 2    // 射撃する魚
};

/// <summary>
/// 敵の行動パターン
/// </summary>
enum class EnemyPattern {
	Straight = 0,   // まっすぐ進み続ける
	LeaveLeft = 1,  // 離脱フェーズで左上に移動
	LeaveRight = 2, // 離脱フェーズで右上に移動
	Homing = 3,     // ホーミング（突進魚用）
	Shooting = 4     // 射撃パターン（射撃魚用）
};

/// <summary>
/// 敵の基底クラス
/// </summary>
class BaseEnemy : public Collider {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	BaseEnemy(EnemyType type);

	/// <summary>
	/// 仮想デストラクタ
	/// </summary>
	virtual ~BaseEnemy();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="position">初期位置</param>
	/// <param name="pattern">敵のパターン</param>
	virtual void Initialize(DirectXCommon* dxCommon, const Vector3& position, EnemyPattern pattern = EnemyPattern::Straight) = 0;

	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	virtual void Update(const Matrix4x4& viewProjectionMatrix) = 0;

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="directionalLight">平行光源</param>
	virtual void Draw(const Light& directionalLight) = 0;

	/// <summary>
	/// ImGui
	/// </summary>
	virtual void ImGui() = 0;

	/// <summary>
	/// 状態変更
	/// </summary>
	/// <param name="state">ステート</param>
	void ChangeState(std::unique_ptr<BaseEnemyState> state);

	/// <summary>
	/// ダメージを受ける
	/// </summary>
	/// <param name="damage">ダメージ量</param>
	virtual void TakeDamage(float damage);

	/// <summary>
	/// 時限発動の更新、終了した者の削除
	/// </summary>
	void UpdateTimedCalls();

	/// <summary>
	/// 時限発動をクリアする
	/// </summary>
	void ClearTimeCalls();

	// Getter
	Vector3 GetPosition() const { return gameObject_->GetPosition(); }
	Vector3 GetRotation() const { return gameObject_->GetRotation(); }
	Vector3 GetVelocity() const { return velocity_; }
	Vector3 GetWorldPosition() override;
	EnemyType GetEnemyType() const { return enemyType_; }
	EnemyPattern GetPattern() const { return pattern_; }
	bool IsDead() const { return isDead_; }
	float GetCurrentHP() const { return currentHP_; }
	float GetMaxHP() const { return maxHP_; }
	Player* GetPlayer() const { return player_; }

	/// <summary>
	/// 衝突時に呼ばれる関数（基本実装、サブクラスでオーバーライド可能）
	/// </summary>
	virtual void OnCollision() override;

	// Setter
	void SetPosition(const Vector3& position) { gameObject_->SetPosition(position); }
	void SetRotation(const Vector3& rotation) { gameObject_->SetRotation(rotation); }
	void SetVelocity(const Vector3& velocity) { velocity_ = velocity; }
	void SetPlayer(Player* player) { player_ = player; }
	void SetGameScene(GameScene* gameScene) { gameScene_ = gameScene; }

protected:
	// ゲームオブジェクト
	std::unique_ptr<GameObject> gameObject_;

	// 敵の種類
	EnemyType enemyType_;

	// 速度
	Vector3 velocity_ = { 0.0f, 0.0f, 0.0f };

	// 状態
	std::unique_ptr<BaseEnemyState> state_;

	// 敵の行動パターン
	EnemyPattern pattern_ = EnemyPattern::Straight;

	// HP関連
	float maxHP_ = 0.0f;        // 最大HP
	float currentHP_ = 0.0f;    // 現在のHP

	// 死亡フラグ
	bool isDead_ = false;

	// 遅延クリア用フラグ
	bool shouldClearTimedCalls_ = false;

	// 時限発動クラス
	std::list<std::unique_ptr<TimedCall>> timedCalls_;

	// プレイヤーの情報
	Player* player_ = nullptr;

	// ゲームシーンの情報
	GameScene* gameScene_ = nullptr;

	// システム参照
	DirectXCommon* directXCommon_ = nullptr;

	/// <summary>
	/// 速度の方向を向く
	/// </summary>
	void SetToVelocityDirection();

	/// <summary>
	/// 設定されたパターンに応じた初期Stateを設定（サブクラスで実装）
	/// </summary>
	virtual void SetInitializeState() = 0;
};