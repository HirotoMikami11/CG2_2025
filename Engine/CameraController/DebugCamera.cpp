#include "CameraController/DebugCamera.h"
#include "Managers/ImGuiManager.h" 
#include <cmath>

void DebugCamera::Initialize()
{
	input_ = InputManager::GetInstance();
	SetDefaultCamera();
}

void DebugCamera::Initialize(const Vector3& Position)
{
	input_ = InputManager::GetInstance();
	SetDefaultCamera(Position);
}

void DebugCamera::Update()
{
	// TABキーでカメラ操作の切り替え
	if (input_->IsKeyTrigger(DIK_TAB) && !input_->IsKeyDown(DIK_LSHIFT)) {
		enableCameraControl_ = !enableCameraControl_;
	}

	if (enableCameraControl_) {
		HandlephivotRotation();		// 中クリックでピボット回転
		HandleCameraMovement();		// Shift+中クリックで移動
		HandleZoom();				// マウスホイールでズーム
		///自分用キーボードで軽くカメラ操作
		HandleKeyboardMovement();	// キーボードでの移動
	}

	// 行列の更新
	UpdateMatrix();
}

void DebugCamera::SetDefaultCamera()
{
	// Cameraクラスと完全に同じデフォルト値に設定
	cameraTransform_.scale = { 1.0f, 1.0f, 1.0f };
	cameraTransform_.rotate = { 0.0f, 0.0f, 0.0f };
	cameraTransform_.translate = { 0.0f, 0.0f, -10.0f };

	// Cameraクラスの初期状態と同じ方向を向くようにターゲットを設定
	// 回転(0,0,0)の時、カメラは+Z方向を向いているので、原点をターゲットに設定
	target_ = { 0.0f, 0.0f, 0.0f };

	// 球面座標を現在の位置から計算
	UpdateSphericalFromPosition();

	// 行列の初期化
	UpdateMatrix();
}

void DebugCamera::SetDefaultCamera(const Vector3& Position)
{
	// Cameraクラスと完全に同じデフォルト値に設定（座標は引数で指定）
	cameraTransform_.scale = { 1.0f, 1.0f, 1.0f };
	cameraTransform_.rotate = { 0.0f, 0.0f, 0.0f };
	cameraTransform_.translate = Position;

	// Cameraクラスの初期状態と同じ方向を向くようにターゲットを設定
	// カメラ位置から+10のところをターゲットにする
	target_ = Position + Vector3(0.0f, 0.0f, 10.0f);

	// 球面座標を現在の位置から計算
	//SetSphericalCoordinatesForFrontFacing(Position, target_);

	// 球面座標を現在の位置から計算
	UpdateSphericalFromPosition();

	// 行列の初期化
	UpdateMatrix();
}

Vector3 DebugCamera::SphericalToCartesian(const SphericalCoordinates& spherical, const Vector3& center) const
{
	// 球面座標からデカルト座標に変換
	// radius: 半径, theta: 水平角度, phi: 垂直角度

	Vector3 result;
	result.x = spherical.radius * sinf(spherical.phi) * cosf(spherical.theta);
	result.y = spherical.radius * cosf(spherical.phi);
	result.z = spherical.radius * sinf(spherical.phi) * sinf(spherical.theta);

	return Add(result, center);
}

SphericalCoordinates DebugCamera::CartesianToSpherical(const Vector3& cartesian, const Vector3& center) const
{
	// デカルト座標から球面座標に変換
	// centerを原点とした相対座標を計算

	//相対座標を計算
	Vector3 relative = Subtract(cartesian, center);

	SphericalCoordinates result;
	result.radius = Length(relative);						//距離

	if (result.radius > 0.0f) {
		// 距離が0でない場合は角度を計算
		result.theta = atan2f(relative.z, relative.x);		//水平角
		result.phi = acosf(relative.y / result.radius);		//垂直角
	} else {
		//距離が0の場合は,角度に意味がないので0.0fに設定しておく
		result.theta = 0.0f;
		result.phi = 0.0f;
	}

	return result;
}

void DebugCamera::UpdateSphericalFromPosition()
{
	spherical_ = CartesianToSpherical(cameraTransform_.translate, target_);
}

void DebugCamera::UpdatePositionFromSpherical()
{
	cameraTransform_.translate = SphericalToCartesian(spherical_, target_);
}
//
//void DebugCamera::SetSphericalCoordinatesForFrontFacing(const Vector3& cameraPos, const Vector3& target)
//{
//
//	// ターゲットまでの距離を計算
//	Vector3 offset = target - cameraPos;
//	spherical_.radius = Length(offset);
//
//	if (spherical_.radius > 0.0f) {
//		Vector3 direction = Normalize(offset);
//
//		// Z方向が正面の場合の球面座標
//		// direction が (0, 0, 1) の場合：
//		// theta = atan2(1, 0) = π/2
//		// phi = acos(0) = π/2
//		spherical_.theta = atan2f(direction.z, direction.x);
//		spherical_.phi = acosf(direction.y);
//	}
//
//
//}

