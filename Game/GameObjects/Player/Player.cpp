#include "Player.h"
#include "Managers/ImGui/ImGuiManager.h"

Player::Player() {
}

Player::~Player() {
	// unique_ptrで自動的に解放される
}

void Player::Initialize(DirectXCommon* dxCommon) {
	directXCommon_ = dxCommon;

	// 入力のシングルトンを取得
	input_ = InputManager::GetInstance();

	// ゲームオブジェクト（球体）の初期化
	gameObject_ = std::make_unique<Sphere>();
	gameObject_->Initialize(dxCommon, "sphere", "uvChecker");
	gameObject_->SetName("Player");

	// 初期位置設定
	Vector3Transform defaultTransform{
		{1.0f, 1.0f, 1.0f},  // scale
		{0.0f, 0.0f, 0.0f},  // rotate
		{0.0f, 0.0f, 0.0f}   // translate
	};
	gameObject_->SetTransform(defaultTransform);
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

	// 弾の更新
	for (auto& bullet : bullets_) {
		bullet->Update(viewProjectionMatrix);
	}

	// ゲームオブジェクトの更新
	gameObject_->Update(viewProjectionMatrix);
}

void Player::Draw(const Light& directionalLight) {
	// プレイヤー本体の描画
	gameObject_->Draw(directionalLight);

	// 弾の描画
	for (auto& bullet : bullets_) {
		bullet->Draw(directionalLight);
	}
}

void Player::ImGui() {
#ifdef _DEBUG
	if (gameObject_) {
		gameObject_->ImGui();
	}

	ImGui::Text("Bullets Count: %zu", bullets_.size());
#endif
}

Vector3 Player::GetWorldPosition() const {
	if (gameObject_) {
		return gameObject_->GetPosition();
	}
	return Vector3{ 0.0f, 0.0f, 0.0f };
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

	// 移動制限の適用
	currentPos.x = std::clamp(currentPos.x, -kMoveLimitX, kMoveLimitX);
	currentPos.y = std::clamp(currentPos.y, -kMoveLimitY, kMoveLimitY);

	// 位置を設定
	gameObject_->SetPosition(currentPos);
}

void Player::Rotate() {
	// 現在の回転を取得
	Vector3 currentRotation = gameObject_->GetRotation();

	// 押した方向で回転
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
		// 弾の速度（プレイヤーの向きに応じて計算）
		Vector3 bulletVelocity(0, 0, kBulletSpeed);

		// プレイヤーの回転を考慮して弾の方向を計算
		Vector3 rotation = gameObject_->GetRotation();
		Matrix4x4 rotationMatrix = MakeRotateYMatrix(rotation.y);
		bulletVelocity = Matrix4x4Transform(bulletVelocity, rotationMatrix);

		// 弾丸を生成・初期化する
		auto newBullet = std::make_unique<PlayerBullet>();
		newBullet->Initialize(directXCommon_, gameObject_->GetPosition(), bulletVelocity);

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