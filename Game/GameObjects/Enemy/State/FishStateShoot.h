#pragma once
#include "BaseEnemyState.h"
#include "MyMath/MyFunction.h"

// 前方宣言
class BaseEnemy;

/// <summary>
/// 射撃魚の射撃状態
/// 停止してプレイヤーの方向を向いて弾を発射
/// </summary>
class FishStateShoot : public BaseEnemyState {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="enemy">敵のポインタ</param>
	FishStateShoot(BaseEnemy* enemy);

	/// <summary>
	/// デストラクタ
	/// </summary>
	~FishStateShoot();

	/// <summary>
	/// 更新
	/// </summary>
	void Update() override;

private:
	// 射撃フェーズ
	enum class ShootPhase {
		Aiming,   // 照準中（回転）
		Shooting, // 射撃
		Cooldown  // クールダウン
	};

	// 現在のフェーズ
	ShootPhase currentPhase_ = ShootPhase::Aiming;

	// 照準時間（フレーム数）
	static constexpr int kAimingTime = 30;

	// クールダウン時間（フレーム数）
	static constexpr int kCooldownTime = 30;

	// フェーズタイマー
	int phaseTimer_ = 0;

	// 回転イージングの強度
	static constexpr float kRotationEasing = 0.1f;

	// 弾の速度
	static constexpr float kBulletSpeed = 3.0f;

	// 最大射撃回数
	static constexpr int kMaxShotCount = 4;

	/// <summary>
	/// プレイヤーの方向を向く（イージング付き）
	/// </summary>
	void AimAtPlayer();

	/// <summary>
	/// 弾を発射する
	/// </summary>
	void Shoot();
};