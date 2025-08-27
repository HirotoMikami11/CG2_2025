#include "PlayerBullet.h"

PlayerBullet::PlayerBullet() {
}

PlayerBullet::~PlayerBullet() {
}

void PlayerBullet::Initialize(DirectXCommon* dxCommon, const Vector3& position, const Vector3& velocity) {
	directXCommon_ = dxCommon;
	velocity_ = velocity;

	// ゲームオブジェクト（球体）の初期化
	gameObject_ = std::make_unique<Model3D>();
	gameObject_->Initialize(dxCommon, "playerBullet", "white");
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

	// 速度の方向を向くように回転
	SetToVelocityDirection();

	// 衝突判定設定
	SetRadius(1.0f); // Colliderの半径をセット
	SetAttackPower(1.0f); // プレイヤーの弾の攻撃力
	SetMaxHP(1.0f); // 弾のHPは1

	/// 衝突属性の設定
	SetCollisionAttribute(kCollisionAttributePlayer);
	/// 衝突対象は自分の属性以外に設定(ビット反転)
	SetCollisionMask(~kCollisionAttributePlayer);
}

void PlayerBullet::Update(const Matrix4x4& viewProjectionMatrix) {
	// 座標を移動させる
	Vector3 currentPos = gameObject_->GetPosition();
	currentPos += velocity_;
	gameObject_->SetPosition(currentPos);

	// タイマーを減らす
	if (--deathTimer_ <= 0) {
		TakeDamage(GetMaxHP()); // 自分のHPを0にして削除
	}

	// ゲームオブジェクトの更新
	gameObject_->Update(viewProjectionMatrix);
}

void PlayerBullet::Draw(const Light& directionalLight) {

	gameObject_->Draw(directionalLight);

}

Vector3 PlayerBullet::GetWorldPosition() {
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

void PlayerBullet::OnCollision(Collider* other) {
	if (!other) return;

	// 衝突相手が敵の属性を持つかチェック
	if (other->GetCollisionAttribute() & kCollisionAttributeEnemy) {
		// 弾は衝突すると消滅
		TakeDamage(GetMaxHP()); // 自分のHPを0にして消滅
	}
}

void PlayerBullet::SetToVelocityDirection() {
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