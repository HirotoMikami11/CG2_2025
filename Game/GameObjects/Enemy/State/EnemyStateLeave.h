#pragma once
#include "BaseEnemyState.h"

class EnemyStateLeave : public BaseEnemyState {

public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="enemy"></param>
	EnemyStateLeave(Enemy* enemy);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

private:
	// 接近フェーズの速度
	Vector3 velocity_ = {-0.35f, 0.35f, -0.35f}; // 離脱フェーズの速度

};