void DebugCamera::HandlephivotRotation()
{
	// 中クリック（マウスボタン2）でピボット回転
	if (input_->IsMouseButtonDown(2) && !input_->IsKeyDown(DIK_LSHIFT)) {
		Vector2 mousePos = input_->GetMousePosition();
		Vector2 preMousePos = input_->GetPreMousePosition();

		//マウスの移動量を計算
		Vector2 mouseDelta = {
			(mousePos.x - preMousePos.x),
			(mousePos.y - preMousePos.y)
		};

		// マウスの移動量を角度の変化に変換
		float deltaTheta = mouseDelta.x * rotationSensitivity_;
		float deltaphi = mouseDelta.y * rotationSensitivity_;

		// 球面座標を更新するだけなので、Thetaとphiを加える
		spherical_.theta -= deltaTheta;		// 水平回転
		spherical_.phi -= deltaphi;			// 垂直回転

		// 制限
		spherical_.phi = std::clamp(spherical_.phi, minphi_, maxphi_);

		// 球面座標から3D位置にカメラ位置を更新
		UpdatePositionFromSpherical();
	}
}

void DebugCamera::HandleCameraMovement()
{
	// Shift+中クリックでカメラ移動（ターゲットも一緒に移動）
	if (input_->IsMouseButtonDown(2) && input_->IsKeyDown(DIK_LSHIFT)) {
		Vector2 mousePos = input_->GetMousePosition();
		Vector2 preMousePos = input_->GetPreMousePosition();

		Vector2 mouseDelta = {
			(mousePos.x - preMousePos.x),
			(mousePos.y - preMousePos.y)
		};

		// カメラのローカル軸を取得
		Vector3 right = GetCameraRight();
		Vector3 up = GetCameraUp();

		// 移動ベクトルを計算
		Vector3 moveVector = Add(
			Multiply(right, -mouseDelta.x * movementSensitivity_),
			Multiply(up, mouseDelta.y * movementSensitivity_)
		);

		// ターゲットとカメラ位置を同時に移動
		target_ = Add(target_, moveVector);
		cameraTransform_.translate = Add(cameraTransform_.translate, moveVector);
	}
}

void DebugCamera::HandleZoom()
{
	// マウスホイールでズーム
	float wheelDelta = static_cast<float>(input_->GetMouseWheel());
	if (wheelDelta != 0.0f) {
		// 距離を変更
		spherical_.radius -= wheelDelta * zoomSensitivity_;

		// 距離を制限
		spherical_.radius = std::clamp(spherical_.radius, minDistance_, maxDistance_);

		// 球面座標からカメラ位置を更新
		UpdatePositionFromSpherical();
	}
}

void DebugCamera::HandleKeyboardMovement()
{
	Vector3 moveVector = { 0.0f, 0.0f, 0.0f };
	float keyboardSpeed = 0.1f;

	// カメラのローカル軸を取得
	Vector3 forward = GetCameraForward();
	Vector3 right = GetCameraRight();
	Vector3 up = { 0.0f, 1.0f, 0.0f }; // ワールドアップベクトル

	// WASD移動
	if (input_->IsKeyDown(DIK_W)) {
		moveVector = Add(moveVector, forward);
	}
	if (input_->IsKeyDown(DIK_S)) {
		moveVector = Subtract(moveVector, forward);
	}
	if (input_->IsKeyDown(DIK_D)) {
		moveVector = Add(moveVector, right);
	}
	if (input_->IsKeyDown(DIK_A)) {
		moveVector = Subtract(moveVector, right);
	}

	// QE で上下移動
	if (input_->IsKeyDown(DIK_Q)) {
		moveVector = Add(moveVector, up);
	}
	if (input_->IsKeyDown(DIK_E)) {
		moveVector = Subtract(moveVector, up);
	}

	// 移動ベクトルを正規化してスピードを適用
	if (Length(moveVector) > 0.0f) {
		moveVector = Normalize(moveVector);
		moveVector = Multiply(moveVector, keyboardSpeed);

		// ターゲットとカメラ位置を同時に移動
		target_ = Add(target_, moveVector);
		cameraTransform_.translate = Add(cameraTransform_.translate, moveVector);
	}
}

