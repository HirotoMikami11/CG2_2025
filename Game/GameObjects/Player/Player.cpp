#define NOMINMAX
#include "Player.h"
#include "Managers/ImGui/ImGuiManager.h"
#include "GameObjects/LockOn/LockOn.h"
#include "Camera/RailCamera.h"

Player::Player() {
}

Player::~Player() {
	// unique_ptrで自動的に解放される
}

void Player::Initialize(DirectXCommon* dxCommon, const Vector3& position) {
	directXCommon_ = dxCommon;

	// 入力のシングルトンを取得
	input_ = InputManager::GetInstance();

	// ゲームオブジェクト（球体）の初期化
	gameObject_ = std::make_unique<Model3D>();
	gameObject_->Initialize(dxCommon, "player");
	gameObject_->SetName("Player");

	// 初期位置設定
	Vector3Transform defaultTransform{
		{1.0f, 1.0f, 1.0f},  // scale
		{0.0f, 0.0f, 0.0f},  // rotate
		position             // translate（引数で指定された位置）
	};
	gameObject_->SetTransform(defaultTransform);

	// 分離したシステムの初期化
	health_ = std::make_unique<PlayerHealth>();
	health_->Initialize();

	ui_ = std::make_unique<PlayerUI>();
	ui_->Initialize(dxCommon);

	reticle_ = std::make_unique<PlayerReticle>();
	reticle_->Initialize(dxCommon);

	// ビュープロジェクション行列を単位行列で初期化
	viewProjectionMatrixSprite_ = MakeIdentity4x4();

	// 衝突判定設定
	SetRadius(1.0f); // Colliderの半径をセット
	// スケールをコライダーの半径に合わせる
	gameObject_->SetScale(Vector3(GetRadius(), GetRadius(), GetRadius()));

	/// 衝突属性の設定
	SetCollisionAttribute(kCollisionAttributePlayer);
	/// 衝突対象は自分の属性以外に設定(ビット反転)
	SetCollisionMask(~kCollisionAttributePlayer);
}

void Player::Update(const Matrix4x4& viewProjectionMatrix) {
	// 弾の削除
	DeleteBullets();
	DeleteHomingBullets();

	// 移動処理
	Move();

	// 自機をカメラに背を向ける処理
	FaceAwayFromCamera();

	//攻撃方法の変更(一時的に封印)
	SwitchAttackMode();

	// 攻撃処理
	Attack();

	// 分離したシステムの更新
	health_->Update();
	ui_->Update(*health_, viewProjectionMatrixSprite_);
	reticle_->Update(viewProjectionMatrix, viewProjectionMatrixSprite_);

	// 弾の更新
	for (auto& bullet : bullets_) {
		bullet->Update(viewProjectionMatrix);
	}

	// ホーミング弾の更新
	for (auto& homingBullet : homingBullets_) {
		homingBullet->Update(viewProjectionMatrix);
	}

	// ゲームオブジェクトの更新
	gameObject_->Update(viewProjectionMatrix);
}

void Player::Draw(const Light& directionalLight) {
	// プレイヤー本体の描画
	gameObject_->Draw(directionalLight);

	// 3Dレティクルの描画
	reticle_->Draw3D(directionalLight);

	// 弾の描画
	for (auto& bullet : bullets_) {
		bullet->Draw(directionalLight);
	}

	// ホーミング弾の描画
	for (auto& homingBullet : homingBullets_) {
		homingBullet->Draw(directionalLight);
	}
}

void Player::DrawUI() {
	// UIシステムによるゲージ描画
	ui_->Draw();

	// マルチロックオンモードか
	// 通常モードで、ロックオン対象がいなければ2Dレティクルの描画
	if (attackMode_ == AttackMode::MultiLockOn ||
		(lockOn_ == nullptr || lockOn_->GetTarget() == nullptr)) {
		// 通常モードでロックオン対象がいない場合は2Dレティクルを描画
		reticle_->Draw2D();
	}
}

