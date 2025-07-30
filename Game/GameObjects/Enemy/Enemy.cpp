#include "Enemy.h"
#include "GameObjects/Player/Player.h"
#include "Managers/ImGui/ImGuiManager.h"
#include "Scenes/GameScene/GameScene.h"

Enemy::Enemy() {
}

Enemy::~Enemy() {
	// 全ての時限発動をクリア
	timedCalls_.clear();
	// AutoFireフラグを下す
	isAutoFire_ = false;
}

void Enemy::Initialize(DirectXCommon* dxCommon, const Vector3& position, EnemyPattern pattern) {
	directXCommon_ = dxCommon;

	// ゲームオブジェクト（球体）の初期化
	gameObject_ = std::make_unique<Sphere>();
	gameObject_->Initialize(dxCommon, "sphere", "uvChecker");
	gameObject_->SetName("Enemy");

	// 初期位置設定
	gameObject_->SetPosition(position);

	// 大きさを一時的に大きくする
	Vector3Transform enemyTransform{
		{3.0f, 3.0f, 3.0f},   // scale
		{0.0f, 0.0f, 0.0f},   // rotate
		position              // translate
	};
	gameObject_->SetTransform(enemyTransform);

	// 速度をセット
	velocity_ = { 0, 0, 1 };

	// パターンの設定
	pattern_ = pattern;

	// 衝突判定設定
	SetRadius(3.0f); // Colliderの半径をセット
	/// 衝突属性の設定
	SetCollisionAttribute(kCollisionAttributeEnemy);
	/// 衝突対象は自分の属性以外に設定(ビット反転)
	SetCollisionMask(~kCollisionAttributeEnemy);

	// 設定したパターンによって初期状態を設定する
	SetInitializeState();
}

void Enemy::Update(const Matrix4x4& viewProjectionMatrix) {
	// 弾の削除
	DeleteBullets();

	// 状態更新
	if (state_) {
		state_->Update();
	}

	// 時限発動の更新
	UpdateTimedCalls();

	// 弾の更新
	for (auto& bullet : bullets_) {
		bullet->Update(viewProjectionMatrix);
	}

	// 向きの更新
	SetToVelocityDirection();

	// ゲームオブジェクトの更新
	gameObject_->Update(viewProjectionMatrix);
}

void Enemy::Draw(const Light& directionalLight) {
	// 敵本体の描画
	gameObject_->Draw(directionalLight);

	// 弾丸の描画
	for (auto& bullet : bullets_) {
		bullet->Draw(directionalLight);
	}
}

void Enemy::ImGui() {
#ifdef _DEBUG
	if (ImGui::CollapsingHeader("Enemy")) {
		if (gameObject_) {
			// 座標調整
			Vector3 pos = gameObject_->GetPosition();
			if (ImGui::DragFloat3("Enemy_Translate", &pos.x, 0.1f)) {
				gameObject_->SetPosition(pos);
			}
		}

		ImGui::Separator();

		// デバッグ情報を表示
		ImGui::Text("Auto Fire: %s", isAutoFire_ ? "ON" : "OFF");
		ImGui::Text("Timed Calls Count: %zu", timedCalls_.size());
		ImGui::Text("Bullets Count: %zu", bullets_.size());
		ImGui::Text("Pattern: %d", static_cast<int>(pattern_));
		ImGui::Text("Is Dead: %s", isDead_ ? "YES" : "NO");

		ImGui::Separator();

		// デバッグでステートの切り替え
		if (ImGui::Button("Change State Leave")) {
			ChangeState(std::make_unique<EnemyStateLeave>(this));
		}
		if (ImGui::Button("Change State Approach")) {
			ChangeState(std::make_unique<EnemyStateApproach>(this));
		}
	}
#endif
}

void Enemy::ChangeState(std::unique_ptr<BaseEnemyState> state) {
	// 引数で受け取った状態を次の状態としてセットする
	state_ = std::move(state);
}

void Enemy::Fire() {
	if (!player_) return;

	// 自機狙いの計算
	Vector3 playerWorldPos = player_->GetWorldPosition();
	Vector3 enemyWorldPos = GetWorldPosition();
	Vector3 enemyToPlayerVec = playerWorldPos - enemyWorldPos; // 差分ベクトルを取る
	enemyToPlayerVec = Normalize(enemyToPlayerVec); // 正規化して向きにする

	// 速度と合わせる
	Vector3 bulletVelocity = enemyToPlayerVec * kBulletSpeed;

	// 弾丸を生成・初期化する
	auto newBullet = std::make_unique<EnemyBullet>();
	newBullet->Initialize(directXCommon_, gameObject_->GetPosition(), bulletVelocity, kBulletSpeed);
	newBullet->SetPlayer(player_);

	// GameSceneがセットされている場合はGameSceneに弾丸を追加
	if (gameScene_) {
		gameScene_->AddEnemyBullet(std::move(newBullet));
	} else {
		// 弾丸を登録する（ローカルで管理）
		bullets_.push_back(std::move(newBullet));
	}
}

void Enemy::DeleteBullets() {
	// デスフラグが立っている弾を削除する
	bullets_.remove_if([](const std::unique_ptr<EnemyBullet>& bullet) {
		return bullet->IsDead();
		});
}

void Enemy::AutoFire() {
	// 自動発射をしない設定になっているなら早期リターン
	if (!isAutoFire_) {
		return;
	}

	// 弾を発射する
	Fire();

	// 発射タイマーをセットする
	timedCalls_.push_back(std::make_unique<TimedCall>(std::bind(&Enemy::AutoFire, this), kFireInterval_));
}

void Enemy::StartAutoFire() {
	// 自動発射をする設定になっているなら意味ないので早期リターン
	if (isAutoFire_) {
		return;
	}

	isAutoFire_ = true; // 自動発射フラグを立てる

	// 発射タイマーをセットする
	timedCalls_.push_back(std::make_unique<TimedCall>(std::bind(&Enemy::AutoFire, this), kFireInterval_));
}

void Enemy::UpdateTimedCalls() {
	// １フレ遅れさせてからクリアの実行
	if (shouldClearTimedCalls_) {
		timedCalls_.clear();
		isAutoFire_ = false;
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

void Enemy::ClearTimeCalls() {
	// 次のフレームでクリアする
	shouldClearTimedCalls_ = true;
}

Vector3 Enemy::GetWorldPosition() {
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

void Enemy::OnCollision() {
	isDead_ = true; // デスフラグを立てる
}

void Enemy::SetToVelocityDirection() {
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

void Enemy::SetInitializeState() {
	// パターンに応じた初期状態をセットする
	switch (pattern_) {
	case EnemyPattern::Straight:
	case EnemyPattern::LeaveLeft:
	case EnemyPattern::LeaveRight:
		// 接近→離脱パターン
		ChangeState(std::make_unique<EnemyStateApproach>(this));
		break;
	}
}