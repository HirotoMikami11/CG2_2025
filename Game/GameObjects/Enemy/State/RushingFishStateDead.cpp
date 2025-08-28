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
	deathTimer_ = 0.0f;
}

RushingFishStateDead::~RushingFishStateDead() {
}

void RushingFishStateDead::Update() {
	// 死亡アニメーションのタイマーを更新
	deathTimer_ += 1.0f / 60.0f;  // 60FPS想定
	float t = deathTimer_ / kDeathDuration;

	if (t >= 1.0f) {
		// アニメーション完了、死亡アニメーション完了を通知
		enemy_->CompleteDeathAnimation();
		return;
	}

	// 回転処理：イージングで下向きに
	float easedT = EaseInQuart(t);
	Vector3 currentRotation = Lerp(initialRotation_, targetRotation_, easedT);
	enemy_->SetRotation(currentRotation);

	// フェード処理：回転と同じ速度で透明度を下げる
	float currentAlpha = initialAlpha_ * (1.0f - t);

	// 現在の色を取得して透明度のみ変更
	Vector4 currentColor = enemy_->GetColor();
	currentColor.w = currentAlpha;
	enemy_->SetColor(currentColor);
}