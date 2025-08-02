#include "RushingFishEnemy.h"
#include "GameObjects/Player/Player.h"
#include "Managers/ImGui/ImGuiManager.h"
#include "Scenes/GameScene/GameScene.h"

RushingFishEnemy::RushingFishEnemy() : BaseEnemy(EnemyType::RushingFish) {
}

RushingFishEnemy::~RushingFishEnemy() {
	// 基底クラスのデストラクタが呼ばれる
}

void RushingFishEnemy::Initialize(DirectXCommon* dxCommon, const Vector3& position, EnemyPattern pattern) {
	directXCommon_ = dxCommon;

	// ゲームオブジェクト（球体）の初期化
	gameObject_ = std::make_unique<Model3D>();
	gameObject_->Initialize(dxCommon, "RushFish");
	gameObject_->SetName("RushingFishEnemy");

	// 初期位置設定
	gameObject_->SetPosition(position);

	// 大きさを設定（通常の敵より少し小さく）
	Vector3Transform enemyTransform{
		{5.0f, 5.0f, 5.0f},   // scale（少し小さめ）
		{0.0f, 0.0f, 0.0f},   // rotate
		position              // translate
	};
	gameObject_->SetTransform(enemyTransform);

	// 初期速度（プレイヤーの方向は後で設定）
	velocity_ = { 0, 0, 0 };

	// パターンの設定
	pattern_ = pattern;

	// HP設定（突進魚はHP10）
	maxHP_ = 10.0f;
	currentHP_ = maxHP_;

	// 衝突判定設定
	SetRadius(4.0f); // Colliderの半径をセット（通常より小さく）
	/// 衝突属性の設定
	SetCollisionAttribute(kCollisionAttributeEnemy);
	/// 衝突対象は自分の属性以外に設定(ビット反転)
	SetCollisionMask(~kCollisionAttributeEnemy);

	// 設定したパターンによって初期状態を設定する
	SetInitializeState();
}

void RushingFishEnemy::Update(const Matrix4x4& viewProjectionMatrix) {
	// 状態更新
	if (state_) {
		state_->Update();
	}

	// 時限発動の更新
	UpdateTimedCalls();

	// 向きの更新
	SetToVelocityDirection();

	// ゲームオブジェクトの更新
	gameObject_->Update(viewProjectionMatrix);
}

void RushingFishEnemy::Draw(const Light& directionalLight) {
	// 敵本体の描画
	gameObject_->Draw(directionalLight);
}

void RushingFishEnemy::ImGui() {
#ifdef _DEBUG
	if (ImGui::CollapsingHeader("Rushing Fish Enemy")) {
		if (gameObject_) {
			// 座標調整
			Vector3 pos = gameObject_->GetPosition();
			if (ImGui::DragFloat3("RushingFish_Translate", &pos.x, 0.1f)) {
				gameObject_->SetPosition(pos);
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

		ImGui::Separator();

		// デバッグでステートの切り替え
		if (ImGui::Button("Change State Homing")) {
			ChangeState(std::make_unique<RushingFishStateHoming>(this));
		}
		if (ImGui::Button("Change State Rush (Current Velocity)")) {
			ChangeState(std::make_unique<RushingFishStateRush>(this, velocity_));
		}
	}
#endif
}

void RushingFishEnemy::OnCollision() {
	// プレイヤーと衝突した場合
	if (player_) {
		// プレイヤーにダメージを与える
		player_->TakeDamage(kPlayerDamage);
	}

	// 自分は死亡
	isDead_ = true;
}

void RushingFishEnemy::SetInitializeState() {
	// パターンに応じた初期状態をセットする
	switch (pattern_) {
	case EnemyPattern::Homing:
	default:
		// ホーミング状態で開始
		ChangeState(std::make_unique<RushingFishStateHoming>(this));
		break;
	}
}