void Player::ImGui() {
#ifdef _DEBUG
	// 現在の名前を表示
	if (ImGui::TreeNode("Player")) {
		if (gameObject_) {
			gameObject_->ImGui();
		}

		// 分離したシステムのImGui
		health_->ImGui();
		ui_->ImGui();
		reticle_->ImGui();

		/// 攻撃モード情報
		ImGui::Separator();
		ImGui::Text("Attack Mode");
		const char* modeText = "";
		switch (attackMode_) {
		case AttackMode::Normal:
			modeText = "Normal";
			break;
		case AttackMode::MultiLockOn:
			modeText = "Multi Lock-On";
			break;
		}
		ImGui::Text("Current Mode: %s", modeText);
		ImGui::Text("Multi Lock-On Targets: %zu", multiLockOnTargets_.size());
		ImGui::Separator();
		ImGui::Text("Bullets Count: %zu", bullets_.size());

		// KamataEngineのデバッグ表示と同様の情報を表示
		Vector3 worldPos = GetWorldPosition();
		ImGui::Text("World Position: (%.2f, %.2f, %.2f)", worldPos.x, worldPos.y, worldPos.z);

		Vector3 localPos = gameObject_->GetPosition();
		ImGui::Text("Local Position: (%.2f, %.2f, %.2f)", localPos.x, localPos.y, localPos.z);

		// 前方向ベクトルのデバッグ表示
		Vector3 forward = GetForward();
		ImGui::Text("Forward Vector: (%.2f, %.2f, %.2f)", forward.x, forward.y, forward.z);

		ImGui::TreePop();
	}
#endif
}

Vector3 Player::GetWorldPosition() {
	if (gameObject_) {
		// Transform3Dが親子関係を考慮したワールド行列を返してくれる
		Matrix4x4 worldMatrix = gameObject_->GetTransform().GetWorldMatrix();
		return Vector3{
			worldMatrix.m[3][0],
			worldMatrix.m[3][1],
			worldMatrix.m[3][2]
		};
	}
	return Vector3{ 0.0f, 0.0f, 0.0f };
}

Vector3 Player::GetWorldPosition3DReticle() {
	return reticle_->GetWorldPosition3DReticle();
}

Vector3 Player::GetForward() const {
	if (!gameObject_) {
		return Vector3{ 0.0f, 0.0f, 1.0f }; // デフォルトの前方向
	}

	// プレイヤーの回転を取得
	Vector3 rotation = gameObject_->GetRotation();

	// Y軸回転から前方向ベクトルを計算
	// プレイヤーはカメラに背を向けているので、Z軸正方向が前方
	Vector3 forward;
	forward.x = std::sin(rotation.y);
	forward.y = 0.0f; // 水平方向のみ考慮
	forward.z = std::cos(rotation.y);

	return Normalize(forward);
}

void Player::OnCollision() {
	// ダメージを受ける(体力システムに渡す)
	health_->TakeDamage(1.0f);
}

void Player::Move() {
	// 移動ベクトル
	Vector3 move = { 0.0f, 0.0f, 0.0f };

	// ゲームパッド入力の処理
	if (input_->IsGamePadConnected()) {
		move.x += input_->GetAnalogStick(InputManager::AnalogStick::LEFT_X) * kCharacterSpeed;
		move.y += input_->GetAnalogStick(InputManager::AnalogStick::LEFT_Y) * kCharacterSpeed;
	}

	// キーボード入力の処理（WASD: プレイヤーの移動）
	if (input_->IsKeyDown(DIK_A)) {
		move.x -= kCharacterSpeed; // 左
	}
	if (input_->IsKeyDown(DIK_D)) {
		move.x += kCharacterSpeed; // 右
	}
	if (input_->IsKeyDown(DIK_W)) {
		move.y += kCharacterSpeed; // 上
	}
	if (input_->IsKeyDown(DIK_S)) {
		move.y -= kCharacterSpeed; // 下
	}

	// 現在位置に移動量を加算
	Vector3 currentPos = gameObject_->GetPosition();
	currentPos += move;

	// 移動制限の適用（KamataEngineと同じ値）
	currentPos.x = std::clamp(currentPos.x, -kMoveLimitX, kMoveLimitX);
	currentPos.y = std::clamp(currentPos.y, -kMoveLimitY, kMoveLimitY);

	// 位置を設定
	gameObject_->SetPosition(currentPos);
}

