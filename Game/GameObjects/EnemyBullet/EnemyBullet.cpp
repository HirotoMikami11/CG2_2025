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

	// プレイヤーの方向を向くように回転
	SetToPlayerDirection();
}

void EnemyBullet::Update(const Matrix4x4& viewProjectionMatrix) {
	// ホーミング
	velocity_ = IsHoming();

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

Vector3 EnemyBullet::GetWorldPosition() const {
	if (gameObject_) {
		return gameObject_->GetPosition();
	}
	return Vector3{ 0.0f, 0.0f, 0.0f };
}

Vector3 EnemyBullet::IsHoming() {
	if (!player_) {
		return velocity_;
	}

	// ホーミングの計算
	Vector3 toPlayer = player_->GetWorldPosition() - GetWorldPosition();

	// それぞれ正規化
	toPlayer = Normalize(toPlayer);
	velocity_ = Normalize(velocity_);

	// 球面線形補間で今の速度とプレイヤーのベクトルを内挿し、新たな速度とする
	velocity_ = Slerp(velocity_, toPlayer, t_) * bulletSpeed_;

	return velocity_;
}

void EnemyBullet::SetToPlayerDirection() {
	if (!player_) {
		return;
	}

	// プレイヤーの方向を向くように回転
	Vector3 rotation = gameObject_->GetRotation();

	// Y軸周り角度（水平回転）
	rotation.y = std::atan2(velocity_.x, velocity_.z);

	// 横軸方向の長さを求める
	float XZLength = std::sqrt(velocity_.x * velocity_.x + velocity_.z * velocity_.z);

	// X軸周り角度（垂直回転）
	rotation.x = std::atan2(-velocity_.y, XZLength);

	gameObject_->SetRotation(rotation);
}