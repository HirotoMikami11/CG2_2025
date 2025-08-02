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
}
