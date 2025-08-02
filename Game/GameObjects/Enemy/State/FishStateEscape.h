#pragma once
#include "BaseEnemyState.h"
#include "MyMath/MyFunction.h"

// 前方宣言
class BaseEnemy;

/// <summary>
/// 射撃魚の逃走状態
/// プレイヤーの向いている方向から垂直なランダムな方向に逃げる
/// </summary>
class FishStateEscape : public BaseEnemyState {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="enemy">敵のポインタ</param>
	FishStateEscape(BaseEnemy* enemy);

	/// <summary>
	/// デストラクタ
	/// </summary>
	~FishStateEscape();

	/// <summary>
	/// 更新
	/// </summary>
	void Update() override;

private:
	// 逃走速度
	static constexpr float kEscapeSpeed = 2.0f;

	// 逃走方向
	Vector3 escapeDirection_;

	/// <summary>
	/// 逃走方向を計算する
	/// </summary>
	void CalculateEscapeDirection();
};