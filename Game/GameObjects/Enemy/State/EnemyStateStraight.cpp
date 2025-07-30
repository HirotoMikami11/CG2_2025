#include "EnemyStateStraight.h"
#include "GameObjects/Enemy/Enemy.h"

EnemyStateStraight::EnemyStateStraight(Enemy* enemy) : BaseEnemyState("State Approach", enemy) {

	// 敵の速度を設定
	enemy_->SetVelocity(velocity_);
}

EnemyStateStraight::~EnemyStateStraight() {
}

void EnemyStateStraight::Update() {

	//									移動処理									//

	// 座標を取得
	Vector3 enemyPos = enemy_->GetPosition();
	// 接近フェーズの更新処理
	enemy_->SetPosition((enemyPos + velocity_));


}