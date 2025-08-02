#include "RushingFishStateHoming.h"
#include "GameObjects/Enemy/BaseEnemy.h"
#include "GameObjects/Player/Player.h"

RushingFishStateHoming::RushingFishStateHoming(BaseEnemy* enemy) : BaseEnemyState("Rushing Fish Homing", enemy) {
	// プレイヤーの方向に初期速度を設定
	if (enemy_->GetPlayer()) {
		Vector3 playerPos = enemy_->GetPlayer()->GetWorldPosition();
		Vector3 enemyPos = enemy_->GetPosition();
		Vector3 direction = playerPos - enemyPos;
		direction = Normalize(direction);

		// 初期速度を設定
		Vector3 initialVelocity = direction * kHomingSpeed;
		enemy_->SetVelocity(initialVelocity);
	}
}

RushingFishStateHoming::~RushingFishStateHoming() {
}

void RushingFishStateHoming::Update() {
	// プレイヤーがいない場合は何もしない
	if (!enemy_->GetPlayer()) {
		return;
	}

	// プレイヤーの位置を取得
	Vector3 playerPos = enemy_->GetPlayer()->GetWorldPosition();
	Vector3 enemyPos = enemy_->GetPosition();

	// プレイヤーへの方向ベクトルを計算
	Vector3 toPlayer = playerPos - enemyPos;
	toPlayer = Normalize(toPlayer);

	// 現在の速度を取得
	Vector3 currentVelocity = enemy_->GetVelocity();

	// ホーミング処理：現在の速度とプレイヤーへの方向を補間
	Vector3 newVelocity = Lerp(currentVelocity, toPlayer * kHomingSpeed, kHomingStrength);

	// 速度の大きさを一定に保つ
	newVelocity = Normalize(newVelocity) * kHomingSpeed;

	// 速度を設定
	enemy_->SetVelocity(newVelocity);

	// 移動処理
	Vector3 newPosition = enemyPos + newVelocity;
	enemy_->SetPosition(newPosition);
}