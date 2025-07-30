#include "EnemyStateApproach.h"
#include "GameObjects/Enemy/Enemy.h"


EnemyStateApproach::EnemyStateApproach(Enemy* enemy) : BaseEnemyState("State Approach", enemy) {
	// 自動発射を開始する
	enemy->StartAutoFire();
	// 敵の速度を設定
	enemy_->SetVelocity(velocity_);
}

EnemyStateApproach::~EnemyStateApproach() {
	// 時限発動のリストを他のフェーズに移行する場合、1フレーム遅れさせて時限発動リストをクリアする。
	enemy_->ClearTimeCalls();

}

void EnemyStateApproach::Update() {

	//									移動処理									//

	// 座標を取得
	Vector3 enemyPos = enemy_->GetPosition();
	// 接近フェーズの更新処理
	enemy_->SetPosition((enemyPos + velocity_));


	//									状態遷移									//
	// 状態遷移の条件
	if (enemyPos.z <= ChangeLeavePosZ_) {
		// パターンに応じて離脱フェーズに移行
		EnemyPattern pattern = enemy_->GetPattern();
		switch (pattern) {

			//まっすぐ進んでくる
		case EnemyPattern::Straight:
			enemy_->ChangeState(std::make_unique<EnemyStateStraight>(enemy_));
			break;
		case EnemyPattern::LeaveLeft:
		case EnemyPattern::LeaveRight:
			// 離脱フェーズに移行（パターンは離脱状態内で判定）
			enemy_->ChangeState(std::make_unique<EnemyStateLeave>(enemy_));
			break;
		default:
			// その他のパターンには現状遷移しないので設定しない
			break;
		}
	}
}