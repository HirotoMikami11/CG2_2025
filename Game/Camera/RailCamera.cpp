#include "RailCamera.h"
#include "Managers/ImGui/ImGuiManager.h"

RailCamera::RailCamera()
	: t_(0.0f)
	, speed_(0.001f)
	, isMoving_(true)
	, loopEnabled_(true)
	, lookAheadDistance_(0.01f)
	, uniformSpeedEnabled_(true)
	, totalLength_(0.0f)
	, lengthTableResolution_(1000)
	, showRailTrack_(true)
	, railTrackColor_({ 1.0f, 0.0f, 0.0f, 1.0f })
	, railTrackSegments_(100)
	, showControlPoints_(true)
	, controlPointColor_({ 0.0f, 1.0f, 0.0f, 1.0f })
	, controlPointSize_(0.5f)
	, directXCommon_(nullptr) {
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

	cameraModel_ = std::make_unique<Model3D>();
	cameraModel_->Initialize(directXCommon_, "camera");
	cameraModel_->SetName("RailCamera");

	lineRenderer_ = std::make_unique<LineRenderer>();
	lineRenderer_->Initialize(directXCommon_);

	// デフォルトの制御点を設定
	controlPoints_ = {
		{0,  0,   0 },
		{10, 10,  0 },
		{10, 15,  0 },
		{20, 15,  0 },
		{20, 0,   0 },
		{20, 0,   10},
		{30, -10, 5 },
		{20, -10, 0 },
		{20, -15, 5 },
		{10, -15, 0 },
		{10, -10, 0 },
		{0,  0,   0 },
	};

	// レール移動パラメータの初期化
	t_ = 0.0f;
	speed_ = 0.001f;
	isMoving_ = true;
	loopEnabled_ = true;
	lookAheadDistance_ = 0.01f;
	uniformSpeedEnabled_ = true;

	// 長さテーブルを構築
	BuildLengthTable();

	// 軌道の線分を生成
	GenerateRailTrackLines();
}

void RailCamera::Update() {
	if (isMoving_) {
		UpdateCameraPosition();
	}

	Matrix4x4 identityMatrix = MakeIdentity4x4();
	transform_.UpdateMatrix(identityMatrix);
}

void RailCamera::UpdateCameraPosition() {
	if (controlPoints_.size() < 4) {
		return;
	}

	if (uniformSpeedEnabled_ && !lengthTable_.empty()) {

		// 等間隔移動：距離ベースでの移動
		float currentLength = t_ * totalLength_;
		currentLength += speed_ * totalLength_; // 速度を距離に変換

		// ループ処理
		if (currentLength >= totalLength_) {
			if (loopEnabled_) {
				currentLength = 0.0f;  // ループ有効時のみリセット
			} else {
				currentLength = totalLength_;  // ループ無効時は終端で停止
				isMoving_ = false;  // 移動を停止
			}
		}
		
		//距離からtパラメータを取得
		float newT = GetTFromLength(currentLength);
		t_ = newT;

		// 進行度を更新
		t_ = currentLength / totalLength_;
	} else {

		// 従来の移動：tパラメータベース
		if (t_ >= 1.0f) {
			if (loopEnabled_) {
				t_ = 0.0f;  // ループ有効時のみリセット
			} else {
				t_ = 1.0f;  // ループ無効時は終端で停止
				isMoving_ = false;  // 移動を停止
			}
		}
	}

	// 現在の位置を計算
	Vector3 currentPosition = CatmullRomPosition(controlPoints_, t_);

	// 注視点を計算
	Vector3 lookAtTarget = CalculateLookAtTarget(t_);

	// カメラの位置を設定
	transform_.SetPosition(currentPosition);

	// 視点から注視点への方向ベクトルを計算
	Vector3 forward = lookAtTarget - currentPosition;
	forward = Normalize(forward);

	// 方向ベクトルから回転角を計算
	Vector3 rotation = transform_.GetRotation();
	rotation.y = std::atan2(forward.x, forward.z);
	float horizontalDistance = std::sqrt(forward.x * forward.x + forward.z * forward.z);
	rotation.x = -std::atan2(forward.y, horizontalDistance);
	rotation.z = 0.0f;

	transform_.SetRotation(rotation);
}

void RailCamera::BuildLengthTable() {
	lengthTable_.clear();

	if (controlPoints_.size() < 4) {
		totalLength_ = 0.0f;
		return;
	}

	lengthTable_.reserve(lengthTableResolution_ + 1);

	float totalLength = 0.0f;
	Vector3 previousPos = CatmullRomPosition(controlPoints_, 0.0f);

	// 最初のエントリ
	lengthTable_.push_back({ 0.0f, 0.0f, 0.0f });

	// 曲線を細かく分割して長さを計算
	for (int i = 1; i <= lengthTableResolution_; ++i) {
		float t = static_cast<float>(i) / static_cast<float>(lengthTableResolution_);
		Vector3 currentPos = CatmullRomPosition(controlPoints_, t);

		float segmentLength = Length(currentPos - previousPos);
		totalLength += segmentLength;

		lengthTable_.push_back({ t, totalLength, segmentLength });

		previousPos = currentPos;
	}

	totalLength_ = totalLength;

}

