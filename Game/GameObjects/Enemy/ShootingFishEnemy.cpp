#include "ShootingFishEnemy.h"
#include "GameObjects/Player/Player.h"
#include "Managers/ImGui/ImGuiManager.h"
#include "Scenes/GameScene/GameScene.h"

ShootingFishEnemy::ShootingFishEnemy() : BaseEnemy(EnemyType::ShootingFish) {
}

ShootingFishEnemy::~ShootingFishEnemy() {
	// 基底クラスのデストラクタが呼ばれる
}

void ShootingFishEnemy::Initialize(DirectXCommon* dxCommon, const Vector3& position, EnemyPattern pattern) {
	directXCommon_ = dxCommon;

	// ゲームオブジェクト（球体）の初期化
	gameObject_ = std::make_unique<Model3D>();
	gameObject_->Initialize(dxCommon, "shootingFish");
	gameObject_->SetName("ShootingFishEnemy");

	// 初期位置設定
	gameObject_->SetPosition(position);

	// 大きさを設定（通常の敵と同程度）
	Vector3Transform enemyTransform{
		{5.0f, 5.0f, 5.0f},   // scale
		{0.0f, 0.0f, 0.0f},   // rotate
		position              // translate
	};
	gameObject_->SetTransform(enemyTransform);

	// 初期速度
	velocity_ = { 0, 0, 0 };

	// パターンの設定
	pattern_ = pattern;

	// HP設定（射撃魚はHP7）
	maxHP_ = 15.0f;
	currentHP_ = maxHP_;

	// 衝突判定設定
	SetRadius(2.5f); // Colliderの半径をセット
	/// 衝突属性の設定
	SetCollisionAttribute(kCollisionAttributeEnemy);
	/// 衝突対象は自分の属性以外に設定(ビット反転)
	SetCollisionMask(~kCollisionAttributeEnemy);

	// 設定したパターンによって初期状態を設定する
	SetInitializeState();
}

void ShootingFishEnemy::Update(const Matrix4x4& viewProjectionMatrix) {
	// 状態更新
	if (state_) {
		state_->Update();
	}

	// 時限発動の更新
	UpdateTimedCalls();

	// 向きの更新（射撃状態では手動で制御するのでスキップする場合もある）
	if (state_ && state_->GetName() != "Shoting Fish Shoot") {
		SetToVelocityDirection();
	}

	// ゲームオブジェクトの更新
	gameObject_->Update(viewProjectionMatrix);
}

void ShootingFishEnemy::Draw(const Light& directionalLight) {
	// 敵本体の描画
	gameObject_->Draw(directionalLight);
}

void ShootingFishEnemy::ImGui() {
#ifdef _DEBUG
	if (ImGui::CollapsingHeader("Shoting Fish Enemy")) {
		if (gameObject_) {
			// 座標調整
			Vector3 pos = gameObject_->GetPosition();
			if (ImGui::DragFloat3("ShotingFish_Translate", &pos.x, 0.1f)) {
				gameObject_->SetPosition(pos);
			}

			// 回転調整
			Vector3 rot = gameObject_->GetRotation();
			if (ImGui::DragFloat3("ShotingFish_Rotation", &rot.x, 0.01f)) {
				gameObject_->SetRotation(rot);
			}
		}

		ImGui::Separator();

		// HP情報
		ImGui::Text("HP: %.1f / %.1f", currentHP_, maxHP_);

		// プレイヤーとの距離表示
		if (player_) {
			Vector3 playerPos = player_->GetWorldPosition();
			Vector3 enemyPos = GetWorldPosition();
			float distance = Distance(playerPos, enemyPos);
			ImGui::Text("Distance to Player: %.2f", distance);
		}

		// デバッグ情報を表示
		ImGui::Text("Velocity: (%.2f, %.2f, %.2f)", velocity_.x, velocity_.y, velocity_.z);
		ImGui::Text("Pattern: %d", static_cast<int>(pattern_));
		ImGui::Text("Is Dead: %s", isDead_ ? "YES" : "NO");

		// 現在の状態名を表示
		if (state_) {
			ImGui::Text("Current State: %s", state_->GetName().c_str());
		}

		ImGui::Separator();

		// デバッグでステートの切り替え
		if (ImGui::Button("Change State Approach")) {
			ChangeState(std::make_unique<FishStateApproach>(this));
		}
		if (ImGui::Button("Change State Hover")) {
			ChangeState(std::make_unique<FishStateHover>(this));
		}
		if (ImGui::Button("Change State Shoot")) {
			ChangeState(std::make_unique<FishStateShoot>(this));
		}
		if (ImGui::Button("Change State Escape")) {
			ChangeState(std::make_unique<FishStateEscape>(this));
		}
	}
#endif
}

void ShootingFishEnemy::OnCollision() {
	// プレイヤーの弾に当たった場合はダメージを受ける
	TakeDamage(1.0f); // 1ダメージ
}

void ShootingFishEnemy::Fire()
{
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

void ShootingFishEnemy::SetInitializeState() {
	// パターンに応じた初期状態をセットする
	switch (pattern_) {
	case EnemyPattern::Shooting:
	default:
		// 接近状態で開始
		ChangeState(std::make_unique<FishStateApproach>(this));
		break;
	}
}