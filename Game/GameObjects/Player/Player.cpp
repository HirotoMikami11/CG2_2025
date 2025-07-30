#include "Player.h"
#include "Managers/ImGui/ImGuiManager.h"

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
		{0.3f, 0.3f, 0.3f},           // scale（小さめに）
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
	matViewport_ = MakeViewportMatrix(0, 0, 1280, 720, 0, 1);

	// スプライト用ビュープロジェクション行列を単位行列で初期化
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

	// 移動処理
	Move();

	// 回転処理
	Rotate();

	// 攻撃処理
	Attack();


	// レティクルの更新
	UpdateReticle(viewProjectionMatrix);

	// 弾の更新
	for (auto& bullet : bullets_) {
		bullet->Update(viewProjectionMatrix);
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
}

void Player::DrawUI() {
	// 2Dレティクルスプライトの更新と描画
	sprite2DReticle_->Update(viewProjectionMatrixSprite_);
	sprite2DReticle_->Draw();
}

void Player::ImGui() {
#ifdef _DEBUG
	if (gameObject_) {
		gameObject_->ImGui();
	}

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
	// 何もしない（必要に応じて実装）
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

void Player::Attack() {
	// SPACEキーまたはゲームパッドRBボタンで発射
	bool shouldFire = input_->IsKeyTrigger(DIK_SPACE) ||
		(input_->IsGamePadConnected() && input_->IsGamePadButtonTrigger(InputManager::GamePadButton::RB));

	if (shouldFire) {
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
	// 自機から3Dレティクルへのオフセット（Z+方向）
	Vector3 offset = { 0.0f, 0.0f, 1.0f };

	// 自機のワールド行列の回転を反映
	Matrix4x4 worldMatrix = gameObject_->GetTransform().GetWorldMatrix();
	offset = Matrix4x4TransformNormal(offset, worldMatrix);

	// ベクトルの長さを整える
	offset = Normalize(offset) * kDistancePlayerTo3DReticleKeyborad;

	// 3Dレティクルの座標を設定
	Vector3 reticlePosition = GetWorldPosition() + offset;
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