void Player::FaceAwayFromCamera() {
	// 親（レールカメラ）が設定されている場合は、ローカル座標系で固定の向きに設定
	if (gameObject_->GetTransform().GetParent() != nullptr) {
		// レールカメラに対して常に向こう側を向く
		// これにより、レールカメラがどの方向を向いていても、プレイヤーは常にカメラに背を向ける
		Vector3 fixedRotation = { 0.0f, 0.0f, 0.0f };
		gameObject_->SetRotation(fixedRotation);
		return;
	}

	// 親がない場合は従来の処理を実行
	CameraController* cameraController = CameraController::GetInstance();
	if (!cameraController) {
		return;
	}

	// カメラの位置を取得
	Vector3 cameraPosition = cameraController->GetPosition();
	Vector3 playerPosition = GetWorldPosition();

	// カメラから自機への方向ベクトルを計算
	Vector3 cameraToPlayer = playerPosition - cameraPosition;
	cameraToPlayer = Normalize(cameraToPlayer);

	// Y軸回転角を計算（カメラに背を向ける）
	float rotationY = std::atan2(cameraToPlayer.x, cameraToPlayer.z);

	// プレイヤーの回転を設定
	Vector3 currentRotation = gameObject_->GetRotation();
	currentRotation.y = rotationY;
	gameObject_->SetRotation(currentRotation);
}

void Player::SwitchAttackMode() {
	// モード切り替え (MキーまたはXボタン)
	if (input_->IsKeyTrigger(DIK_M) ||// Mキー
		(input_->IsGamePadConnected() && input_->IsGamePadButtonTrigger(InputManager::GamePadButton::X))) { // Xボタン

		// モード切り替え
		switch (attackMode_) {
		case AttackMode::Normal:
			attackMode_ = AttackMode::MultiLockOn;
			break;
		case AttackMode::MultiLockOn:
			attackMode_ = AttackMode::Normal;
			// 通常モードに戻る時はマルチロックオンリストをクリア
			multiLockOnTargets_.clear();
			break;
		}
	}
}

void Player::Attack() {
	// SPACEキーまたはゲームパッドRBボタンで発射
	bool shouldFire = input_->IsKeyDown(DIK_SPACE) ||
		(input_->IsGamePadConnected() && input_->IsGamePadButtonDown(InputManager::GamePadButton::RB));

	if (shouldFire) {
		fireTimer_--;

		if (fireTimer_ <= 0) {
			switch (attackMode_) {
			case AttackMode::MultiLockOn:
				FireMultiLockOn(); // マルチロックオンモード時の発射
				break;
			case AttackMode::Normal:
			default:
				Fire(); // 通常モード時の発射
				break;
			}
			fireTimer_ = kFireInterval; // 発射間隔をリセット
		}
	}
}

void Player::Fire() {
	// エネルギーが足りない場合は発射できない
	if (!health_->HasEnoughEnergy(health_->GetEnergyCostPerShot())) {
		return;
	}

	// 通常モードでは偏差射撃を使用
	if (lockOn_ && lockOn_->GetTarget() != nullptr && !lockOn_->GetTarget()->IsDead()) {
		// ロックオン対象の敵を取得
		BaseEnemy* target = lockOn_->GetTarget();

		// 敵の速度を取得
		Vector3 enemyVelocity = target->GetVelocity();

		// 偏差射撃で予測位置を計算
		Vector3 predictedPosition = CalculateLeadingShot(
			target->GetWorldPosition(),
			enemyVelocity,
			kBulletSpeed
		);

		// プレイヤーから予測位置へのベクトル
		Vector3 bulletVelocity = predictedPosition - GetWorldPosition();
		// ベクトルの長さを整える
		bulletVelocity = Normalize(bulletVelocity) * kBulletSpeed;

		// 通常弾を生成・初期化する
		auto newBullet = std::make_unique<PlayerBullet>();
		newBullet->Initialize(directXCommon_, GetWorldPosition(), bulletVelocity);

		// 弾を登録する
		bullets_.push_back(std::move(newBullet));

	} else {
		// ターゲットが存在しない場合は、通常弾を発射（レティクル狙い弾）
		// 自機からレティクルへのベクトルを計算
		Vector3 playerPos = GetWorldPosition();
		Vector3 reticlePos = GetWorldPosition3DReticle();
		Vector3 bulletVelocity = reticlePos - playerPos;

		// 正規化して速度を設定
		bulletVelocity = Normalize(bulletVelocity) * kBulletSpeed;

		// 弾丸を生成・初期化する
		auto newBullet = std::make_unique<PlayerBullet>();
		newBullet->Initialize(directXCommon_, playerPos, bulletVelocity);

		// 弾丸を登録する
		bullets_.push_back(std::move(newBullet));

	}
	// エネルギーを消費(体力システムに渡す)
	health_->ConsumeEnergy(health_->GetEnergyCostPerShot());

}

