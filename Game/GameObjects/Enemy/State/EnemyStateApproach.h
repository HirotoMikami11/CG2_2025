#pragma once
#include "BaseEnemyState.h"

class EnemyStateApproach : public BaseEnemyState {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    /// <param name="enemy">敵のポインタ</param>
    EnemyStateApproach(BaseEnemy* enemy);

    /// <summary>
    /// デストラクタ　
    /// </summary>
    ~EnemyStateApproach();

    /// <summary>
    /// 更新
    /// </summary>
    void Update() override;

private:
    // 接近フェーズの速度
    Vector3 velocity_ = { 0.0f, 0.0f, -0.35f }; // 接近フェーズの速度

    // 状態遷移の条件
    // 　この座標に到達したら離脱フェーズに移行する
    float changeLeavePosZ_ = 50.0f; // 離脱フェーズに移行する座標Z値
};