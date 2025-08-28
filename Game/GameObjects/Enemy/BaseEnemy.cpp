#include "BaseEnemy.h"
#include "GameObjects/Player/Player.h"
#include "Scenes/GameScene/GameScene.h"
#include "Managers/Audio/AudioManager.h"
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

	// 元の色を保存（初期化後に呼び出す）
	SaveOriginalColor();
}

void BaseEnemy::OnCollision(Collider* other) {
	if (!other) return;

	// 死亡アニメーション中は衝突処理をスキップ
	if (isPlayingDeathAnimation_) {
		return;
	}

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
	// 死亡アニメーション中はダメージを受けない
	if (isPlayingDeathAnimation_) {
		return 0.0f;
	}

	// Colliderベースのダメージ処理を呼び出し（HPが0になる）
	float actualDamage = Collider::TakeDamage(damage);

	// ダメージエフェクトを開始
	StartDamageEffect();

	// HPが0以下になったら死亡アニメーションを開始
	if (GetCurrentHP() <= 0.0f) {
		isPlayingDeathAnimation_ = true;
		isDeathAnimationComplete_ = false;
		// 敵死亡音を再生
		AudioManager* audioManager = AudioManager::GetInstance();
		if (audioManager) {
			audioManager->Play("EnemyDead");
		}
		OnDeath(); // 各派生クラスで実装される死亡処理
	}

	return actualDamage;
}
void BaseEnemy::KillOffscreenEnemy() {
	// 既に死亡アニメーション中の場合は何もしない
	if (isPlayingDeathAnimation_) {
		return;
	}

	// HPを直接設定
	SetCurrentHP(0);

	// 死亡アニメーションフラグを設定
	isPlayingDeathAnimation_ = true;
	isDeathAnimationComplete_ = false;
	// 各派生クラスの死亡処理を呼び出し
	OnDeath();
}

void BaseEnemy::StartDamageEffect() {
	// ダメージエフェクトを開始
	isDamageEffectActive_ = true;
	damageEffectTimer_ = damageEffectDuration_;

	// すぐに赤色に変更
	if (gameObject_) {
		gameObject_->SetColor(damageColor_);
	}

}

void BaseEnemy::UpdateDamageEffect() {
	// ダメージエフェクトが有効でない場合は何もしない
	if (!isDamageEffectActive_ || !gameObject_) {
		return;
	}

	// タイマーを減少
	damageEffectTimer_--;

	// エフェクト終了チェック
	if (damageEffectTimer_ <= 0.0f) {
		// エフェクト終了：元の色に戻す
		gameObject_->SetColor(originalColor_);
		isDamageEffectActive_ = false;
		damageEffectTimer_ = 0.0f;
		return;
	}

	// イージング計算（0.0f～1.0f の範囲で補間）
	float t = 1.0f - (damageEffectTimer_ / damageEffectDuration_);
	float easedT = EaseOutQuart(t);

	// Lerp を使用して色を補間（赤色→元の色）
	Vector4 currentColor;
	currentColor.x = Lerp(damageColor_.x, originalColor_.x, easedT);
	currentColor.y = Lerp(damageColor_.y, originalColor_.y, easedT);
	currentColor.z = Lerp(damageColor_.z, originalColor_.z, easedT);
	currentColor.w = Lerp(damageColor_.w, originalColor_.w, easedT);

	// 補間した色を適用
	gameObject_->SetColor(currentColor);
}

void BaseEnemy::SaveOriginalColor() {
	// ゲームオブジェクトが存在する場合のみ色を保存
	if (gameObject_) {
		originalColor_ = gameObject_->GetColor();
	}
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

