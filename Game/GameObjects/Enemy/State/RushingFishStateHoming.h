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
	static constexpr float kHomingSpeed = 0.5f;

	// ホーミングの強度（0.0f〜1.0f）
	static constexpr float kHomingStrength = 0.3f;
};