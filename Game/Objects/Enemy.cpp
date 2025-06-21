#include "Enemy.h"
#include "Player.h"
#include "Managers/ImGuiManager.h"

Enemy::Enemy() {
}

Enemy::~Enemy() {
}

void Enemy::Initialize(const Vector3& position) {
	// システム参照の取得
	directXCommon_ = Engine::GetInstance()->GetDirectXCommon();

	// 敵オブジェクトの初期化
	Object_ = std::make_unique<Model3D>();
	Object_->Initialize(directXCommon_, "resources/Model/Enemy", "enemy.obj");

	// 初期位置設定
	Object_->SetPosition(position);

	// 初期回転（左向き）
	Object_->SetRotation({ 0.0f, std::numbers::pi_v<float> *3.0f / 2.0f, 0.0f });

	// 移動する速度設定（左方向）
	velocity_ = { -kWalkSpeed, 0.0f, 0.0f };

	// 移動するアニメーションのタイマー
	walkTimer_ = 0.0f;
}

void Enemy::Update(const Matrix4x4& viewProjectionMatrix) {
	// 移動
	IsMove();

	// アニメーション
	IsAnimation();

	// オブジェクトの更新
	Object_->Update(viewProjectionMatrix);
}

void Enemy::OnCollision(const Player* player) {
	// 今回は使用しないので適当に配置しておく
	(void)player;
}

void Enemy::IsMove() {
	Vector3 currentPos = Object_->GetPosition();
	Vector3 newPos = Add(currentPos, velocity_);
	Object_->SetPosition(newPos);
}

void Enemy::IsAnimation() {
	// 毎フレームタイマーを加算
	walkTimer_ += 1.0f / 60.0f;

	// 回転アニメーション
	// sinを使って-1~1の範囲の振動を作成
	float param = std::sin(std::numbers::pi_v<float> *(walkTimer_ / kWalkMotionTime));

	// ((param+1.0f)/2.0f)で0~1の範囲に限定する
	// 0~1に限定したら、イージングして角度を補間する。
	float radian = Lerp(kWalkMotionAngleStart, kWalkMotionAngleEnd, ((param + 1.0f) / 2.0f));

	// それをラジアンに直して回転させる。
	Vector3 rotation = Object_->GetRotation();
	rotation.x = radian * std::numbers::pi_v<float> / 180.0f;
	Object_->SetRotation(rotation);
}

void Enemy::Draw(const Light& directionalLight) {
	// 3Dモデルを描画
	Object_->Draw(directionalLight);
}

AABB Enemy::GetAABB() const {
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;

	aabb.min = { worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f };
	aabb.max = { worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f };
	return aabb;
}

Vector3 Enemy::GetWorldPosition() const {
	// Model3Dから位置を取得
	return Object_->GetPosition();
}

void Enemy::ImGui() {
#ifdef _DEBUG
	ImGui::Text("Enemy");
	Object_->ImGui();

	// 物理パラメータの表示
	ImGui::Text("Velocity: %.3f, %.3f, %.3f", velocity_.x, velocity_.y, velocity_.z);
	ImGui::Text("Walk Timer: %.3f", walkTimer_);

	ImGui::Spacing();
#endif
}