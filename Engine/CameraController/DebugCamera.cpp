#include "DebugCamera.h"
#include "Managers/ImGui/ImGuiManager.h"
#include <cmath>

DebugCamera::DebugCamera()
	: cameraTransform_{
		.scale{1.0f, 1.0f, 1.0f},
		.rotate{0.0f, 0.0f, 0.0f},
		.translate{0.0f, 0.0f, -10.0f}
	}
	, viewProjectionMatrix_{}
	, viewMatrix_{}
	, projectionMatrix_{}
	, spherical_{}
	, cartesianPosition_{ 0.0f, 0.0f, -10.0f }
	, cartesianRotation_{ 0.0f, 0.0f, 0.0f }
	, input_(nullptr) {
}

DebugCamera::~DebugCamera() = default;

void DebugCamera::Initialize(const Vector3& position, const Vector3& rotation) {
	input_ = InputManager::GetInstance();
	// 初期値を保存
	initialPosition_ = position;
	initialRotation_ = rotation;
	SetDefaultCamera(position, rotation);
}

void DebugCamera::Update() {
	HandleDebugInput();

	if (enableCameraControl_) {
		HandlePivotRotation();      // 中クリックでピボット回転
		HandleCameraMovement();     // Shift+中クリックで移動
		HandleZoom();               // マウスホイールでズーム
		HandleKeyboardMovement();   // キーボードでの移動
	}

	// デカルト座標系の変数を現在の状態に同期
	cartesianPosition_ = cameraTransform_.translate;
	cartesianRotation_ = cameraTransform_.rotate;

	// 行列の更新
	UpdateMatrix();
}

Matrix4x4 DebugCamera::GetSpriteViewProjectionMatrix() const {
	// スプライト用行列を作成して返す
	Matrix4x4 spriteViewMatrix = MakeIdentity4x4();
	Matrix4x4 spriteProjectionMatrix = MakeOrthograpicMatrix(
		0.0f, 0.0f,
		float(GraphicsConfig::kClientWidth),
		float(GraphicsConfig::kClientHeight),
		0.0f, 1000.0f
	);
	return Matrix4x4Multiply(spriteViewMatrix, spriteProjectionMatrix);
}

void DebugCamera::SetPosition(const Vector3& position) {
	cameraTransform_.translate = position;
	cartesianPosition_ = position;
	UpdateSphericalFromPosition();
}

void DebugCamera::SetDefaultCamera(const Vector3& position, const Vector3& rotation) {
	// デフォルト値に設定（座標・回転は引数で指定）
	cameraTransform_.scale = { 1.0f, 1.0f, 1.0f };
	cameraTransform_.rotate = rotation;
	cameraTransform_.translate = position;

	// デカルト座標系の変数も同期
	cartesianPosition_ = position;
	cartesianRotation_ = rotation;

	// 回転を反映して10m先にターゲットを配置
	target_ = CalculateTargetFromRotation(position, rotation);
	// プロジェクション行列
	projectionMatrix_ = MakePerspectiveFovMatrix(
		0.45f,
		(float(GraphicsConfig::kClientWidth) / float(GraphicsConfig::kClientHeight)),
		0.1f,
		1000.0f
	);

	// 球面座標を現在の位置から計算
	UpdateSphericalFromPosition();

	// 行列の初期化
	UpdateMatrix();
}

Vector3 DebugCamera::CalculateTargetFromRotation(const Vector3& position, const Vector3& rotation) const {
	// 回転行列を作成
	Matrix4x4 rotationMatrix = MakeRotateXYZMatrix(rotation);

	// 前方ベクトル（Z軸の正方向）を回転させる
	Vector3 forward = { 0.0f, 0.0f, 1.0f };
	Vector3 rotatedForward = TransformDirection(forward, rotationMatrix);

	// 10m先の座標を計算
	Vector3 target = Add(position, Multiply(rotatedForward, 10.0f));

	return target;
}

Vector3 DebugCamera::SphericalToCartesian(const SphericalCoordinates& spherical, const Vector3& center) const {
	// 球面座標からデカルト座標に変換
	Vector3 result;
	result.x = spherical.radius * sinf(spherical.phi) * cosf(spherical.theta);
	result.y = spherical.radius * cosf(spherical.phi);
	result.z = spherical.radius * sinf(spherical.phi) * sinf(spherical.theta);

	return Add(result, center);
}