float RailCamera::GetTFromLength(float targetLength) const {
	if (lengthTable_.empty() || targetLength <= 0.0f) {
		return 0.0f;
	}

	if (targetLength >= totalLength_) {
		return 1.0f;
	}

	// バイナリサーチで対応するエントリを見つける
	auto it = std::lower_bound(lengthTable_.begin(), lengthTable_.end(), targetLength,
		[](const LengthTableEntry& entry, float length) {
			return entry.length < length;
		});

	if (it == lengthTable_.end()) {
		return 1.0f;
	}

	if (it == lengthTable_.begin()) {
		return 0.0f;
	}

	// 線形補間
	auto prevIt = it - 1;

	float ratio = (targetLength - prevIt->length) / (it->length - prevIt->length);
	return prevIt->t + ratio * (it->t - prevIt->t);
}

float RailCamera::CalculateSegmentLength(float t1, float t2, int subdivisions) const {
	if (controlPoints_.size() < 4) {
		return 0.0f;
	}

	float totalLength = 0.0f;
	Vector3 previousPos = CatmullRomPosition(controlPoints_, t1);

	float dt = (t2 - t1) / static_cast<float>(subdivisions);

	for (int i = 1; i <= subdivisions; ++i) {
		float t = t1 + dt * static_cast<float>(i);
		Vector3 currentPos = CatmullRomPosition(controlPoints_, t);

		totalLength += Length(currentPos - previousPos);
		previousPos = currentPos;
	}

	return totalLength;
}

void RailCamera::SetControlPoints(const std::vector<Vector3>& controlPoints) {
	controlPoints_ = controlPoints;

	// 長さテーブルを再構築
	if (uniformSpeedEnabled_) {
		BuildLengthTable();
	}

	// 軌道の線分を再生成
	GenerateRailTrackLines();

}

void RailCamera::GenerateRailTrackLines() {
	if (!lineRenderer_ || controlPoints_.size() < 4) {
		return;
	}

	lineRenderer_->Reset();

	// 軌道の線分を生成
	std::vector<Vector3> pointsDrawing;
	for (int i = 0; i < railTrackSegments_ + 1; i++) {
		float t = 1.0f / railTrackSegments_ * i;
		Vector3 point = CatmullRomPosition(controlPoints_, t);
		pointsDrawing.push_back(point);
	}

	// 隣接する2点を結ぶ線分を追加
	for (size_t i = 0; i < pointsDrawing.size() - 1; ++i) {
		lineRenderer_->AddLine(pointsDrawing[i], pointsDrawing[i + 1], railTrackColor_);
	}

	// 制御点を描画
	if (showControlPoints_) {
		DrawControlPoints();
	}
}

void RailCamera::DrawControlPoints() {
	if (!lineRenderer_) {
		return;
	}

	// 制御点を小さな十字で表示
	for (const auto& point : controlPoints_) {
		Vector3 offset1 = { controlPointSize_, 0.0f, 0.0f };
		Vector3 offset2 = { 0.0f, controlPointSize_, 0.0f };
		Vector3 offset3 = { 0.0f, 0.0f, controlPointSize_ };

		// X軸方向の線
		lineRenderer_->AddLine(point - offset1, point + offset1, controlPointColor_);
		// Y軸方向の線
		lineRenderer_->AddLine(point - offset2, point + offset2, controlPointColor_);
		// Z軸方向の線
		lineRenderer_->AddLine(point - offset3, point + offset3, controlPointColor_);
	}
}

void RailCamera::UpdateCameraModel() {
	if (cameraModel_) {
		cameraModel_->SetPosition(transform_.GetPosition());
		cameraModel_->SetRotation(transform_.GetRotation());

		CameraController* cameraController = CameraController::GetInstance();
		Matrix4x4 viewProjectionMatrix = cameraController->GetViewProjectionMatrix();
		cameraModel_->Update(viewProjectionMatrix);
	}
}

Vector3 RailCamera::CalculateLookAtTarget(float currentT) {
	float lookAheadT = currentT + lookAheadDistance_;

	if (uniformSpeedEnabled_ && !lengthTable_.empty()) {
		// 等間隔移動の場合は距離ベースで先読み
		float currentLength = currentT * totalLength_;
		float lookAheadLength = currentLength + (lookAheadDistance_ * totalLength_);

		if (lookAheadLength >= totalLength_) {
			lookAheadLength = totalLength_;
		}

		lookAheadT = GetTFromLength(lookAheadLength);
	} else {
		// 従来の方法
		if (lookAheadT >= 1.0f) {
			lookAheadT = 1.0f;
		}
	}

	return CatmullRomPosition(controlPoints_, lookAheadT);
}

