#include "FishStateEscape.h"
#include "GameObjects/Enemy/BaseEnemy.h"
#include "GameObjects/Player/Player.h"
#include "CameraController/CameraController.h"
#include "MyMath/Random/Random.h"

FishStateEscape::FishStateEscape(BaseEnemy* enemy) : BaseEnemyState("Shoting Fish Escape", enemy) {
	// 逃走方向を計算
	CalculateEscapeDirection();

	// 逃走速度を設定
	Vector3 escapeVelocity = escapeDirection_ * kEscapeSpeed;
	enemy_->SetVelocity(escapeVelocity);
}

FishStateEscape::~FishStateEscape() {
}

void FishStateEscape::Update() {
	// 一定方向に移動し続ける
	Vector3 currentPos = enemy_->GetPosition();
	Vector3 newPosition = currentPos + enemy_->GetVelocity();
	enemy_->SetPosition(newPosition);
}

void FishStateEscape::CalculateEscapeDirection() {
	// カメラコントローラーを取得
	CameraController* cameraController = CameraController::GetInstance();
	if (!cameraController) {
		// カメラコントローラーがない場合はデフォルト方向
		escapeDirection_ = Vector3{ 1.0f, 0.0f, 0.0f };
		return;
	}

	// カメラの前方向を取得（レールカメラの進行方向）
	Vector3 cameraForward = cameraController->GetForward();
	Vector3 cameraPos = cameraController->GetPosition();

	// カメラの座標系を計算
	Vector3 worldUp = { 0.0f, 1.0f, 0.0f };
	Vector3 cameraRight = Normalize(Cross(cameraForward, worldUp));
	Vector3 cameraUp = Normalize(Cross(cameraRight, cameraForward));

	// カメラの前方向に垂直な平面でランダムな方向を生成
	float randomAngle = Random::GetInstance().GenerateFloat(0.0f, 2.0f * 3.14159f); // 0～2π

	// カメラの座標系での垂直方向を計算
	Vector3 perpendicularDirection = cameraRight * std::cos(randomAngle) + cameraUp * std::sin(randomAngle);

	// カメラから遠ざかる成分を追加
	Vector3 enemyPos = enemy_->GetPosition();
	Vector3 awayFromCamera = Normalize(enemyPos - cameraPos);

	// 垂直方向とカメラから遠ざかる方向を合成
	// より強くカメラから遠ざかるように重み付け
	escapeDirection_ = Normalize(perpendicularDirection * 0.3f + awayFromCamera * 0.7f);

	// 後方への逃走も考慮（カメラの後方へ逃げる可能性も追加）
	float backwardWeight = Random::GetInstance().GenerateFloat(0.0f, 0.5f);
	escapeDirection_ = Normalize(escapeDirection_ - cameraForward * backwardWeight);
}