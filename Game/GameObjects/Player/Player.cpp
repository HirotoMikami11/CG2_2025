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

	// 3Dレティクルの初期化
	reticle3D_ = std::make_unique<Model3D>();
	reticle3D_->Initialize(dxCommon, "player"); // 仮でplayerモデルを使用
	reticle3D_->SetName("Reticle3D");

	Vector3Transform reticleTransform{
		{2.0f, 2.0f, 2.0f},           // scale（わかりやすくデカく）
		{0.0f, 0.0f, 0.0f},           // rotate
		{0.0f, 0.0f, 50.0f}           // translate（プレイヤーの前方）
	};
	reticle3D_->SetTransform(reticleTransform);

	// 2Dレティクル用のスプライト初期化
	sprite2DReticle_ = std::make_unique<Sprite>();
	sprite2DReticle_->Initialize(
		dxCommon,
		"reticle",                    // テクスチャ名
		{ 640.0f, 360.0f },             // 画面中央
		{ 64.0f, 64.0f }                // サイズ
	);

	// ビューポート行列の計算(画面サイズが変わったら変更)
	matViewport_ = MakeViewportMatrix(0, 0, GraphicsConfig::kClientWidth, GraphicsConfig::kClientHeight, 0, 1);

	// スプライト用ビュープロジェクション行列を単位行列で初期化
	viewProjectionMatrixSprite_ = MakeIdentity4x4();

	// HP/ENゲージの初期化
	InitializeGauges();

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

	// 回転処理（レールカメラ対応）
	RotateWithRailCamera();

	//攻撃方法の変更
	SwitchAttackMode();

	// 攻撃処理
	Attack();

	// エネルギーの更新
	UpdateEnergy();

	// HP/ENゲージの更新
	UpdateGauges();

	// レティクルの更新
	UpdateReticle(viewProjectionMatrix);

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
	reticle3D_->Update(viewProjectionMatrix);

}

void Player::Draw(const Light& directionalLight) {
	// プレイヤー本体の描画
	gameObject_->Draw(directionalLight);

	// 3Dレティクルの描画
	reticle3D_->Draw(directionalLight);

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
	// HP/ENゲージの描画
	if (hpGaugeBar_) {
		hpGaugeBar_->Update(viewProjectionMatrixSprite_);
		hpGaugeBar_->Draw();
	}
	if (hpGaugeFill_) {
		hpGaugeFill_->Update(viewProjectionMatrixSprite_);
		hpGaugeFill_->Draw();
	}
	if (enGaugeBar_) {
		enGaugeBar_->Update(viewProjectionMatrixSprite_);
		enGaugeBar_->Draw();
	}
	if (enGaugeFill_) {
		enGaugeFill_->Update(viewProjectionMatrixSprite_);
		enGaugeFill_->Draw();
	}

	// マルチロックオンモードか
	// 通常モードで、ロックオン対象がいなければ2Dレティクルの描画
	if (
		attackMode_ == AttackMode::MultiLockOn ||
		(lockOn_ == nullptr || lockOn_->GetTarget() == nullptr)) {
		// 通常モードでロックオン対象がいない場合は2Dレティクルを描画
		sprite2DReticle_->Update(viewProjectionMatrixSprite_);
		sprite2DReticle_->Draw();
	}
}

void Player::ImGui() {
#ifdef _DEBUG
	// 現在の名前を表示
	if (ImGui::TreeNode("Player")) {
		if (gameObject_) {
			gameObject_->ImGui();
		}

		// HP/EN情報
		ImGui::Separator();
		ImGui::Text("HP/EN System");
		ImGui::Text("HP: %.1f / %.1f", currentHP_, maxHP_);
		ImGui::ProgressBar(currentHP_ / maxHP_, ImVec2(200, 20), "HP");
		ImGui::Text("EN: %.1f / %.1f", currentEN_, maxEN_);
		ImGui::ProgressBar(currentEN_ / maxEN_, ImVec2(200, 20), "EN");
		ImGui::Text("Energy Regen Timer: %d", energyRegenTimer_);

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

		// レティクル情報
		ImGui::Separator();
		ImGui::Text("Reticle Info:");
		Vector3 reticlePos = GetWorldPosition3DReticle();
		ImGui::Text("3D Reticle Position: (%.2f, %.2f, %.2f)", reticlePos.x, reticlePos.y, reticlePos.z);

		Vector2 spritePos = sprite2DReticle_->GetPosition();
		ImGui::Text("2D Reticle Position: (%.2f, %.2f)", spritePos.x, spritePos.y);

		// KamataEngineのデバッグ表示と同様の情報を表示
		Vector3 worldPos = GetWorldPosition();
		ImGui::Text("World Position: (%.2f, %.2f, %.2f)", worldPos.x, worldPos.y, worldPos.z);

		Vector3 localPos = gameObject_->GetPosition();
		ImGui::Text("Local Position: (%.2f, %.2f, %.2f)", localPos.x, localPos.y, localPos.z);
		ImGui::TreePop();
	}
#endif
}

