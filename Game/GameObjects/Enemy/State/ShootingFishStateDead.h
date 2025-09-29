#pragma once
#include "BaseEnemyState.h"

// 前方宣言
class BaseEnemy;

/// <summary>
/// 射撃魚の死亡状態
/// 上方向にゆっくり移動しながら下を向き、透明度を下げて消滅する
/// </summary>
class ShootingFishStateDead : public BaseEnemyState {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="enemy">敵のポインタ</param>
	ShootingFishStateDead(BaseEnemy* enemy);

	/// <summary>
	/// デストラクタ
	/// </summary>
	~ShootingFishStateDead();

	/// <summary>
	/// 更新
	/// </summary>
	void Update() override;

private:
	// 移動関連
	static constexpr float kUpwardSpeed = 2.0f;  // 上向きの移動速度

	// 回転関連
	Vector3 initialRotation_;            // 初期回転値
	Vector3 targetRotation_;             // 目標回転値（下向き）
	float deathTimer_ = 0.0f;            // 死亡アニメーションのタイマー
	static constexpr float kDeathDuration = 1.0f;  // 死亡アニメーションにかける時間

	// フェード関連
	float initialAlpha_ = 1.0f;          // 初期透明度
};