void DebugCamera::UpdateMatrix()
{
	// ターゲット方向のベクトルを計算
	Vector3 forward = Normalize(Subtract(target_, cameraTransform_.translate));

	// Y軸回転（Yaw）を計算 - atan2の引数順序を既存の座標系に合わせる
	float yaw = atan2f(forward.x, forward.z);

	// X軸回転（phitch）を計算
	float phitch = asinf(-forward.y);

	// 回転角度をcameraTransformに設定
	cameraTransform_.rotate = { phitch, yaw, 0.0f };

	//ビュー行列
	Matrix4x4 cameraMatrix = MakeAffineMatrix(
		cameraTransform_.scale,
		cameraTransform_.rotate,
		cameraTransform_.translate
	);
	viewMatrix_ = Matrix4x4Inverse(cameraMatrix);

	// プロジェクション行列
	projectionMatrix_ = MakePerspectiveFovMatrix(
		0.45f,
		(float(GraphicsConfig::kClientWidth) / float(GraphicsConfig::kClientHeight)),
		0.1f,
		100.0f
	);

	// ビュープロジェクション行列
	viewProjectionMatrix_ = Matrix4x4Multiply(viewMatrix_, projectionMatrix_);
}

Vector3 DebugCamera::GetCameraForward() const
{
	// 現在のカメラ行列を取得
	Matrix4x4 cameraMatrix = MakeAffineMatrix(
		cameraTransform_.scale,
		cameraTransform_.rotate,
		cameraTransform_.translate
	);

	// +Z方向のローカルベクトルをワールド方向に変換
	Vector3 localForward = { 0.0f, 0.0f, 1.0f };
	return TransformDirection(localForward, cameraMatrix);
}

Vector3 DebugCamera::GetCameraRight() const
{
	// 現在のカメラ行列を取得
	Matrix4x4 cameraMatrix = MakeAffineMatrix(
		cameraTransform_.scale,
		cameraTransform_.rotate,
		cameraTransform_.translate
	);

	// +X方向のローカルベクトルをワールド方向に変換
	Vector3 localRight = { 1.0f, 0.0f, 0.0f };
	return TransformDirection(localRight, cameraMatrix);
}

Vector3 DebugCamera::GetCameraUp() const
{
	// 現在のカメラ行列を取得
	Matrix4x4 cameraMatrix = MakeAffineMatrix(
		cameraTransform_.scale,
		cameraTransform_.rotate,
		cameraTransform_.translate
	);

	// +Y方向のローカルベクトルをワールド方向に変換
	Vector3 localUp = { 0.0f, 1.0f, 0.0f };
	return TransformDirection(localUp, cameraMatrix);
}

void DebugCamera::SetPositon(const Vector3& position)
{
	cameraTransform_.translate = position;
	UpdateSphericalFromPosition();
}

void DebugCamera::ImGui()
{
#ifdef _DEBUG
	ImGui::Text("DebugCamera");
	ImGui::Separator();

	// カメラ操作の有効/無効
	ImGui::Text("Camera Control: %s", enableCameraControl_ ? "ON" : "OFF");
	if (ImGui::Button(enableCameraControl_ ? "Disable Control" : "Enable Control")) {
		enableCameraControl_ = !enableCameraControl_;
	}

	ImGui::Separator();

	// 現在の座標情報
	ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)",
		cameraTransform_.translate.x, cameraTransform_.translate.y, cameraTransform_.translate.z);
	ImGui::Text("Target Position: (%.2f, %.2f, %.2f)",
		target_.x, target_.y, target_.z);

	// 球面座標情報
	ImGui::Text("Spherical Coordinates:");
	ImGui::Text("  Radius: %.2f", spherical_.radius);
	ImGui::Text("  Theta: %.2f rad (%.1f deg)", spherical_.theta, spherical_.theta * 180.0f / 3.14159f);
	ImGui::Text("  phi: %.2f rad (%.1f deg)", spherical_.phi, spherical_.phi * 180.0f / 3.14159f);

	ImGui::Separator();

	// パラメータ調整
	ImGui::SliderFloat3("Target Position", &target_.x, -20.0f, 20.0f);
	ImGui::SliderFloat("Distance", &spherical_.radius, minDistance_, maxDistance_);

	if (ImGui::SliderFloat("Theta", &spherical_.theta, -3.14159f, 3.14159f) ||
		ImGui::SliderFloat("phi", &spherical_.phi, minphi_, maxphi_)) {
		UpdatePositionFromSpherical();
	}

	ImGui::Separator();

	// 感度設定
	ImGui::SliderFloat("Rotation Sensitivity", &rotationSensitivity_, 0.001f, 0.01f);
	ImGui::SliderFloat("Movement Sensitivity", &movementSensitivity_, 0.001f, 0.1f);
	ImGui::SliderFloat("Zoom Sensitivity", &zoomSensitivity_, 0.01f, 1.0f);

	ImGui::Separator();

	// リセットボタン
	if (ImGui::Button("Reset Camera")) {
		SetDefaultCamera();
	}

	ImGui::Separator();

	// 操作説明
	ImGui::Text("TAB: Toggle camera control");
	ImGui::Text("Middle Mouse: Orbit around target");
	ImGui::Text("Shift + Middle Mouse: Pan camera");
	ImGui::Text("Mouse Wheel: Zoom in/out");
	ImGui::Text("WASD: Move camera and target");
	ImGui::Text("QE: Move up/down");

#endif
}