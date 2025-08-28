#include "GameObjects/Enemy/State/FishStateApproach.h"
#include "GameObjects/Enemy/State/FishStateHover.h"
#include "GameObjects/Enemy/BaseEnemy.h"
#include "GameObjects/Player/Player.h"
#include "CameraController/CameraController.h"
#include "MyMath/Random/Random.h"

FishStateApproach::FishStateApproach(BaseEnemy* enemy) : BaseEnemyState("Shoting Fish Approach", enemy) {
	// 初期目標位置を計算
	CalculateTargetPosition();
}

FishStateApproach::~FishStateApproach() {
}

void FishStateApproach::Update() {
	// プレイヤーがいない場合は何もしない
	if (!GetPlayer()) {
		return;
	}

	// カメラコントローラーを取得
	CameraController* cameraController = CameraController::GetInstance();
	if (!cameraController) {
		return;
	}

	// カメラが動いた場合は目標位置をカメラの前方向基準で再計算
	Vector3 currentCameraPos = cameraController->GetPosition();
	Vector3 currentCameraForward = cameraController->GetForward();
	if (Distance(currentCameraPos, lastCameraPosition_) > kCameraMovementThreshold ||
		Distance(currentCameraForward, lastCameraForward_) > kCameraRotationThreshold) {

		// カメラの新しい座標系を計算
		Vector3 worldUp = { 0.0f, 1.0f, 0.0f };
		Vector3 newCameraRight = Normalize(Cross(currentCameraForward, worldUp));
		Vector3 newCameraUp = Normalize(Cross(newCameraRight, currentCameraForward));

		// カメラから基準距離離れた基準位置
		Vector3 basePosition = currentCameraPos + currentCameraForward * kTargetDistance;

		// 保存されたオフセットを新しいカメラ座標系で再適用
		targetPosition_ = basePosition + newCameraRight * savedOffsetX_ + newCameraUp * savedOffsetY_;

		// 前回の値を更新
		lastCameraPosition_ = currentCameraPos;
		lastCameraForward_ = currentCameraForward;
	}

	Vector3 enemyPos = enemy_->GetPosition();

	// 目標位置への方向ベクトルを計算
	Vector3 toTarget = targetPosition_ - enemyPos;
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

void FishStateApproach::CalculateTargetPosition() {
	// カメラコントローラーを取得
	CameraController* cameraController = CameraController::GetInstance();
	if (!cameraController) {
		return;
	}

	// カメラの現在位置と前方向を取得
	Vector3 cameraPos = cameraController->GetPosition();
	Vector3 cameraForward = cameraController->GetForward();
	lastCameraPosition_ = cameraPos;
	lastCameraForward_ = cameraForward;

	// カメラの右方向と上方向を計算
	Vector3 worldUp = { 0.0f, 1.0f, 0.0f };
	Vector3 cameraRight = Normalize(Cross(cameraForward, worldUp));
	Vector3 cameraUp = Normalize(Cross(cameraRight, cameraForward));

	// カメラから kTargetDistance 離れた基準位置
	Vector3 basePosition = cameraPos + cameraForward * kTargetDistance;

	// x,y方向に -+10 の範囲でランダムオフセットを生成して保存
	savedOffsetX_ = Random::GetInstance().GenerateFloat(-kOffsetRange, kOffsetRange);
	savedOffsetY_ = Random::GetInstance().GenerateFloat(-kOffsetRange, kOffsetRange);

	// カメラの座標系に基づいてオフセットを適用
	targetPosition_ = basePosition + cameraRight * savedOffsetX_ + cameraUp * savedOffsetY_;
}