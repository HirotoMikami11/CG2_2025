#pragma once
#include "BaseEnemyState.h"
class EnemyStateApproach : public BaseEnemyState {

public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="enemy"></param>
	EnemyStateApproach(Enemy* enemy);

	/// <summary>
	/// デストラクタ　
	/// </summary>
	~EnemyStateApproach();
	/// <summary>
	/// 更新
	/// </summary>
	void Update();

private:
	// 接近フェーズの速度
	Vector3 velocity_ = {0.0f, 0.0f, -0.35f}; // 接近フェーズの速度

	// 状態遷移の条件
	// 　この座標に到達したら離脱フェーズに移行する
	float ChangeLeavePosZ_ = 50.0f; // 離脱フェーズに移行する座標Z値
};
