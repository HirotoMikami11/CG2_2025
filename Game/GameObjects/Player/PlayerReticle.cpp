#include "PlayerReticle.h"
#include "Managers/ImGui/ImGuiManager.h"

void PlayerReticle::Initialize(DirectXCommon* dxCommon) {
	directXCommon_ = dxCommon;
	input_ = InputManager::GetInstance();

	// 3Dレティクルの初期化
	reticle3D_ = std::make_unique<Sphere>();
	reticle3D_->Initialize(dxCommon, "sphere"); // 仮でplayerモデルを使用
	reticle3D_->SetName("Reticle3D");

	Vector3Transform reticleTransform{
		{0.5f, 0.5f, 0.5f},           // scale（わかりやすくデカく）
		{0.0f, 0.0f, 0.0f},           // rotate
		{0.0f, 0.0f, 50.0f}           // translate（プレイヤーの前方）
	};

	reticle3D_->SetTransform(reticleTransform);
	reticle3D_->SetColor({ 1.0f,0.0f,0.0f,1.0f });

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
}

void PlayerReticle::Update(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewProjectionMatrixSprite) {
	UpdateReticle(viewProjectionMatrix);

	// 3Dレティクルの更新
	reticle3D_->Update(viewProjectionMatrix);

	// 2Dレティクルの更新
	sprite2DReticle_->Update(viewProjectionMatrixSprite);
}

void PlayerReticle::Draw3D(const Light& directionalLight) {
	// 3Dレティクルの描画
	reticle3D_->Draw(directionalLight);
}

void PlayerReticle::Draw2D() {
	if (show2DReticle_) {
		sprite2DReticle_->Draw();
	}
}

Vector3 PlayerReticle::GetWorldPosition3DReticle() const {
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

Vector2 PlayerReticle::GetPosition2DReticle() const {
	if (sprite2DReticle_) {
		return sprite2DReticle_->GetPosition();
	}
	return Vector2{ 640.0f, 360.0f }; // デフォルト位置
}

void PlayerReticle::UpdateReticle(const Matrix4x4& viewProjectionMatrix) {
	if (input_->IsGamePadConnected()) {
		// ゲームパッドが接続されている場合：右スティックでレティクル操作
		ConvertGamePadToWorldReticle(viewProjectionMatrix);
	} else {
		// キーボードの場合：上下左右キーでレティクル操作
		ConvertKeyboardToWorldReticle(viewProjectionMatrix);
	}
}

void PlayerReticle::ConvertGamePadToWorldReticle(const Matrix4x4& viewProjectionMatrix) {
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

	posNear_ = Transform(posNear_, matInverseVPV);
	posFar_ = Transform(posFar_, matInverseVPV);

	Vector3 direction = Normalize(posFar_ - posNear_);
	spritePosition_ = posNear_ + (direction * kDistancePlayerTo3DReticleGamepad_);
	reticle3D_->SetPosition(spritePosition_);
}

void PlayerReticle::ConvertKeyboardToWorldReticle(const Matrix4x4& viewProjectionMatrix) {
	// 現在のスプライト位置を取得
	Vector2 spritePos = sprite2DReticle_->GetPosition();

	// 上下左右キーでレティクル移動
	if (input_->IsKeyDown(DIK_LEFT)) {
		spritePos.x -= kReticleSpeed_; // 左
	}
	if (input_->IsKeyDown(DIK_RIGHT)) {
		spritePos.x += kReticleSpeed_; // 右
	}
	if (input_->IsKeyDown(DIK_UP)) {
		spritePos.y -= kReticleSpeed_; // 上
	}
	if (input_->IsKeyDown(DIK_DOWN)) {
		spritePos.y += kReticleSpeed_; // 下
	}

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

	posNear_ = Transform(posNear_, matInverseVPV);
	posFar_ = Transform(posFar_, matInverseVPV);

	Vector3 direction = Normalize(posFar_ - posNear_);
	spritePosition_ = posNear_ + (direction * kDistancePlayerTo3DReticleKeyboard_);
	reticle3D_->SetPosition(spritePosition_);
}

void PlayerReticle::ImGui() {
#ifdef _DEBUG
	if (ImGui::TreeNode("Reticle System")) {
		// 表示設定
		ImGui::Checkbox("Show 2D Reticle", &show2DReticle_);

		// レティクル設定
		ImGui::Separator();
		ImGui::DragFloat("Reticle Speed", &kReticleSpeed_, 0.5f, 1.0f, 50.0f);
		ImGui::DragFloat("Distance (Keyboard)", &kDistancePlayerTo3DReticleKeyboard_, 1.0f, 10.0f, 200.0f);
		ImGui::DragFloat("Distance (Gamepad)", &kDistancePlayerTo3DReticleGamepad_, 1.0f, 10.0f, 200.0f);

		// レティクル情報
		ImGui::Separator();
		Vector3 reticlePos = GetWorldPosition3DReticle();
		ImGui::Text("3D Reticle Position: (%.2f, %.2f, %.2f)", reticlePos.x, reticlePos.y, reticlePos.z);

		Vector2 spritePos = GetPosition2DReticle();
		ImGui::Text("2D Reticle Position: (%.2f, %.2f)", spritePos.x, spritePos.y);

		// 入力情報
		ImGui::Separator();
		ImGui::Text("Input Device: %s", input_->IsGamePadConnected() ? "Gamepad" : "Keyboard");
		if (input_->IsGamePadConnected()) {
			float stickX = input_->GetAnalogStick(InputManager::AnalogStick::RIGHT_X);
			float stickY = input_->GetAnalogStick(InputManager::AnalogStick::RIGHT_Y);
			ImGui::Text("Right Stick: (%.2f, %.2f)", stickX, stickY);
		}

		ImGui::TreePop();
	}
#endif
}