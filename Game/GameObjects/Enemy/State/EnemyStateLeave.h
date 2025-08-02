#pragma once
#include "BaseEnemyState.h"

class EnemyStateLeave : public BaseEnemyState {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="enemy">敵のポインタ</param>
	EnemyStateLeave(BaseEnemy* enemy);

	/// <summary>
	/// 更新
	/// </summary>
	void Update() override;

private:
	// 離脱フェーズの速度
	Vector3 velocity_;
};