#include "FishStateShoot.h"
#include "FishStateHover.h"
#include "FishStateEscape.h"
#include "GameObjects/Enemy/BaseEnemy.h"
#include "GameObjects/Enemy/ShootingFishEnemy.h"
#include "GameObjects/Player/Player.h"
#include "GameObjects/EnemyBullet/EnemyBullet.h"
#include "Scenes/GameScene/GameScene.h"

FishStateShoot::FishStateShoot(BaseEnemy* enemy) : BaseEnemyState("Shoting Fish Shoot", enemy) {
	// 停止
	enemy_->SetVelocity({ 0.0f, 0.0f, 0.0f });

	// 照準フェーズから開始
	currentPhase_ = ShootPhase::Aiming;
	phaseTimer_ = kAimingTime;
}

FishStateShoot::~FishStateShoot() {
}

void FishStateShoot::Update() {
	// プレイヤーがいない場合は何もしない
	if (!GetPlayer()) {
		return;
	}
	if (enemy_->GetEnemyType() == EnemyType::ShootingFish) {
		ShootingFishEnemy* shotingEnemy = dynamic_cast<ShootingFishEnemy*>(enemy_);

		phaseTimer_--;

		switch (currentPhase_) {
		case ShootPhase::Aiming:
			// プレイヤーの方向を向く
			AimAtPlayer();

			if (phaseTimer_ <= 0) {
				// 射撃フェーズに移行
				currentPhase_ = ShootPhase::Shooting;
				Shoot();
				if (shotingEnemy) {
					shotingEnemy->AddShotCount();
				}
				phaseTimer_ = kCooldownTime;
				currentPhase_ = ShootPhase::Cooldown; // 射撃後すぐにクールダウン
			}

			break;

		case ShootPhase::Cooldown:
			if (phaseTimer_ <= 0) {
				// 最大射撃回数に到達したかチェック
				if (shotingEnemy->GetShotCount() >= kMaxShotCount) {
					// 逃走状態に遷移
					enemy_->ChangeState(std::make_unique<FishStateEscape>(enemy_));
					return;
				} else {
					// ホバー状態に戻る
					enemy_->ChangeState(std::make_unique<FishStateHover>(enemy_));
					return;
				}
			}
			break;
		}
	}
}

void FishStateShoot::AimAtPlayer() {
	if (!GetPlayer()) {
		return;
	}

	// プレイヤーの位置を取得
	Vector3 playerPos = GetPlayer()->GetWorldPosition();
	Vector3 enemyPos = enemy_->GetPosition();

	// プレイヤーへの方向ベクトルを計算
	Vector3 toPlayer = Normalize(playerPos - enemyPos);

	// 現在の向きを取得（GameObjectから）
	Vector3 currentRotation = enemy_->GetRotation(); // BaseEnemyにGetRotation()が必要

	// 目標の回転角度を計算
	Vector3 targetRotation;
	targetRotation.y = std::atan2(toPlayer.x, toPlayer.z);

	float XZLength = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.z * toPlayer.z);
	targetRotation.x = std::atan2(-toPlayer.y, XZLength);
	targetRotation.z = 0.0f;

	// イージングで回転
	Vector3 newRotation = Lerp(currentRotation, targetRotation, kRotationEasing);
	enemy_->SetRotation(newRotation); // BaseEnemyにSetRotation()が必要
}

void FishStateShoot::Shoot() {

	if (enemy_->GetEnemyType() == EnemyType::ShootingFish) {
		ShootingFishEnemy* normalEnemy = dynamic_cast<ShootingFishEnemy*>(enemy_);
		if (normalEnemy) {
			normalEnemy->Fire();
		}
	}

}