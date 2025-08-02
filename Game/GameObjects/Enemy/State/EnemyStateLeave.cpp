#include "EnemyStateLeave.h"
#include "GameObjects/Enemy/BaseEnemy.h"

EnemyStateLeave::EnemyStateLeave(BaseEnemy* enemy) : BaseEnemyState("State Leave", enemy) {
	// パターンを保持
	EnemyPattern pattern = enemy_->GetPattern();

	// パターンによって移動する方向を変更
	switch (pattern) {
	case EnemyPattern::LeaveLeft:
		// 左上に移動
		velocity_ = { -0.35f, 0.35f, -0.35f };
		break;
	case EnemyPattern::LeaveRight:
		// 右上に移動
		velocity_ = { 0.35f, 0.35f, -0.35f };
		break;
	default:
		// どれにも該当しないのはおかしいので停止させておく
		velocity_ = { 0, 0, 0 };
		break;
	}

	// 敵の速度を設定
	enemy_->SetVelocity(velocity_);
}

void EnemyStateLeave::Update() {
	// 離脱フェーズの更新処理
	enemy_->SetPosition((enemy_->GetPosition() + velocity_));
}