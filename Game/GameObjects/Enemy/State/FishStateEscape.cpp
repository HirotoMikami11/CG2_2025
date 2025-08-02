#include "FishStateEscape.h"
#include "GameObjects/Enemy/BaseEnemy.h"
#include "GameObjects/Player/Player.h"
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
	if (!GetPlayer()) {
		// プレイヤーがいない場合はデフォルト方向
		escapeDirection_ = Vector3{ 1.0f, 0.0f, 0.0f };
		return;
	}

	// プレイヤーの前方向を取得
	Vector3 playerForward = GetPlayer()->GetForward();

	// プレイヤーの前方向に垂直な方向を2つ計算
	// 上方向ベクトル（仮にY軸上方向とする）
	Vector3 up = { 0.0f, 1.0f, 0.0f };

	// 外積で垂直なベクトルを2つ作成
	Vector3 right = Cross(playerForward, up);
	Vector3 realUp = Cross(right, playerForward);

	right = Normalize(right);
	realUp = Normalize(realUp);

	// ランダムな角度で垂直方向を選択
	float randomAngle = Random::GetInstance().GenerateFloat(0.0f, 2.0f * 3.14159f); // 0～2π

	// 垂直平面内でランダムな方向を計算
	escapeDirection_ = right * std::cos(randomAngle) + realUp * std::sin(randomAngle);
	escapeDirection_ = Normalize(escapeDirection_);

	// カメラ外に向かうように、プレイヤーから遠ざかる成分を追加
	Vector3 enemyPos = enemy_->GetPosition();
	Vector3 playerPos = GetPlayer()->GetWorldPosition();
	Vector3 awayFromPlayer = Normalize(enemyPos - playerPos);

	// 逃走方向とプレイヤーから遠ざかる方向を合成
	escapeDirection_ = Normalize(escapeDirection_ + awayFromPlayer * 0.5f);
}