#include "FishStateHover.h"
#include "FishStateShoot.h"
#include "GameObjects/Enemy/BaseEnemy.h"
#include "GameObjects/Player/Player.h"
#include "CameraController/CameraController.h"
#include "MyMath/Random/Random.h"

FishStateHover::FishStateHover(BaseEnemy* enemy) : BaseEnemyState("Shoting Fish Hover", enemy) {
	// カメラコントローラーを取得
	CameraController* cameraController = CameraController::GetInstance();
	if (cameraController) {
		lastCameraPosition_ = cameraController->GetPosition();
	}

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

	// カメラコントローラーを取得
	CameraController* cameraController = CameraController::GetInstance();
	if (!cameraController) {
		return;
	}

	// カメラが移動した場合は目標位置を再計算
	Vector3 currentCameraPos = cameraController->GetPosition();
	if (Distance(currentCameraPos, lastCameraPosition_) > kCameraMovementThreshold) {
		// カメラの移動量を計算
		Vector3 cameraMovement = currentCameraPos - lastCameraPosition_;
		// ホバー目標位置もカメラと一緒に移動
		hoverTarget_ += cameraMovement;
		lastCameraPosition_ = currentCameraPos;
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
	if (distanceToTarget < 1.0f) {
		return;
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
	// カメラコントローラーを取得
	CameraController* cameraController = CameraController::GetInstance();
	if (!cameraController) {
		return;
	}

	// カメラの現在位置と前方向を取得
	Vector3 cameraPos = cameraController->GetPosition();
	Vector3 cameraForward = cameraController->GetForward();

	// カメラの右方向と上方向を計算
	Vector3 worldUp = { 0.0f, 1.0f, 0.0f };
	Vector3 cameraRight = Normalize(Cross(cameraForward, worldUp));
	Vector3 cameraUp = Normalize(Cross(cameraRight, cameraForward));

	// カメラから基準距離離れた位置
	Vector3 basePosition = cameraPos + cameraForward * kBaseDistance;

	// カメラ座標系でランダムオフセットを生成
	float randomOffsetX = Random::GetInstance().GenerateFloat(-kOffsetRange, kOffsetRange);
	float randomOffsetY = Random::GetInstance().GenerateFloat(-kOffsetRange, kOffsetRange);

	// カメラの座標系に基づいてオフセットを適用
	hoverTarget_ = basePosition + cameraRight * randomOffsetX + cameraUp * randomOffsetY;
}