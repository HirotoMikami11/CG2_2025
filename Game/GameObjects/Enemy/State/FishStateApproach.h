#pragma once
#include "BaseEnemyState.h"

// 前方宣言
class BaseEnemy;

/// <summary>
/// 射撃魚の接近状態
/// プレイヤーの前方50付近まで移動する
/// </summary>
class FishStateApproach : public BaseEnemyState {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="enemy">敵のポインタ</param>
	FishStateApproach(BaseEnemy* enemy);

	/// <summary>
	/// デストラクタ
	/// </summary>
	~FishStateApproach();

	/// <summary>
	/// 更新
	/// </summary>
	void Update() override;

private:
	// 移動速度
	static constexpr float kMoveSpeed = 1.2f;

	// プレイヤーの前方に停止する距離
	static constexpr float kTargetDistance = 50.0f;

	// 距離の許容誤差
	static constexpr float kDistanceTolerance = 5.0f;
};