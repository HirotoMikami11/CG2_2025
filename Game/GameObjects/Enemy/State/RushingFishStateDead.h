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
	// 死亡アニメーションの段階
	enum class DeathPhase {
		Moving,     // 上向きに移動しながら下向きになる段階（透明度変更開始）
		Fading      // 透明度を急激に下げる段階
	};

	DeathPhase currentPhase_ = DeathPhase::Moving;

	
	// 回転関連
	Vector3 initialRotation_;        // 初期回転値
	Vector3 targetRotation_;         // 目標回転値（下向き）
	float rotationTimer_ = 0.0f;     // 回転のタイマー
	static constexpr float kRotationDuration = 0.5f;  // 回転にかける時間

	// フェード関連
	float initialAlpha_ = 1.0f;      // 初期透明度
	float fadeTimer_ = 0.0f;         // フェードのタイマー
	static constexpr float kFadeDuration = 0.5f;   // フェードにかける時間（回転と同時進行）

	/// <summary>
	/// 移動段階の更新
	/// </summary>
	void UpdateMovingPhase();

	/// <summary>
	/// フェード段階の更新
	/// </summary>
	void UpdateFadingPhase();
};