SphericalCoordinates DebugCamera::CartesianToSpherical(const Vector3& cartesian, const Vector3& center) const {
	// デカルト座標から球面座標に変換
	// centerを原点とした相対座標を計算
	Vector3 relative = Subtract(cartesian, center);

	SphericalCoordinates result;
	result.radius = Length(relative);

	if (result.radius > 0.0f) {
		// 距離が0でない場合は角度を計算
		result.theta = atan2f(relative.z, relative.x);     // 水平角
		result.phi = acosf(relative.y / result.radius);    // 垂直角
	} else {
		// 距離が0の場合は、角度に意味がないので0.0fに設定しておく
		result.theta = 0.0f;
		result.phi = 0.0f;
	}

	return result;
}

void DebugCamera::UpdateSphericalFromPosition() {
	spherical_ = CartesianToSpherical(cameraTransform_.translate, target_);
}

void DebugCamera::UpdatePositionFromSpherical() {
	cameraTransform_.translate = SphericalToCartesian(spherical_, target_);
}

void DebugCamera::HandleDebugInput() {
	// TABキーでカメラ操作の切り替え
	if (input_->IsKeyTrigger(DIK_TAB) && !input_->IsKeyDown(DIK_LSHIFT)) {
		enableCameraControl_ = !enableCameraControl_;
	}

	// Shift+Enterでカメラコントロールの切り替え
	if (input_->IsKeyTrigger(DIK_RETURN) && input_->IsKeyDown(DIK_LSHIFT)) {
		enableCameraControl_ = !enableCameraControl_;
	}
}

void DebugCamera::HandlePivotRotation() {
	// 中クリック（マウスボタン2）でピボット回転
	if (input_->IsMouseButtonDown(2) && !input_->IsKeyDown(DIK_LSHIFT)) {
		Vector2 mousePos = input_->GetMousePosition();
		Vector2 preMousePos = input_->GetPreMousePosition();

		// マウスの移動量を計算
		Vector2 mouseDelta = {
			(mousePos.x - preMousePos.x),
			(mousePos.y - preMousePos.y)
		};

		// マウスの移動量を角度の変化に変換
		float deltaTheta = mouseDelta.x * rotationSensitivity_;
		float deltaPhi = mouseDelta.y * rotationSensitivity_;

		// 球面座標を更新するだけなので、Thetaとphiを加える
		spherical_.theta -= deltaTheta;     // 水平回転
		spherical_.phi -= deltaPhi;         // 垂直回転

		// 制限
		spherical_.phi = std::clamp(spherical_.phi, minPhi_, maxPhi_);

		// 球面座標から3D位置にカメラ位置を更新
		UpdatePositionFromSpherical();
	}
}

void DebugCamera::HandleCameraMovement() {
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

		// 移動ベクトルを計算（速度を可変に）
		Vector3 moveVector = Add(
			Multiply(right, -mouseDelta.x * mousePanSpeed_),
			Multiply(up, mouseDelta.y * mousePanSpeed_)
		);

		// ターゲットとカメラ位置を同時に移動
		target_ = Add(target_, moveVector);
		cameraTransform_.translate = Add(cameraTransform_.translate, moveVector);
	}
}

void DebugCamera::HandleZoom() {
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

void DebugCamera::HandleKeyboardMovement() {
	Vector3 moveVector = { 0.0f, 0.0f, 0.0f };

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
		moveVector = Multiply(moveVector, keyboardSpeed_);

		// ターゲットとカメラ位置を同時に移動
		target_ = Add(target_, moveVector);
		cameraTransform_.translate = Add(cameraTransform_.translate, moveVector);
	}
}

void DebugCamera::UpdateMatrix() {
	// ターゲット方向のベクトルを計算
	Vector3 forward = Normalize(Subtract(target_, cameraTransform_.translate));

	// Y軸回転（Yaw）を計算 - atan2の引数順序を既存の座標系に合わせる
	float yaw = atan2f(forward.x, forward.z);

	// X軸回転（Pitch）を計算
	float pitch = asinf(-forward.y);

	// 回転角度をcameraTransformに設定
	cameraTransform_.rotate = { pitch, yaw, 0.0f };

	// ビュー行列
	Matrix4x4 cameraMatrix = MakeAffineMatrix(
		cameraTransform_.scale,
		cameraTransform_.rotate,
		cameraTransform_.translate
	);
	viewMatrix_ = Matrix4x4Inverse(cameraMatrix);
	// ビュープロジェクション行列
	viewProjectionMatrix_ = Matrix4x4Multiply(viewMatrix_, projectionMatrix_);
}

