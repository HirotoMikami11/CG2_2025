#include "RailCamera.h"
#include "Managers/ImGui/ImGuiManager.h"

RailCamera::RailCamera() {
}

RailCamera::~RailCamera() {
	ReleaseResources();
}

void RailCamera::Initialize(const Vector3& position, const Vector3& rotation) {
	initialPosition_ = position;
	initialRotation_ = rotation;

	directXCommon_ = Engine::GetInstance()->GetDirectXCommon();

	transform_.Initialize(directXCommon_);
	SetDefaultCamera(position, rotation);

	// コンポーネントの初期化
	track_ = std::make_unique<RailTrack>();
	movement_ = std::make_unique<RailCameraMovement>();
	movement_->Initialize(track_.get());

	debugger_ = std::make_unique<RailCameraDebugger>();
	debugger_->Initialize(directXCommon_, track_.get());

	lastValidRotation_ = rotation;
}

void RailCamera::Update() {
	if (movement_) {
		movement_->Update();
		UpdateCameraTransform();
	}

	Matrix4x4 identityMatrix = MakeIdentity4x4();
	transform_.UpdateMatrix(identityMatrix);
}

void RailCamera::UpdateCameraTransform() {
	if (!movement_ || !track_ || !track_->IsValid()) {
		return;
	}

	// 現在の位置を取得
	Vector3 currentPosition = movement_->GetCurrentPosition();
	transform_.SetPosition(currentPosition);

	// 終端に到達している場合は最後の回転を維持
	if (movement_->IsAtEnd()) {
		transform_.SetRotation(lastValidRotation_);
		return;
	}

	// 注視点を取得
	Vector3 lookAtTarget = movement_->GetLookAtTarget();

	// 視点から注視点への方向ベクトルを計算
	Vector3 forward = lookAtTarget - currentPosition;

	// 方向ベクトルが極小の場合は最後の回転を維持
	if (Length(forward) < 0.001f) {
		transform_.SetRotation(lastValidRotation_);
		return;
	}

	forward = Normalize(forward);

	// 方向ベクトルから回転角を計算
	Vector3 rotation = transform_.GetRotation();
	rotation.y = std::atan2(forward.x, forward.z);
	float horizontalDistance = std::sqrt(forward.x * forward.x + forward.z * forward.z);
	rotation.x = -std::atan2(forward.y, horizontalDistance);
	rotation.z = 0.0f;

	transform_.SetRotation(rotation);
	lastValidRotation_ = rotation;
}

Vector3 RailCamera::GetForwardDirection() const {
	if (!track_ || !track_->IsValid()) {
		return Vector3{ 0.0f, 0.0f, 1.0f };
	}

	if (movement_ && movement_->IsAtEnd()) {
		Vector3 forward;
		forward.x = std::sin(lastValidRotation_.y) * std::cos(lastValidRotation_.x);
		forward.y = -std::sin(lastValidRotation_.x);
		forward.z = std::cos(lastValidRotation_.y) * std::cos(lastValidRotation_.x);
		return Normalize(forward);
	}

	// 現在の位置から少し先の位置を計算して進行方向を求める
	float currentT = movement_ ? movement_->GetProgress() : 0.0f;
	float lookAheadT = std::min(currentT + 0.001f, 1.0f);

	Vector3 currentPos = track_->CalculatePosition(currentT);
	Vector3 futurePos = track_->CalculatePosition(lookAheadT);

	Vector3 forwardDirection = futurePos - currentPos;

	if (Length(forwardDirection) < 0.001f) {
		Vector3 currentRotation = transform_.GetRotation();
		Vector3 forward;
		forward.x = std::sin(currentRotation.y) * std::cos(currentRotation.x);
		forward.y = -std::sin(currentRotation.x);
		forward.z = std::cos(currentRotation.y) * std::cos(currentRotation.x);
		return Normalize(forward);
	}

	return Normalize(forwardDirection);
}

void RailCamera::SetControlPoints(const std::vector<Vector3>& controlPoints) {
	if (track_) {
		track_->SetControlPoints(controlPoints);

		// 等間隔移動が有効な場合は長さテーブルを構築
		if (movement_ && movement_->IsUniformSpeedEnabled()) {
			track_->BuildLengthTable();
		}

		// 軌道の線分を再生成
		if (debugger_) {
			debugger_->GenerateRailTrackLines();
		}
	}
}

void RailCamera::DrawRailTrack(const Matrix4x4& viewProjectionMatrix, const Light& directionalLight) {
	if (debugger_) {
		debugger_->UpdateCameraModel(transform_.GetPosition(), transform_.GetRotation(), viewProjectionMatrix);
		debugger_->Draw(viewProjectionMatrix, directionalLight, transform_.GetPosition(), transform_.GetRotation());
	}
}

void RailCamera::ImGui() {
	// 最小限の情報のみ表示（詳細はRailCameraEditorで処理）
#ifdef _DEBUG
	ImGui::Text("RailCamera (Use RailCameraEditor for controls)");
	ImGui::Text("Position: (%.2f, %.2f, %.2f)",
		transform_.GetPosition().x,
		transform_.GetPosition().y,
		transform_.GetPosition().z);
	ImGui::Text("Progress: %.1f%%", GetProgress() * 100.0f);
#endif
}

Matrix4x4 RailCamera::GetViewProjectionMatrix() const {
	Matrix4x4 viewMatrix = Matrix4x4Inverse(transform_.GetWorldMatrix());
	return Matrix4x4Multiply(viewMatrix, projectionMatrix_);
}

Matrix4x4 RailCamera::GetSpriteViewProjectionMatrix() const {
	Matrix4x4 spriteViewMatrix = MakeIdentity4x4();
	return Matrix4x4Multiply(spriteViewMatrix, spriteProjectionMatrix_);
}

void RailCamera::ReleaseResources() {
	debugger_.reset();
	movement_.reset();
	track_.reset();
	directXCommon_ = nullptr;

	Logger::Log(Logger::GetStream(), "RailCamera: Resources released\n");
}

void RailCamera::SetDefaultCamera(const Vector3& position, const Vector3& rotation) {
	transform_.SetPosition(position);
	transform_.SetRotation(rotation);
	transform_.SetScale({ 1.0f, 1.0f, 1.0f });

	lastValidRotation_ = rotation;

	float fov = 0.45f;
	float nearClip = 0.1f;
	float farClip = 1000.0f;
	float aspectRatio = (float(GraphicsConfig::kClientWidth) / float(GraphicsConfig::kClientHeight));

	projectionMatrix_ = MakePerspectiveFovMatrix(fov, aspectRatio, nearClip, farClip);
	spriteProjectionMatrix_ = MakeOrthograpicMatrix(
		0.0f, 0.0f,
		float(GraphicsConfig::kClientWidth),
		float(GraphicsConfig::kClientHeight),
		0.0f, 1000.0f
	);
}