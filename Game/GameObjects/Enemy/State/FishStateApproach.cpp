#include "GameObjects/Enemy/State/FishStateApproach.h"
#include "GameObjects/Enemy/State/FishStateHover.h"
#include "GameObjects/Enemy/BaseEnemy.h"
#include "GameObjects/Player/Player.h"

FishStateApproach::FishStateApproach(BaseEnemy* enemy) : BaseEnemyState("Shoting Fish Approach", enemy) {
}

FishStateApproach::~FishStateApproach() {
}

void FishStateApproach::Update() {
	// プレイヤーがいない場合は何もしない
	if (!GetPlayer()) {
		return;
	}

	// プレイヤーの位置と方向を取得
	Vector3 playerPos = GetPlayer()->GetWorldPosition();
	Vector3 enemyPos = enemy_->GetPosition();

	// プレイヤーの前方ベクトルを取得（プレイヤーの向いている方向）
	Vector3 playerForward = GetPlayer()->GetForward(); // プレイヤークラスにGetForward()があると仮定

	// プレイヤーの前方kTargetDistance離れた位置を目標とする
	Vector3 targetPos = playerPos + playerForward * kTargetDistance;

	// 目標位置への方向ベクトルを計算
	Vector3 toTarget = targetPos - enemyPos;
	float distanceToTarget = Length(toTarget);

	// 目標距離に到達したかチェック
	if (distanceToTarget <= kDistanceTolerance) {
		// ホバー状態に遷移
		enemy_->ChangeState(std::make_unique<FishStateHover>(enemy_));
		return;
	}

	// 目標位置に向かって移動
	Vector3 direction = Normalize(toTarget);
	Vector3 velocity = direction * kMoveSpeed;

	enemy_->SetVelocity(velocity);

	// 移動処理
	Vector3 newPosition = enemyPos + velocity;
	enemy_->SetPosition(newPosition);
}