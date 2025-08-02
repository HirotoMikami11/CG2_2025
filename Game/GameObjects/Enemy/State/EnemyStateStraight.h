#pragma once
#include "BaseEnemyState.h"

class EnemyStateStraight : public BaseEnemyState {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="enemy">敵のポインタ</param>
	EnemyStateStraight(BaseEnemy* enemy);

	/// <summary>
	/// デストラクタ　
	/// </summary>
	~EnemyStateStraight();

	/// <summary>
	/// 更新
	/// </summary>
	void Update() override;

private:
	Vector3 velocity_ = { 0.0f, 0.0f, -0.35f };
};