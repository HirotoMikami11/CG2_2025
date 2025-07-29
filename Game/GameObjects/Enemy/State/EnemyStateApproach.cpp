#include "EnemyStateApproach.h"
#include "GameObjects/Enemy/Enemy.h"



EnemyStateApproach::EnemyStateApproach(Enemy* enemy) : BaseEnemyState("State Approach", enemy) {
	// 自動発射を開始する
	enemy->StartAutoFire();
}

EnemyStateApproach::~EnemyStateApproach() {
	// 時限発動のリストを他のフェーズに移行する場合、1フレーム遅れさせて時限発動リストをクリアする。
	enemy_->ClearTimeCalls();

}

void EnemyStateApproach::Update() {
	// デバッグログ出力
	DebugLog();

	//									移動処理									//

	// 座標を取得
	Vector3 enemyPos = enemy_->GetPosition();
	// 接近フェーズの更新処理
	enemy_->SetPosition((enemyPos + velocity_));


	//									状態遷移									//
	// 状態遷移の条件
	if (enemyPos.z <= ChangeLeavePosZ_) {
		// 離脱フェーズに移行
		enemy_->ChangeState(std::make_unique<EnemyStateLeave>(enemy_));
	}
}