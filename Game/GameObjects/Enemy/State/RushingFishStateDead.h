#pragma once
#include "BaseEnemyState.h"

// 前方宣言
class BaseEnemy;

/// <summary>
/// 突進魚の死亡状態
/// 最後に向いていた方向から上向きに移動しながら下を向き、透明度を下げて消滅する
/// </summary>
class RushingFishStateDead : public BaseEnemyState {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="enemy">敵のポインタ</param>
	RushingFishStateDead(BaseEnemy* enemy);

	/// <summary>
	/// デストラクタ
	/// </summary>
	~RushingFishStateDead();

	/// <summary>
	/// 更新
	/// </summary>
	void Update() override;

private:
	// 回転関連
	Vector3 initialRotation_;        // 初期回転値
	Vector3 targetRotation_;         // 目標回転値（下向き）
	float deathTimer_ = 0.0f;        // 死亡アニメーションのタイマー
	static constexpr float kDeathDuration = 0.8f;  // 死亡アニメーションにかける時間

	// フェード関連
	float initialAlpha_ = 1.0f;      // 初期透明度
};