Vector3 Player::GetWorldPosition() {
	if (gameObject_) {
		// Transform3Dが親子関係を考慮したワールド行列を返してくれる（KamataEngineと同じ）
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
	if (reticle3D_) {
		Matrix4x4 worldMatrix = reticle3D_->GetTransform().GetWorldMatrix();
		return Vector3{
			worldMatrix.m[3][0],
			worldMatrix.m[3][1],
			worldMatrix.m[3][2]
		};
	}
	return Vector3{ 0.0f, 0.0f, 0.0f };
}

void Player::OnCollision() {
	// ダメージを受ける（仮のダメージ量1）
	TakeDamage(1.0f);
}

void Player::Move() {
	// 移動ベクトル
	Vector3 move = { 0.0f, 0.0f, 0.0f };

	// ゲームパッド入力の処理
	if (input_->IsGamePadConnected()) {
		move.x += input_->GetAnalogStick(InputManager::AnalogStick::LEFT_X) * kCharacterSpeed;
		move.y += input_->GetAnalogStick(InputManager::AnalogStick::LEFT_Y) * kCharacterSpeed;
	}

	// キーボード入力の処理
	if (input_->IsKeyDown(DIK_LEFT)) {
		move.x -= kCharacterSpeed; // 左
	} else if (input_->IsKeyDown(DIK_RIGHT)) {
		move.x += kCharacterSpeed; // 右
	}

	if (input_->IsKeyDown(DIK_UP)) {
		move.y += kCharacterSpeed; // 上
	} else if (input_->IsKeyDown(DIK_DOWN)) {
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

void Player::Rotate() {
	// 現在の回転を取得
	Vector3 currentRotation = gameObject_->GetRotation();

	// 押した方向で回転（KamataEngineと同じキー割り当て）
	if (input_->IsKeyDown(DIK_A)) {
		currentRotation.y += kRotSpeed;
	} else if (input_->IsKeyDown(DIK_D)) {
		currentRotation.y -= kRotSpeed;
	}

	// 回転を設定
	gameObject_->SetRotation(currentRotation);
}

void Player::RotateWithRailCamera() {
	// レールカメラを使用している場合は、カメラの進行方向に合わせて回転
	CameraController* cameraController = CameraController::GetInstance();
	if (cameraController && cameraController->GetActiveCameraId() == "rail") {
		RailCamera* railCamera = dynamic_cast<RailCamera*>(cameraController->GetCamera("rail"));
		if (railCamera) {
			// レールカメラの進行方向を取得
			Vector3 forwardDirection = railCamera->GetForwardDirection();

			// 進行方向からY軸回転角を計算
			float targetRotationY = std::atan2(forwardDirection.x, forwardDirection.z);

			// プレイヤーの回転を設定
			Vector3 currentRotation = gameObject_->GetRotation();
			currentRotation.y = targetRotationY;
			gameObject_->SetRotation(currentRotation);

			return; // レールカメラ使用時は通常の回転処理をスキップ
		}
	}

	// レールカメラを使用していない場合は通常の回転処理
	Rotate();
}

void Player::SwitchAttackMode() {
	// モード切り替え (MキーまたはXボタン)
	if (input_->IsKeyTrigger(DIK_M) ||                                      // Mキー
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
	if (currentEN_ < energyCostPerShot_) {
		return;
	}

	// 通常モードでは偏差射撃を使用
	if (lockOn_ && lockOn_->GetTarget() != nullptr && !lockOn_->GetTarget()->IsDead()) {
		// ロックオン対象の敵を取得
		Enemy* target = lockOn_->GetTarget();

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

	// エネルギーを消費
	currentEN_ -= energyCostPerShot_;
	currentEN_ = std::max(0.0f, currentEN_); // 0以下にならないように

	// エネルギー回復タイマーをリセット
	energyRegenTimer_ = energyRegenDelay_;
}

void Player::FireMultiLockOn() {
	// マルチロックオンモードでロックオン対象がいない場合は発射しない
	if (multiLockOnTargets_.empty()) {
		return;
	}

	// 必要なエネルギーを計算（ターゲット数 × 消費量）
	float requiredEnergy = multiLockOnTargets_.size() * energyCostPerShot_;
	if (currentEN_ < requiredEnergy) {
		return; // エネルギーが足りない
	}

	// ロックオンしている全ての敵に向けてホーミング弾を発射
	for (Enemy* target : multiLockOnTargets_) {
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

	// エネルギーを消費
	currentEN_ -= requiredEnergy;
	currentEN_ = std::max(0.0f, currentEN_); // 0以下にならないように

	// エネルギー回復タイマーをリセット
	energyRegenTimer_ = energyRegenDelay_;
}

void Player::DeleteBullets() {
	// デスフラグが立っている弾を削除する
	bullets_.remove_if([](const std::unique_ptr<PlayerBullet>& bullet) {
		return bullet->IsDead();
		});
}

void Player::UpdateReticle(const Matrix4x4& viewProjectionMatrix) {

	if (input_->IsGamePadConnected()) {
		// ゲームパッドが接続されている場合：ゲームパッドでレティクル操作
		ConvertGamePadToWorldReticle(viewProjectionMatrix);
	} else {
		// キーボードの場合：プレイヤーの向きに基づいてレティクル操作
		ConvertKeyboardToWorldReticle(viewProjectionMatrix);
	}
}

void Player::ConvertGamePadToWorldReticle(const Matrix4x4& viewProjectionMatrix) {
	// 現在のスプライト位置を取得
	Vector2 spritePos = sprite2DReticle_->GetPosition();

	// 右スティックでレティクル移動
	float stickX = input_->GetAnalogStick(InputManager::AnalogStick::RIGHT_X);
	float stickY = input_->GetAnalogStick(InputManager::AnalogStick::RIGHT_Y);

	// スティック入力をスプライト移動量に変換
	spritePos.x += stickX * 10.0f; // 感度調整
	spritePos.y -= stickY * 10.0f; // Y軸反転

	// 画面外に出ないように制限
	spritePos.x = std::clamp(spritePos.x, 0.0f, 1280.0f);
	spritePos.y = std::clamp(spritePos.y, 0.0f, 720.0f);

	// スプライト位置を更新
	sprite2DReticle_->SetPosition(spritePos);

	// 3D座標に変換
	Matrix4x4 matVPV = Matrix4x4Multiply(viewProjectionMatrix, matViewport_);
	Matrix4x4 matInverseVPV = Matrix4x4Inverse(matVPV);

	// スクリーンからワールドに変換
	posNear_ = Vector3(spritePos.x, spritePos.y, 0.0f);
	posFar_ = Vector3(spritePos.x, spritePos.y, 1.0f);

	posNear_ = Matrix4x4Transform(posNear_, matInverseVPV);
	posFar_ = Matrix4x4Transform(posFar_, matInverseVPV);

	Vector3 direction = Normalize(posFar_ - posNear_);
	spritePosition_ = posNear_ + (direction * kDistancePlayerTo3DReticleGamepad);
	reticle3D_->SetPosition(spritePosition_);
}

void Player::ConvertKeyboardToWorldReticle(const Matrix4x4& viewProjectionMatrix) {
	// キーボード操作：プレイヤーの向きに基づいて3Dレティクルを配置

	// レールカメラ使用時は進行方向を使用、それ以外は従来通り
	Vector3 forwardDirection = { 0.0f, 0.0f, 1.0f }; // デフォルト方向

	CameraController* cameraController = CameraController::GetInstance();
	if (cameraController && cameraController->GetActiveCameraId() == "rail") {
		RailCamera* railCamera = dynamic_cast<RailCamera*>(cameraController->GetCamera("rail"));
		if (railCamera) {
			forwardDirection = railCamera->GetForwardDirection();
		}
	} else {
		// 従来の方法：自機のワールド行列の回転を反映
		Matrix4x4 worldMatrix = gameObject_->GetTransform().GetWorldMatrix();
		forwardDirection = Matrix4x4TransformNormal(forwardDirection, worldMatrix);
	}

	// ベクトルの長さを整える
	forwardDirection = Normalize(forwardDirection) * kDistancePlayerTo3DReticleKeyborad;

	// 3Dレティクルの座標を設定
	Vector3 reticlePosition = GetWorldPosition() + forwardDirection;
	reticle3D_->SetPosition(reticlePosition);

	// 3Dレティクルの位置を2Dスプライトに反映
	ConvertWorldToScreenReticle(viewProjectionMatrix);
}

void Player::ConvertWorldToScreenReticle(const Matrix4x4& viewProjectionMatrix) {
	// 3Dレティクルの位置を取得
	Vector3 positionReticle = GetWorldPosition3DReticle();

	// ビュープロジェクション行列とビューポート行列を合成
	Matrix4x4 matViewProjectionViewport = Matrix4x4Multiply(viewProjectionMatrix, matViewport_);

	// ワールド座標からスクリーン座標に変換（3Dから2Dになる）
	positionReticle = Matrix4x4Transform(positionReticle, matViewProjectionViewport);

	// 2Dレティクルスプライトに座標設定
	sprite2DReticle_->SetPosition(Vector2(positionReticle.x, positionReticle.y));
}

void Player::DeleteHomingBullets()
{
	// デスフラグが立っているホーミング弾を削除する
	homingBullets_.remove_if([](const std::unique_ptr<PlayerHomingBullet>& homingBullet) {
		return homingBullet->IsDead();
		});
}

void Player::TakeDamage(float damage) {
	currentHP_ -= damage;
	currentHP_ = std::max(0.0f, currentHP_); // 0以下にならないように
}

void Player::UpdateEnergy() {
	// エネルギー回復タイマーを更新
	if (energyRegenTimer_ > 0) {
		energyRegenTimer_--;
	}

	// タイマーが0になったらエネルギーを回復
	if (energyRegenTimer_ <= 0 && currentEN_ < maxEN_) {
		currentEN_ += energyRegenRate_;
		currentEN_ = std::min(currentEN_, maxEN_); // 最大値を超えないように
	}
}
void Player::InitializeGauges() {
	// HPゲージの枠
	hpGaugeBar_ = std::make_unique<Sprite>();
	hpGaugeBar_->Initialize(
		directXCommon_,
		"white",
		{ 25.0f, 50.0f },
		{ 204.0f, 24.0f },
		{ 0.0f, 0.5f }
	);
	hpGaugeBar_->SetColor({ 0.2f, 0.2f, 0.2f, 1.0f }); // 暗い灰色

	// HPゲージの中身
	hpGaugeFill_ = std::make_unique<Sprite>();
	hpGaugeFill_->Initialize(
		directXCommon_,
		"white",
		{ 27.0f, 50.0f },
		{ 200.0f, 20.0f },
		{ 0.0f, 0.5f }
	);
	hpGaugeFill_->SetColor({ 0.0f, 1.0f, 0.0f, 1.0f }); // 緑色

	// ENゲージの枠
	enGaugeBar_ = std::make_unique<Sprite>();
	enGaugeBar_->Initialize(
		directXCommon_,
		"white",
		{ 25.0f, 80.0f },
		{ 204.0f, 24.0f },
		{ 0.0f, 0.5f }
	);
	enGaugeBar_->SetColor({ 0.2f, 0.2f, 0.2f, 1.0f }); // 暗い灰色

	// ENゲージの中身
	enGaugeFill_ = std::make_unique<Sprite>();
	enGaugeFill_->Initialize(
		directXCommon_,
		"white",
		{ 27.0f, 80.0f },
		{ 200.0f, 20.0f },
		{ 0.0f, 0.5f }
	);
	enGaugeFill_->SetColor({ 0.0f, 0.5f, 1.0f, 1.0f }); // 青色
}

void Player::UpdateGauges() {
	// HPゲージの更新
	if (hpGaugeFill_) {
		float hpRatio = currentHP_ / maxHP_;
		Vector2 currentSize = hpGaugeFill_->GetSize();
		currentSize.x = 200.0f * hpRatio; // 最大幅200に対する割合
		hpGaugeFill_->SetSize(currentSize);

		// HPが低い時は赤色に変更
		if (hpRatio < 0.3f) {
			hpGaugeFill_->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f }); // 赤色
		} else if (hpRatio < 0.6f) {
			hpGaugeFill_->SetColor({ 1.0f, 1.0f, 0.0f, 1.0f }); // 黄色
		} else {
			hpGaugeFill_->SetColor({ 0.0f, 1.0f, 0.0f, 1.0f }); // 緑色
		}
	}

	// ENゲージの更新
	if (enGaugeFill_) {
		float enRatio = currentEN_ / maxEN_;
		Vector2 currentSize = enGaugeFill_->GetSize();
		currentSize.x = 200.0f * enRatio; // 最大幅200に対する割合
		enGaugeFill_->SetSize(currentSize);

		// ENが低い時は暗い青色に変更
		if (enRatio < 0.3f) {
			enGaugeFill_->SetColor({ 0.0f, 0.2f, 0.5f, 1.0f }); // 暗い青色
		} else {
			enGaugeFill_->SetColor({ 0.0f, 0.5f, 1.0f, 1.0f }); // 通常の青色
		}
	}
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