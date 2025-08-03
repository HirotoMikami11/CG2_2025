#pragma once
#include "BaseEnemyState.h"

// 前方宣言
class BaseEnemy;

/// <summary>
/// 突進魚のホーミング状態
/// </summary>
class RushingFishStateHoming : public BaseEnemyState {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="enemy">敵のポインタ</param>
	RushingFishStateHoming(BaseEnemy* enemy);

	/// <summary>
	/// デストラクタ
	/// </summary>
	~RushingFishStateHoming();

	/// <summary>
	/// 更新
	/// </summary>
	void Update() override;

private:
	// ホーミング速度
	float homingSpeed_ = 1.5f; // デフォルト値

	// ホーミングの強度（0.0f〜1.0f）
	static constexpr float kHomingStrength = 0.07f;

	// 直進状態に遷移する距離の閾値
	static constexpr float kRushTriggerDistance = 30.0f;
};