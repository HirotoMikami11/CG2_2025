#include "FishStateHover.h"
#include "FishStateShoot.h"
#include "GameObjects/Enemy/BaseEnemy.h"
#include "GameObjects/Player/Player.h"
#include "MyMath/Random/Random.h"

FishStateHover::FishStateHover(BaseEnemy* enemy) : BaseEnemyState("Shoting Fish Hover", enemy) {
	// 初期ホバー目標位置を生成
	GenerateNewHoverTarget();
	shootTimer_ = kShootInterval; // 最初の射撃までの時間
}

FishStateHover::~FishStateHover() {
}

void FishStateHover::Update() {
	// プレイヤーがいない場合は何もしない
	if (!GetPlayer()) {
		return;
	}

	// 射撃タイマーの更新
	shootTimer_--;
	if (shootTimer_ <= 0) {
		// 射撃状態に遷移
		enemy_->ChangeState(std::make_unique<FishStateShoot>(enemy_));
		return;
	}

	// 目標位置変更タイマーの更新
	targetChangeTimer_--;
	if (targetChangeTimer_ <= 0) {
		GenerateNewHoverTarget();
		targetChangeTimer_ = kTargetChangeInterval;
	}

	// ホバー目標位置に向かって移動
	Vector3 enemyPos = enemy_->GetPosition();
	Vector3 toTarget = hoverTarget_ - enemyPos;
	float distanceToTarget = Length(toTarget);

	// 目標位置に近づいた場合は新しい目標を生成
	if (distanceToTarget < 3.0f) {
		GenerateNewHoverTarget();
	}

	// ホバー移動
	Vector3 direction = Normalize(toTarget);
	Vector3 velocity = direction * kHoverSpeed;

	enemy_->SetVelocity(velocity);

	// 移動処理
	Vector3 newPosition = enemyPos + velocity;
	enemy_->SetPosition(newPosition);
}

void FishStateHover::GenerateNewHoverTarget() {
	if (!GetPlayer()) {
		return;
	}

	// プレイヤーの位置と方向を取得
	Vector3 playerPos = GetPlayer()->GetWorldPosition();
	Vector3 playerForward = GetPlayer()->GetForward(); // プレイヤーの前方向

	// プレイヤーの前方基準距離にランダムオフセットを追加
	float randomOffsetX = Random::GetInstance().GenerateFloat(-kOffsetRange, kOffsetRange);
	float randomOffsetY = Random::GetInstance().GenerateFloat(-kOffsetRange, kOffsetRange);
	float randomOffsetZ = Random::GetInstance().GenerateFloat(-kOffsetRange, kOffsetRange);

	Vector3 basePosition = playerPos + playerForward * kBaseDistance;
	hoverTarget_ = basePosition + Vector3{ randomOffsetX, randomOffsetY, randomOffsetZ };
}