#include "RushingFishStateHoming.h"
#include "RushingFishStateRush.h"
#include "GameObjects/Enemy/BaseEnemy.h"
#include "GameObjects/Player/Player.h"
#include "MyMath/Random/Random.h"

RushingFishStateHoming::RushingFishStateHoming(BaseEnemy* enemy) : BaseEnemyState("Rushing Fish Homing", enemy) {
	// ホーミング速度をランダムに設定
	homingSpeed_ = Random::GetInstance().GenerateFloat(0.3f, 0.7f);

	if (enemy_->GetPlayer()) {
		Vector3 playerPos = enemy_->GetPlayer()->GetWorldPosition();
		Vector3 enemyPos = enemy_->GetPosition();
		Vector3 direction = Normalize(playerPos - enemyPos);

		Vector3 initialVelocity = direction * homingSpeed_;
		enemy_->SetVelocity(initialVelocity);
	}
}

RushingFishStateHoming::~RushingFishStateHoming() {
}

void RushingFishStateHoming::Update() {
	// プレイヤーがいない場合は何もしない
	if (!GetPlayer()) {
		return;
	}

	// プレイヤーの位置を取得
	Vector3 playerPos = GetPlayer()->GetWorldPosition();
	Vector3 enemyPos = enemy_->GetPosition();

	// プレイヤーとの距離を計算
	float distanceToPlayer = Distance(playerPos, enemyPos);

	// 距離が閾値以下になったら直進状態に遷移
	if (distanceToPlayer <= kRushTriggerDistance) {
		// 現在の速度で直進状態に遷移
		Vector3 currentVelocity = enemy_->GetVelocity();
		enemy_->ChangeState(std::make_unique<RushingFishStateRush>(enemy_, currentVelocity));
		return;
	}

	// プレイヤーへの方向ベクトルを計算
	Vector3 toPlayer = playerPos - enemyPos;
	toPlayer = Normalize(toPlayer);

	// 現在の速度を取得
	Vector3 currentVelocity = enemy_->GetVelocity();

	// ホーミング処理：現在の速度とプレイヤーへの方向を補間
	Vector3 newVelocity = Lerp(currentVelocity, toPlayer * homingSpeed_, kHomingStrength);

	// 速度の大きさを一定に保つ
	newVelocity = Normalize(newVelocity) * homingSpeed_;

	// 速度を設定
	enemy_->SetVelocity(newVelocity);

	// 移動処理
	Vector3 newPosition = enemyPos + newVelocity;
	enemy_->SetPosition(newPosition);
}