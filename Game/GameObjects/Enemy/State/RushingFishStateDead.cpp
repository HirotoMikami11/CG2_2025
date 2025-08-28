#include "RushingFishStateDead.h"
#include "GameObjects/Enemy/BaseEnemy.h"
#include "MyMath/MyMath.h"

RushingFishStateDead::RushingFishStateDead(BaseEnemy* enemy)
	: BaseEnemyState("Rushing Fish Dead", enemy) {

	// 現在の回転値を取得（速度による向き更新の前に取得）
	initialRotation_ = enemy_->GetRotation();

	// 目標回転値を設定（下向き = X軸回転で90度）
	targetRotation_ = initialRotation_;
	targetRotation_.x = float(M_PI) / 2.0f;  // 90度下向き

	// 初期透明度を保存
	Vector4 currentColor = enemy_->GetColor();
	initialAlpha_ = currentColor.w;

	// タイマーをリセット
	rotationTimer_ = 0.0f;
	fadeTimer_ = 0.0f;
	currentPhase_ = DeathPhase::Moving;

}

RushingFishStateDead::~RushingFishStateDead() {
}

void RushingFishStateDead::Update() {
	switch (currentPhase_) {
	case DeathPhase::Moving:
		UpdateMovingPhase();
		break;
	case DeathPhase::Fading:
		UpdateFadingPhase();
		break;
	}
}

void RushingFishStateDead::UpdateMovingPhase() {

	// 回転処理：イージングで下向きに
	rotationTimer_ += 1.0f / 60.0f;  // 60FPS想定
	float t = rotationTimer_ / kRotationDuration;

	// フェード処理：回転と同時に透明度を変更
	fadeTimer_ += 1.0f / 60.0f;
	float fadeT = fadeTimer_ / kFadeDuration;

	if (t >= 1.0f) {
		// 回転完了、フェード段階に移行
		t = 1.0f;
		currentPhase_ = DeathPhase::Fading;
		fadeTimer_ = 0.0f;
	}

	// イージングを適用して回転値を補間
	float easedT = EaseInQuart(t);
	Vector3 currentRotation = Lerp(initialRotation_, targetRotation_, easedT);
	enemy_->SetRotation(currentRotation);

	// 透明度の変更（回転と同時進行）
	if (fadeT <= 1.0f) {
		// 線形的なフェード（徐々に透明になる）
		float currentAlpha = initialAlpha_ * (1.0f - fadeT);

		// 現在の色を取得して透明度のみ変更
		Vector4 currentColor = enemy_->GetColor();
		currentColor.w = currentAlpha;
		enemy_->SetColor(currentColor);
	}
}

void RushingFishStateDead::UpdateFadingPhase() {

	// フェード処理：急激に透明度を下げる
	fadeTimer_ += 1.0f / 60.0f;  // 60FPS想定
	float t = fadeTimer_ / kFadeDuration;

	if (t >= 1.0f) {
		// フェード完了、死亡アニメーション完了を通知
		enemy_->CompleteDeathAnimation();
		return;
	}

	// 急激なフェード（二次関数的に透明度を下げる）
	float fadeT = 1.0f - (t * t);  // t^2で急激に下がる
	float currentAlpha = initialAlpha_ * fadeT;

	// 現在の色を取得して透明度のみ変更
	Vector4 currentColor = enemy_->GetColor();
	currentColor.w = currentAlpha;
	enemy_->SetColor(currentColor);
}