#include "PlayerBullet.h"

PlayerBullet::PlayerBullet() {
}

PlayerBullet::~PlayerBullet() {
}

void PlayerBullet::Initialize(DirectXCommon* dxCommon, const Vector3& position, const Vector3& velocity) {
	directXCommon_ = dxCommon;
	velocity_ = velocity;

	// ゲームオブジェクト（球体）の初期化
	gameObject_ = std::make_unique<Sphere>();
	gameObject_->Initialize(dxCommon, "sphere", "white");
	gameObject_->SetName("PlayerBullet");

	// 初期位置設定
	gameObject_->SetPosition(position);

	// 弾らしい見た目にするため細長くする
	Vector3Transform bulletTransform{
		{0.5f, 0.5f, 3.0f},   // scale - 細長く
		{0.0f, 0.0f, 0.0f},   // rotate
		position              // translate
	};
	gameObject_->SetTransform(bulletTransform);

	// 弾の色を設定（白色）
	gameObject_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
}

void PlayerBullet::Update(const Matrix4x4& viewProjectionMatrix) {
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

void PlayerBullet::Draw(const Light& directionalLight) {
	if (!isDead_) {
		gameObject_->Draw(directionalLight);
	}
}

Vector3 PlayerBullet::GetWorldPosition() const {
	if (gameObject_) {
		return gameObject_->GetPosition();
	}
	return Vector3{ 0.0f, 0.0f, 0.0f };
}