void Player::FireMultiLockOn() {
	// マルチロックオンモードでロックオン対象がいない場合は発射しない
	if (multiLockOnTargets_.empty()) {
		return;
	}

	// 必要なエネルギーを計算（ターゲット数 × 消費量）
	float requiredEnergy = multiLockOnTargets_.size() * health_->GetEnergyCostPerShot();
	if (!health_->HasEnoughEnergy(requiredEnergy)) {
		return; // エネルギーが足りない
	}

	// ロックオンしている全ての敵に向けてホーミング弾を発射
	for (BaseEnemy* target : multiLockOnTargets_) {
		if (target != nullptr && !target->IsDead()) {
			// ターゲットのワールド座標を取得
			Vector3 targetPosition = target->GetWorldPosition();
			// 自機からターゲットへのベクトル
			Vector3 bulletVelocity = targetPosition - GetWorldPosition();
			// ベクトルの長さを整える
			bulletVelocity = Normalize(bulletVelocity) * kBulletSpeed;

			// ホーミング弾を生成・初期化する
			auto newHomingBullet = std::make_unique<PlayerHomingBullet>();
			newHomingBullet->Initialize(directXCommon_, GetWorldPosition(), bulletVelocity, target);

			// ホーミング弾を登録する(moveで所有権を渡す)
			homingBullets_.push_back(std::move(newHomingBullet));
		}
	}

	// エネルギーを消費(体力システムに渡す)
	health_->ConsumeEnergy(requiredEnergy * 3.0f);
}

void Player::DeleteBullets() {
	// デスフラグが立っている弾を削除する
	bullets_.remove_if([](const std::unique_ptr<PlayerBullet>& bullet) {
		return bullet->IsDead();
		});
}

void Player::DeleteHomingBullets()
{
	// デスフラグが立っているホーミング弾を削除する
	homingBullets_.remove_if([](const std::unique_ptr<PlayerHomingBullet>& homingBullet) {
		return homingBullet->IsDead();
		});
}

Vector3 Player::CalculateLeadingShot(const Vector3& enemyPos, const Vector3& enemyVelocity, float bulletSpeed) {
	// プレイヤーの位置
	Vector3 playerPos = GetWorldPosition();

	// プレイヤーから敵への相対位置
	Vector3 relativePos = enemyPos - playerPos;

	// 二次方程式の係数を計算
	// at^2 + bt + c = 0
	float a = Dot(enemyVelocity, enemyVelocity) - bulletSpeed * bulletSpeed;
	float b = 2.0f * Dot(enemyVelocity, relativePos);
	float c = Dot(relativePos, relativePos);

	// 判別式
	float discriminant = b * b - 4 * a * c;

	// 解が存在しない場合は現在の敵位置を返す
	if (discriminant < 0) {
		return enemyPos;
	}

	// 二次方程式を解く
	float sqrtDiscriminant = std::sqrt(discriminant);
	float t1 = (-b - sqrtDiscriminant) / (2 * a);
	float t2 = (-b + sqrtDiscriminant) / (2 * a);

	// 正の最小時間を選択
	float t = 0;
	if (t1 > 0 && t2 > 0) {
		t = std::min(t1, t2);
	} else if (t1 > 0) {
		t = t1;
	} else if (t2 > 0) {
		t = t2;
	} else {
		// 両方負の場合は現在の敵位置を返す
		return enemyPos;
	}

	// 予測位置を計算
	return enemyPos + enemyVelocity * t;
}