void RailCamera::ImGui() {
#ifdef _DEBUG
	ImGui::Text("RailCamera");
	ImGui::Separator();

	Vector3 position = transform_.GetPosition();
	Vector3 rotation = transform_.GetRotation();

	if (ImGui::DragFloat3("Position", &position.x, 0.01f)) {
		transform_.SetPosition(position);
	}
	if (ImGui::DragFloat3("Rotation", &rotation.x, 0.01f)) {
		transform_.SetRotation(rotation);
	}
	ImGui::Separator();

	// レール移動制御
	ImGui::Text("Rail Movement:");
	ImGui::Checkbox("Moving", &isMoving_);
	ImGui::SameLine();
	if (ImGui::Button("Start")) {
		StartMovement();
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop")) {
		StopMovement();
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset")) {
		ResetPosition();
	}

	ImGui::Checkbox("Loop Movement", &loopEnabled_);
	ImGui::DragFloat("Speed", &speed_, 0.0001f, 0.0001f, 0.01f);
	ImGui::DragFloat("Look Ahead Distance", &lookAheadDistance_, 0.001f);

	// 等間隔移動設定
	ImGui::Separator();
	ImGui::Text("Uniform Speed:");
	if (ImGui::Checkbox("Enable Uniform Speed", &uniformSpeedEnabled_)) {
		if (uniformSpeedEnabled_) {
			BuildLengthTable();
		}
	}

	if (uniformSpeedEnabled_) {
		ImGui::Text("Total Length: %.2f", totalLength_);
		ImGui::Text("Current Progress: %.1f%%", t_ * 100.0f);

		if (ImGui::DragInt("Length Table Resolution", &lengthTableResolution_, 10, 100, 2000)) {
			BuildLengthTable();
		}

		if (ImGui::Button("Rebuild Length Table")) {
			BuildLengthTable();
		}
	}

	ImGui::Separator();

	// 軌道描画設定
	ImGui::Text("Rail Track Display:");
	bool trackSettingsChanged = false;

	if (ImGui::Checkbox("Show Rail Track", &showRailTrack_)) {
		trackSettingsChanged = true;
	}

	if (ImGui::ColorEdit4("Track Color", &railTrackColor_.x)) {
		trackSettingsChanged = true;
	}

	if (ImGui::DragInt("Track Segments", &railTrackSegments_, 1, 10, 500)) {
		trackSettingsChanged = true;
	}

	// 制御点描画設定
	ImGui::Separator();
	ImGui::Text("Control Points Display:");
	if (ImGui::Checkbox("Show Control Points", &showControlPoints_)) {
		trackSettingsChanged = true;
	}

	if (ImGui::ColorEdit4("Control Point Color", &controlPointColor_.x)) {
		trackSettingsChanged = true;
	}

	if (ImGui::DragFloat("Control Point Size", &controlPointSize_, 0.01f, 0.1f, 2.0f)) {
		trackSettingsChanged = true;
	}

	// 軌道設定が変更された場合は再生成
	if (trackSettingsChanged) {
		GenerateRailTrackLines();
	}

	// 線描画システムのImGui
	if (lineRenderer_) {
		lineRenderer_->ImGui();
	}

	ImGui::Separator();

	// 制御点情報
	ImGui::Text("Control Points: %zu", controlPoints_.size());
	if (ImGui::Button("Regenerate Track")) {
		GenerateRailTrackLines();
	}

	ImGui::Separator();

	// リセットボタン
	if (ImGui::Button("Reset Camera")) {
		SetDefaultCamera(initialPosition_, initialRotation_);
	}
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

void RailCamera::DrawRailTrack(const Matrix4x4& viewProjectionMatrix, const Light& directionalLight) {
	UpdateCameraModel();

	if (cameraModel_) {
		cameraModel_->Draw(directionalLight);
	}

	if (lineRenderer_ && showRailTrack_) {
		lineRenderer_->Draw(viewProjectionMatrix);
	}
}

void RailCamera::ReleaseResources() {
	if (lineRenderer_) {
		lineRenderer_.reset();
	}

	if (cameraModel_) {
		cameraModel_.reset();
	}

	directXCommon_ = nullptr;

	Logger::Log(Logger::GetStream(), "RailCamera: Resources released\n");
}

void RailCamera::SetDefaultCamera(const Vector3& position, const Vector3& rotation) {
	transform_.SetPosition(position);
	transform_.SetRotation(rotation);
	transform_.SetScale({ 1.0f, 1.0f, 1.0f });

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