#include "BaseEnemyState.h"
#include "GameObjects/Enemy/BaseEnemy.h"
#include "GameObjects/Player/Player.h"

Player* BaseEnemyState::GetPlayer() const {
	if (enemy_) {
		return enemy_->GetPlayer();
	}
	return nullptr;
}