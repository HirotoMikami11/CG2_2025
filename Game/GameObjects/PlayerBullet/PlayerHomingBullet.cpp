#include "PlayerHomingBullet.h"
#include "GameObjects/Enemy/Enemy.h"

PlayerHomingBullet::PlayerHomingBullet() {
}

PlayerHomingBullet::~PlayerHomingBullet() {
}

void PlayerHomingBullet::Initialize(DirectXCommon* dxCommon, const Vector3& position, const Vector3& velocity, Enemy* target) {
	directXCommon_ = dxCommon;
	velocity_ = velocity;
	target_ = target;

	// ゲームオブジェクト（弾）の初期化
	gameObject_ = std::make_unique<Model3D>();
	gameObject_->Initialize(dxCommon, "playerBullet", "white");
	gameObject_->SetName("PlayerHomingBullet");

	// 初期位置設定
	gameObject_->SetPosition(position);

	// ホーミング弾らしい見た目にするため細長くして色を変える
	Vector3Transform bulletTransform{
		{0.5f, 0.5f, 3.0f},   // scale - 細長く
		{0.0f, 0.0f, 0.0f},   // rotate
		position              // translate
	};
	gameObject_->SetTransform(bulletTransform);

	// ホーミング弾の色を設定（黄色で区別）
	gameObject_->SetColor({ 1.0f, 1.0f, 0.0f, 1.0f });

	// ホーミング用パラメータの初期化
	bulletSpeed_ = Length(velocity_); // 初期速度の大きさを保存
	t_ = 0.05f;                      // ホーミングの補間割合

	// ターゲットの方向を向くように回転
	SetToTargetDirection();

	// 衝突判定設定
	SetRadius(1.0f); // Colliderの半径をセット
	/// 衝突属性の設定
	SetCollisionAttribute(kCollisionAttributePlayer);
	/// 衝突対象は自分の属性以外に設定(ビット反転)
	SetCollisionMask(~kCollisionAttributePlayer);
}

void PlayerHomingBullet::Update(const Matrix4x4& viewProjectionMatrix) {
	// ホーミング処理
	velocity_ = IsHoming();

	// ターゲットの方向を向くように回転
	SetToTargetDirection();

	// 座標を移動させる
	Vector3 currentPos = gameObject_->GetPosition();
	currentPos = currentPos + velocity_;
	gameObject_->SetPosition(currentPos);

	// ゲームオブジェクトの更新
	gameObject_->Update(viewProjectionMatrix);

	// 時間経過でデス
	if (--deathTimer_ <= 0) {
		isDead_ = true;
	}
}

void PlayerHomingBullet::Draw(const Light& directionalLight) {
	if (!isDead_) {
		gameObject_->Draw(directionalLight);
	}
}

Vector3 PlayerHomingBullet::GetWorldPosition() {
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

void PlayerHomingBullet::OnCollision() {
	isDead_ = true; // デスフラグを立てる
}

Vector3 PlayerHomingBullet::IsHoming() {
	// ターゲットが存在しない場合はホーミングしない
	if (target_ == nullptr) {
		return velocity_; // 最後の方向にまっすぐ飛ぶ
	}

	// ターゲットが死んでいる場合は、ターゲットをnullにして直進
	if (target_->IsDead()) {
		target_ = nullptr; // ターゲットを無効化
		return velocity_;  // 最後の方向にまっすぐ飛ぶ
	}

	// ホーミングの計算
	Vector3 toTarget = target_->GetWorldPosition() - GetWorldPosition();

	// それぞれ正規化
	toTarget = Normalize(toTarget);
	Vector3 normalizedVelocity = Normalize(velocity_);

	// 球面線形補間で今の速度とターゲットのベクトルを内挿し、新たな速度とする
	Vector3 newVelocity = Slerp(normalizedVelocity, toTarget, t_) * bulletSpeed_;

	return newVelocity;
}

void PlayerHomingBullet::SetToTargetDirection() {
	if (gameObject_) {
		// 速度の方向を向くように回転
		Vector3 rotation = gameObject_->GetRotation();

		// Y軸周り角度（水平回転）
		rotation.y = std::atan2(velocity_.x, velocity_.z);

		// 横軸方向の長さを求める
		float XZLength = std::sqrt(velocity_.x * velocity_.x + velocity_.z * velocity_.z);

		// X軸周り角度（垂直回転）
		rotation.x = std::atan2(-velocity_.y, XZLength);

		gameObject_->SetRotation(rotation);
	}
}