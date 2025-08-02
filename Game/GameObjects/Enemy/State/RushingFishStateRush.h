#pragma once
#include "BaseEnemyState.h"

// 前方宣言
class BaseEnemy;

/// <summary>
/// 突進魚の直進状態
/// ホーミング中に一定距離に近づいたら、その時の速度で直進する
/// </summary>
class RushingFishStateRush : public BaseEnemyState {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="enemy">敵のポインタ</param>
	/// <param name="rushVelocity">突進速度</param>
	RushingFishStateRush(BaseEnemy* enemy, const Vector3& rushVelocity);

	/// <summary>
	/// デストラクタ
	/// </summary>
	~RushingFishStateRush();

	/// <summary>
	/// 更新
	/// </summary>
	void Update() override;

private:


};