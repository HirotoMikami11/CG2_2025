#include "EnemyStateLeave.h"
#include "GameObjects/Enemy/Enemy.h"


EnemyStateLeave::EnemyStateLeave(Enemy* enemy) : BaseEnemyState("State Leave", enemy) {}

void EnemyStateLeave::Update() {
	// デバッグログ出力
	DebugLog();

	// 離脱フェーズの更新処理
	enemy_->SetPosition((enemy_->GetPosition() + velocity_));


}
