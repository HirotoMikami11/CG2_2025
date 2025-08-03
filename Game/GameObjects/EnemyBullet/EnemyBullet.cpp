#include "EnemyBullet.h"
#include "GameObjects/Player/Player.h"

EnemyBullet::EnemyBullet() {
}

EnemyBullet::~EnemyBullet() {
}

void EnemyBullet::Initialize(DirectXCommon* dxCommon, const Vector3& position, const Vector3& velocity, const float& speed) {
	directXCommon_ = dxCommon;
	velocity_ = velocity;
	bulletSpeed_ = speed;
	t_ = 0.055f; // ホーミング補間割合

	homingTimer_ = kHomingTime_;

	// ゲームオブジェクト（球体）の初期化
	gameObject_ = std::make_unique<Sphere>();
	gameObject_->Initialize(dxCommon, "sphere", "white");
	gameObject_->SetName("EnemyBullet");

	// 初期位置設定
	gameObject_->SetPosition(position);

	// 弾らしい見た目にするため細長くする
	Vector3Transform bulletTransform{
		{0.5f, 0.5f, 3.0f},   // scale - 細長く
		{0.0f, 0.0f, 0.0f},   // rotate
		position              // translate
	};
	gameObject_->SetTransform(bulletTransform);

	// 弾の色を設定（赤色で敵の弾とわかりやすく）
	gameObject_->SetColor({ 1.0f, 0.2f, 0.2f, 1.0f });

	// 衝突判定設定
	SetRadius(1.0f);
	SetCollisionAttribute(kCollisionAttributeEnemy);
	SetCollisionMask(~kCollisionAttributeEnemy);

	SetToPlayerDirection();
}

void EnemyBullet::Update(const Matrix4x4& viewProjectionMatrix) {

	// ホーミング時間中のみホーミング処理
	if (homingTimer_ > 0.0f) {
		velocity_ = IsHoming();
		homingTimer_ -- ;
	}

	// プレイヤーの方向を向くように回転
	SetToPlayerDirection();

	// 座標を移動させる
	Vector3 currentPos = gameObject_->GetPosition();
	currentPos += velocity_;
	gameObject_->SetPosition(currentPos);

	// ゲームオブジェクトの更新
	gameObject_->Update(viewProjectionMatrix);

	// 時間経過でデス
	if (--deathTimer_ <= 0) {
		isDead_ = true;
	}
}

void EnemyBullet::Draw(const Light& directionalLight) {
	if (!isDead_) {
		gameObject_->Draw(directionalLight);
	}
}

Vector3 EnemyBullet::GetWorldPosition() {
	if (gameObject_) {
		Matrix4x4 worldMatrix = gameObject_->GetTransform().GetWorldMatrix();
		return Vector3{
			worldMatrix.m[3][0],
			worldMatrix.m[3][1],
			worldMatrix.m[3][2]
		};
	}
	return Vector3{ 0.0f, 0.0f, 0.0f };
}

Vector3 EnemyBullet::IsHoming() {
	if (!player_ || player_ == nullptr) {
		return velocity_;
	}

	Vector3 toPlayer = player_->GetWorldPosition() - GetWorldPosition();
	toPlayer = Normalize(toPlayer);
	velocity_ = Normalize(velocity_);
	velocity_ = Slerp(velocity_, toPlayer, t_) * bulletSpeed_;
	return velocity_;
}

void EnemyBullet::OnCollision() {
	TakeDamage(1);
}

void EnemyBullet::TakeDamage(int damage) {
	hp_ -= damage;
	if (hp_ <= 0) {
		isDead_ = true;
	}
}

void EnemyBullet::SetToPlayerDirection() {
	if (!player_ || player_ == nullptr) {
		return;
	}

	Vector3 rotation = gameObject_->GetRotation();
	rotation.y = std::atan2(velocity_.x, velocity_.z);
	float XZLength = std::sqrt(velocity_.x * velocity_.x + velocity_.z * velocity_.z);
	rotation.x = std::atan2(-velocity_.y, XZLength);
	gameObject_->SetRotation(rotation);
}
