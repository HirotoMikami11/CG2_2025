#include "ShootingFishStateDead.h"
#include "GameObjects/Enemy/BaseEnemy.h"
#include "MyMath/MyMath.h"

ShootingFishStateDead::ShootingFishStateDead(BaseEnemy* enemy)
	: BaseEnemyState("Shooting Fish Dead", enemy) {

	// 現在の回転値を取得（速度による向き更新の前に取得）
	initialRotation_ = enemy_->GetRotation();

	// 上方向への移動速度を設定
	deathVelocity_ = { 0.0f, kUpwardSpeed, 0.0f };

	// 目標回転値を設定（下向き = X軸回転で90度）
	targetRotation_ = initialRotation_;
	targetRotation_.x = float(M_PI) / 2.0f;  // 90度下向き

	// 初期透明度を保存
	Vector4 currentColor = enemy_->GetColor();
	initialAlpha_ = currentColor.w;

	// タイマーをリセット
	rotationTimer_ = 0.0f;
	fadeTimer_ = 0.0f;

	// 速度を設定
	enemy_->SetVelocity(deathVelocity_);
}

ShootingFishStateDead::~ShootingFishStateDead() {
}

void ShootingFishStateDead::Update() {
	// 上方向への移動処理
	/*Vector3 currentPos = enemy_->GetPosition();
	Vector3 newPosition = currentPos + deathVelocity_;
	enemy_->SetPosition(newPosition);*/

	// 速度を維持
	enemy_->SetVelocity(deathVelocity_);

	// 回転処理：イージングで下向きに
	rotationTimer_ += 1.0f / 60.0f;  // 60FPS想定
	float rotationT = rotationTimer_ / kRotationDuration;

	// 回転の進行度を制限
	if (rotationT > 1.0f) {
		rotationT = 1.0f;
	}

	// イージングを適用して回転値を補間
	float easedRotationT = EaseOutSine(rotationT);
	Vector3 currentRotation = Lerp(initialRotation_, targetRotation_, easedRotationT);
	enemy_->SetRotation(currentRotation);

	// フェード処理：回転と同時に透明度を下げる
	fadeTimer_ += 1.0f / 60.0f;  // 60FPS想定
	float fadeT = fadeTimer_ / kFadeDuration;

	if (fadeT >= 1.0f) {
		// フェード完了、死亡アニメーション完了を通知
		enemy_->CompleteDeathAnimation();
		return;
	}

	// 線形的なフェード（徐々に透明になる）
	float currentAlpha = initialAlpha_ * (1.0f - fadeT);

	// 現在の色を取得して透明度のみ変更
	Vector4 currentColor = enemy_->GetColor();
	currentColor.w = currentAlpha;
	enemy_->SetColor(currentColor);
}