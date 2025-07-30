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

	// ゲームオブジェクト（モデル）の初期化
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

	// 衝突判定設定
	SetRadius(1.0f); // Colliderの半径をセット
	/// 衝突属性の設定
	SetCollisionAttribute(kCollisionAttributePlayer);
	/// 衝突対象は自分の属性以外に設定(ビット反転)
	SetCollisionMask(~kCollisionAttributePlayer);

	// レティクル関連の初期化
	// 3Dレティクル用のTransform3Dの初期化
	transform3DReticle_.Initialize(directXCommon_);

	// 2Dレティクル用のスプライト
	sprite2DReticle_ = std::make_unique<Sprite>();
	sprite2DReticle_->Initialize(directXCommon_, "reticle", { 640, 360 }, { 64, 64 });

	// ビューポート行列の設定
	matViewport_ = MakeViewportMatrix(0, 0, 1280, 720, 0, 1); // 画面サイズに合わせて調整
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

	// レティクル更新
	UpdateReticle();

	// 弾の更新
	for (auto& bullet : bullets_) {
		bullet->Update(viewProjectionMatrix);
	}

	// ゲームオブジェクトの更新
	gameObject_->Update(viewProjectionMatrix);

	// 3Dレティクルの更新
	transform3DReticle_.UpdateMatrix(MakeIdentity4x4());

	// スプライトの更新
	if (sprite2DReticle_) {
		Matrix4x4 spriteViewProjection = MakeIdentity4x4(); // スプライト用の行列
		sprite2DReticle_->Update(spriteViewProjection);
	}
}

void Player::Draw(const Light& directionalLight) {
	// プレイヤー本体の描画
	gameObject_->Draw(directionalLight);

	// 弾の描画
	for (auto& bullet : bullets_) {
		bullet->Draw(directionalLight);
	}
}

void Player::DrawUI() {
	// 2Dレティクルの描画
	if (sprite2DReticle_) {
		sprite2DReticle_->Draw();
	}
}

void Player::ImGui() {
#ifdef _DEBUG
	if (gameObject_) {
		gameObject_->ImGui();
	}

	ImGui::Text("Bullets Count: %zu", bullets_.size());

	// KamataEngineのデバッグ表示と同様の情報を表示
	Vector3 worldPos = GetWorldPosition();
	ImGui::Text("World Position: (%.2f, %.2f, %.2f)", worldPos.x, worldPos.y, worldPos.z);

	Vector3 localPos = gameObject_->GetPosition();
	ImGui::Text("Local Position: (%.2f, %.2f, %.2f)", localPos.x, localPos.y, localPos.z);

	// レティクルの情報
	Vector3 reticlePos = GetWorldPosition3DReticle();
	ImGui::Text("Reticle Position: (%.2f, %.2f, %.2f)", reticlePos.x, reticlePos.y, reticlePos.z);
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
	// 3Dレティクルのワールド行列から移動成分を取得
	Matrix4x4 worldMatrix = transform3DReticle_.GetWorldMatrix();
	return Vector3{
		worldMatrix.m[3][0],
		worldMatrix.m[3][1],
		worldMatrix.m[3][2]
	};
}

void Player::OnCollision() {
	// 何もしない（必要に応じて実装）
}

void Player::Move() {
	// 移動ベクトル
	Vector3 move = { 0.0f, 0.0f, 0.0f };

	// 押した方向で移動ベクトルを変更
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
	// SPACEキーで発射
	if (input_->IsKeyTrigger(DIK_SPACE)) {
		// 弾の速度
		// 自機からレティクルへのベクトル
		Vector3 bulletVelocity = GetWorldPosition3DReticle() - GetWorldPosition();
		bulletVelocity = Normalize(bulletVelocity) * kBulletSpeed;

		// 弾丸を生成・初期化する
		auto newBullet = std::make_unique<PlayerBullet>();
		newBullet->Initialize(directXCommon_, GetWorldPosition(), bulletVelocity);

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

void Player::UpdateReticle() {
	// 簡易実装：固定距離にレティクルを配置
	ConvertMouseToWorldReticle();
}

void Player::ConvertWorldToScreenReticle() {
	// 実装が必要な場合は後で追加
}

void Player::ConvertMouseToWorldReticle() {
	// 簡易実装：プレイヤーの前方固定距離にレティクルを配置
	const float kDistancePlayerTo3DReticle = 50.0f;
	Vector3 offset = { 0, 0, 1.0f };

	// プレイヤーのワールド行列の回転を反映
	Matrix4x4 worldMatrix = gameObject_->GetTransform().GetWorldMatrix();
	offset = Matrix4x4TransformNormal(offset, worldMatrix);
	offset = Normalize(offset) * kDistancePlayerTo3DReticle;

	// 3Dレティクルの座標を設定
	Vector3 reticlePosition = GetWorldPosition() + offset;
	transform3DReticle_.SetPosition(reticlePosition);
}

void Player::ConvertGamepadToWorldReticle() {
	// ゲームパッド対応は後で実装
}