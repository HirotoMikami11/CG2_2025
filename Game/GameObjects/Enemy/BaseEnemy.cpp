#include "BaseEnemy.h"
#include "GameObjects/Player/Player.h"
#include "Scenes/GameScene/GameScene.h"

BaseEnemy::BaseEnemy(EnemyType type) : enemyType_(type) {
}

BaseEnemy::~BaseEnemy() {
	// 全ての時限発動をクリア
	timedCalls_.clear();
}

void BaseEnemy::ChangeState(std::unique_ptr<BaseEnemyState> state) {
	// 引数で受け取った状態を次の状態としてセットする
	state_ = std::move(state);
}

void BaseEnemy::UpdateTimedCalls() {
	// １フレ遅れさせてからクリアの実行
	if (shouldClearTimedCalls_) {
		timedCalls_.clear();
		shouldClearTimedCalls_ = false;
		return; // 今回は更新をスキップ
	}

	// 終了した時限発動の削除
	timedCalls_.remove_if([](const std::unique_ptr<TimedCall>& timedCall) {
		return timedCall->IsFinished();
		});

	// 時限発動の更新
	for (auto& timedCall : timedCalls_) {
		timedCall->Update();
	}
}

void BaseEnemy::ClearTimeCalls() {
	// 次のフレームでクリアする
	shouldClearTimedCalls_ = true;
}

Vector3 BaseEnemy::GetWorldPosition() {
	if (gameObject_) {
		// Transform3DのWorld行列から移動成分を取得
		Matrix4x4 worldMatrix = gameObject_->GetTransform().GetWorldMatrix();
		return Vector3{
			worldMatrix.m[3][0],
			worldMatrix.m[3][1],
			worldMatrix.m[3][2]
		};
	}
	return Vector3{ 0.0f, 0.0f, 0.0f };
}

void BaseEnemy::SetEnemyStats(float hp, float attackPower) {
	SetMaxHP(hp);
	SetAttackPower(attackPower);

	// 衝突判定の基本設定
	SetCollisionAttribute(kCollisionAttributeEnemy);
	SetCollisionMask(~kCollisionAttributeEnemy);
}

void BaseEnemy::OnCollision(Collider* other) {
	if (!other) return;

	// 衝突相手がプレイヤー陣営かチェック
 	if (other->GetCollisionAttribute() & kCollisionAttributePlayer) {
		// プレイヤーの弾に当たった場合はダメージを受ける
		float damage = other->GetAttackPower();
		TakeDamage(damage);
#ifdef _DEBUG
		Logger::LogF("Enemy took %.1f damage. Remaining HP: %.1f\n", damage, GetCurrentHP());
#endif
	}
}

float BaseEnemy::TakeDamage(float damage) {
	// Colliderベースのダメージ処理を呼び出し
	float actualDamage = Collider::TakeDamage(damage);

	// HPが0以下になったら死亡フラグを立てる
	if (GetCurrentHP() <= 0.0f) {
		isDead_ = true;
	}

	return actualDamage;
}


void BaseEnemy::SetToVelocityDirection() {
	if (gameObject_) {
		// 速度の方向を向くように回転
		Vector3 rotation = gameObject_->GetRotation();

		// Y軸周り角度
		rotation.y = std::atan2(velocity_.x, velocity_.z);

		// 横軸方向の長さを求める
		float XZLength = std::sqrt(velocity_.x * velocity_.x + velocity_.z * velocity_.z);

		// X軸周り角度
		rotation.x = std::atan2(-velocity_.y, XZLength);

		gameObject_->SetRotation(rotation);
	}
}