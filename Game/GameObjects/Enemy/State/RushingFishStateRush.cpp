#include "RushingFishStateRush.h"
#include "GameObjects/Enemy/BaseEnemy.h"
#include "GameObjects/Player/Player.h"

RushingFishStateRush::RushingFishStateRush(BaseEnemy* enemy, const Vector3& rushVelocity)
	: BaseEnemyState("Rushing Fish Rush", enemy) {
}

RushingFishStateRush::~RushingFishStateRush() {
}

void RushingFishStateRush::Update() {
	// 直進処理：現在の突進速度で移動し続ける
	Vector3 currentPos = enemy_->GetPosition();
	Vector3 newPosition = currentPos + enemy_->GetVelocity();
	enemy_->SetPosition(newPosition);



	//消滅処理を描く(今後もっとちゃんとした奴角)

	// プレイヤーがいない場合は何もしない
	if (!GetPlayer()) {
		return;
	}

	// プレイヤーの位置を取得
	Vector3 playerPos = GetPlayer()->GetWorldPosition();
	Vector3 enemyPos = enemy_->GetPosition();

	// プレイヤーとの距離を計算
	float distanceToPlayer = Distance(playerPos, enemyPos);


	//自機から離れたら死亡
	if (distanceToPlayer >= 200) {
		enemy_->Collider::IsDead();
		return;
	}

}
