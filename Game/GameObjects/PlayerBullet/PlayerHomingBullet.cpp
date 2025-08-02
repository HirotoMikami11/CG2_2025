#include "PlayerHomingBullet.h"
#include "GameObjects/Enemy/BaseEnemy.h"
#include "Managers/ImGui/ImGuiManager.h"

PlayerHomingBullet::PlayerHomingBullet() {
}

PlayerHomingBullet::~PlayerHomingBullet() {
	// unique_ptrで自動的に解放される
}

void PlayerHomingBullet::Initialize(DirectXCommon* dxCommon, const Vector3& position, const Vector3& velocity, BaseEnemy* target) {
	directXCommon_ = dxCommon;

	// ゲームオブジェクト（モデル）の初期化
	gameObject_ = std::make_unique<Model3D>();
	gameObject_->Initialize(dxCommon, "playerHomingBullet");
	gameObject_->SetName("PlayerHomingBullet");

	// 初期位置設定
	gameObject_->SetPosition(position);

	// 初期速度設定
	velocity_ = velocity;

	// ターゲット設定
	target_ = target;

	// 衝突判定設定
	SetRadius(0.5f); // Colliderの半径をセット
	/// 衝突属性の設定
	SetCollisionAttribute(kCollisionAttributePlayer);
	/// 衝突対象は敵に設定
	SetCollisionMask(kCollisionAttributeEnemy);
}

void PlayerHomingBullet::Update(const Matrix4x4& viewProjectionMatrix) {
	// ホーミング処理
	UpdateHoming();

	// 移動処理
	Vector3 currentPos = gameObject_->GetPosition();
	currentPos += velocity_;
	gameObject_->SetPosition(currentPos);

	// 時間経過による削除処理
	deathTimer_--;
	if (deathTimer_ <= 0) {
		isDead_ = true;
	}

	// 画面外に出た場合の削除処理（適当な範囲で判定）
	if (currentPos.x < -100.0f || currentPos.x > 100.0f ||
		currentPos.y < -100.0f || currentPos.y > 100.0f ||
		currentPos.z < -50.0f || currentPos.z > 300.0f) {
		isDead_ = true;
	}

	// ゲームオブジェクトの更新
	gameObject_->Update(viewProjectionMatrix);
}

void PlayerHomingBullet::Draw(const Light& directionalLight) {
	// 弾の描画
	gameObject_->Draw(directionalLight);
}

void PlayerHomingBullet::ImGui() {
#ifdef _DEBUG
	if (ImGui::TreeNode("PlayerHomingBullet")) {
		if (gameObject_) {
			// 座標調整
			Vector3 pos = gameObject_->GetPosition();
			if (ImGui::DragFloat3("HomingBullet_Translate", &pos.x, 0.1f)) {
				gameObject_->SetPosition(pos);
			}

			// 速度調整
			if (ImGui::DragFloat3("HomingBullet_Velocity", &velocity_.x, 0.01f)) {
				// 速度が変更された場合の処理
			}
		}

		ImGui::Text("Death Timer: %d", deathTimer_);
		ImGui::Text("Is Dead: %s", isDead_ ? "YES" : "NO");
		ImGui::Text("Has Target: %s", (target_ != nullptr) ? "YES" : "NO");
		if (target_ != nullptr) {
			Vector3 targetPos = target_->GetWorldPosition();
			ImGui::Text("Target Position: (%.2f, %.2f, %.2f)", targetPos.x, targetPos.y, targetPos.z);
		}

		ImGui::TreePop();
	}
#endif
}

void PlayerHomingBullet::OnCollision() {
	// 敵に当たったら弾は消える
	isDead_ = true;
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

void PlayerHomingBullet::UpdateHoming() {
	// ターゲットが存在しない、または死んでいる場合はホーミングしない
	if (target_ == nullptr || target_->IsDead()) {
		return;
	}

	// ターゲットの位置を取得
	Vector3 targetPosition = target_->GetWorldPosition();
	Vector3 currentPosition = gameObject_->GetPosition();

	// ターゲットへの方向ベクトルを計算
	Vector3 toTarget = targetPosition - currentPosition;
	toTarget = Normalize(toTarget);

	// 現在の速度を正規化
	Vector3 currentDirection = Normalize(velocity_);

	// ホーミング：現在の方向とターゲットへの方向を補間
	Vector3 newDirection = Lerp(currentDirection, toTarget, kHomingStrength);
	newDirection = Normalize(newDirection);

	// 新しい速度を設定（速度の大きさは保持）
	velocity_ = newDirection * kMaxSpeed;
}