Vector3 DebugCamera::GetCameraForward() const {
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

Vector3 DebugCamera::GetCameraRight() const {
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

Vector3 DebugCamera::GetCameraUp() const {
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

void DebugCamera::ImGui() {
#ifdef _DEBUG
	ImGui::Text("DebugCamera");
	ImGui::Separator();

	// カメラ操作の有効/無効
	ImGui::Text("Camera Control: %s", enableCameraControl_ ? "ON" : "OFF");
	if (ImGui::Button(enableCameraControl_ ? "Disable Control" : "Enable Control")) {
		enableCameraControl_ = !enableCameraControl_;
	}

	ImGui::Separator();

	// ターゲット座標
	ImGui::Text("Target Position: (%.2f, %.2f, %.2f)",
		target_.x, target_.y, target_.z);

	if (ImGui::DragFloat3("Target", &target_.x, 0.1f, -100.0f, 100.0f)) {
		UpdateSphericalFromPosition();
	}

	ImGui::Separator();

	// デカルト座標系の設定（折りたたみ可能）
	if (ImGui::CollapsingHeader("Cartesian Coordinates", ImGuiTreeNodeFlags_DefaultOpen)) {
		bool positionChanged = false;
		bool rotationChanged = false;

		if (ImGui::DragFloat3("Position", &cartesianPosition_.x, 0.1f, -100.0f, 100.0f)) {
			cameraTransform_.translate = cartesianPosition_;
			positionChanged = true;
		}

		if (ImGui::DragFloat3("Rotation", &cartesianRotation_.x, 0.01f, -3.14159f, 3.14159f)) {
			cameraTransform_.rotate = cartesianRotation_;
			rotationChanged = true;
		}

		// 位置または回転が変更された場合、ターゲットを再計算
		if (positionChanged || rotationChanged) {
			target_ = CalculateTargetFromRotation(cartesianPosition_, cartesianRotation_);
			UpdateSphericalFromPosition();
		}
	}

	ImGui::Separator();

	// 球面座標系の設定（折りたたみ可能）
	if (ImGui::CollapsingHeader("Spherical Coordinates")) {
		ImGui::Text("  Radius: %.2f", spherical_.radius);
		ImGui::Text("  Theta: %.2f rad (%.1f deg)", spherical_.theta, spherical_.theta * 180.0f / 3.14159f);
		ImGui::Text("  Phi: %.2f rad (%.1f deg)", spherical_.phi, spherical_.phi * 180.0f / 3.14159f);

		ImGui::DragFloat("Distance", &spherical_.radius, 0.1f, minDistance_, maxDistance_);

		if (ImGui::DragFloat("Theta", &spherical_.theta, 0.01f, -3.14159f, 3.14159f) ||
			ImGui::DragFloat("Phi", &spherical_.phi, 0.01f, minPhi_, maxPhi_)) {
			UpdatePositionFromSpherical();
		}
	}

	ImGui::Separator();

	// 移動・操作設定（折りたたみ可能）
	if (ImGui::CollapsingHeader("Movement & Control Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::DragFloat("Keyboard Speed", &keyboardSpeed_, 0.01f, 0.01f, 2.0f);
		ImGui::DragFloat("Mouse Pan Speed", &mousePanSpeed_, 0.001f, 0.001f, 0.1f);
		ImGui::DragFloat("Rotation Sensitivity", &rotationSensitivity_, 0.0001f, 0.001f, 0.01f);
		ImGui::DragFloat("Zoom Sensitivity", &zoomSensitivity_, 0.001f, 0.01f, 1.0f);
	}

	ImGui::Separator();

	// リセットボタン
	if (ImGui::Button("Reset Camera")) {
		SetDefaultCamera(initialPosition_, initialRotation_);
	}

	ImGui::Separator();

	// 操作説明
	if (ImGui::CollapsingHeader("Controls", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Text("TAB: Toggle camera control");
		ImGui::Text("Shift + Enter: Toggle camera control");
		ImGui::Text("Middle Mouse: Orbit around target");
		ImGui::Text("Shift + Middle Mouse: Pan camera");
		ImGui::Text("Mouse Wheel: Zoom in/out");
		ImGui::Text("WASD: Move camera and target");
		ImGui::Text("QE: Move up/down");
	}

#endif
}