#include "RailCamera.h"
#include "Managers/ImGui/ImGuiManager.h"

RailCamera::RailCamera()
	: t_(0.0f)
	, speed_(0.001f)
	, isMoving_(true)
	, loopEnabled_(true)
	, lookAheadDistance_(0.005f)
	, uniformSpeedEnabled_(true)
	, totalLength_(0.0f)
	, lengthTableResolution_(1000)
	, showRailTrack_(true)
	, railTrackColor_({ 1.0f, 0.0f, 0.0f, 1.0f })
	, railTrackSegments_(100)
	, showControlPoints_(true)
	, controlPointColor_({ 0.0f, 1.0f, 0.0f, 1.0f })  // 通常は緑色
	, selectedPointColor_({ 1.0f, 1.0f, 0.0f, 1.0f })  // 選択時は黄色
	, controlPointSize_(0.5f)
	, selectedPointIndex_(-1)
	, directXCommon_(nullptr)
	, showViewFrustum_(false)                           // 視錐台表示フラグ
	, viewFrustumColor_({ 1.0f, 1.0f, 0.0f, 1.0f })   // 視錐台の色（黄色）
	, viewFrustumDistance_(50.0f)                       // 視錐台描画距離
	, debugFrameInput_(0)                               // デバッグ用フレーム入力
{
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

	// レール移動パラメータの初期化
	t_ = 0.0f;
	speed_ = 0.00025f;
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
		t_ += speed_;
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

Vector3 RailCamera::GetForwardDirection() const {
	if (controlPoints_.size() < 4) {
		return Vector3{ 0.0f, 0.0f, 1.0f }; // デフォルト方向
	}

	// 現在の位置から少し先の位置を計算して進行方向を求める
	float lookAheadT = t_ + 0.001f; // 小さな値で先読み
	if (lookAheadT > 1.0f) {
		lookAheadT = 1.0f;
	}

	Vector3 currentPos = CatmullRomPosition(controlPoints_, t_);
	Vector3 futurePos = CatmullRomPosition(controlPoints_, lookAheadT);

	Vector3 forwardDirection = futurePos - currentPos;

	// 方向ベクトルが極小の場合はデフォルト方向を返す
	if (Length(forwardDirection) < 0.001f) {
		return Vector3{ 0.0f, 0.0f, 1.0f };
	}

	return Normalize(forwardDirection);
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

	// 注意：視錐台はここでは描画しない（動的に更新するため）
}

void RailCamera::GenerateViewFrustumLines() {
	if (!lineRenderer_ || !showViewFrustum_) {
		return;
	}

	Vector3 cameraPos = transform_.GetPosition();
	Vector3 cameraRot = transform_.GetRotation();

	ViewFrustum frustum = CalculateViewFrustum(cameraPos, cameraRot, viewFrustumDistance_);

	// Near面の線分
	for (int i = 0; i < 4; ++i) {
		int nextIndex = (i + 1) % 4;
		lineRenderer_->AddLine(frustum.nearCorners[i], frustum.nearCorners[nextIndex], viewFrustumColor_);
	}

	// Far面の線分
	for (int i = 0; i < 4; ++i) {
		int nextIndex = (i + 1) % 4;
		lineRenderer_->AddLine(frustum.farCorners[i], frustum.farCorners[nextIndex], viewFrustumColor_);
	}

	// Near面とFar面を結ぶ線分
	for (int i = 0; i < 4; ++i) {
		lineRenderer_->AddLine(frustum.nearCorners[i], frustum.farCorners[i], viewFrustumColor_);
	}
}

void RailCamera::DrawControlPoints() {
	if (!lineRenderer_) {
		return;
	}

	// 制御点を小さな十字で表示
	for (int i = 0; i < static_cast<int>(controlPoints_.size()); ++i) {
		const Vector3& point = controlPoints_[i];

		// 選択されたポイントかどうかで色を変える
		Vector4 pointColor = (i == selectedPointIndex_) ? selectedPointColor_ : controlPointColor_;

		Vector3 offset1 = { controlPointSize_, 0.0f, 0.0f };
		Vector3 offset2 = { 0.0f, controlPointSize_, 0.0f };
		Vector3 offset3 = { 0.0f, 0.0f, controlPointSize_ };

		// X軸方向の線
		lineRenderer_->AddLine(point - offset1, point + offset1, pointColor);
		// Y軸方向の線
		lineRenderer_->AddLine(point - offset2, point + offset2, pointColor);
		// Z軸方向の線
		lineRenderer_->AddLine(point - offset3, point + offset3, pointColor);
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

// ============================ デバッグ機能の実装 ============================

int RailCamera::GetCurrentFrameFromStart() const {
	if (speed_ <= 0.0f) {
		return 0;
	}

	if (uniformSpeedEnabled_ && totalLength_ > 0.0f) {
		// 等間隔移動の場合：距離ベースで計算
		float currentDistance = t_ * totalLength_;
		float distancePerFrame = speed_ * totalLength_;
		return static_cast<int>(currentDistance / distancePerFrame);
	} else {
		// 従来の移動：tパラメータベース
		return static_cast<int>(t_ / speed_);
	}
}

float RailCamera::GetProgressFromFrame(int frame) const {
	if (frame <= 0 || speed_ <= 0.0f) {
		return 0.0f;
	}

	if (uniformSpeedEnabled_ && totalLength_ > 0.0f) {
		// 等間隔移動の場合：距離ベースで計算
		float distancePerFrame = speed_ * totalLength_;
		float targetDistance = frame * distancePerFrame;
		return std::clamp(targetDistance / totalLength_, 0.0f, 1.0f);
	} else {
		// 従来の移動：tパラメータベース
		return std::clamp(frame * speed_, 0.0f, 1.0f);
	}
}

void RailCamera::SetProgressFromFrame(int frame) {
	float newProgress = GetProgressFromFrame(frame);
	SetProgress(newProgress);
}

int RailCamera::GetMaxFrames() const {
	if (speed_ <= 0.0f) {
		return 0;
	}

	if (uniformSpeedEnabled_ && totalLength_ > 0.0f) {
		// 等間隔移動の場合：距離ベースで計算
		float distancePerFrame = speed_ * totalLength_;
		return static_cast<int>(totalLength_ / distancePerFrame);
	} else {
		// 従来の移動：tパラメータベース
		return static_cast<int>(1.0f / speed_);
	}
}

RailCamera::ViewFrustum RailCamera::CalculateViewFrustum(const Vector3& cameraPos, const Vector3& cameraRot, float distance) const {
	ViewFrustum frustum;

	// プロジェクション行列からカメラパラメータを逆算
	// 簡易的にデフォルト値を使用（より正確にはprojectionMatrix_から計算）
	float fov = 0.45f;  // SetDefaultCameraで設定されているfov
	float aspectRatio = (float(GraphicsConfig::kClientWidth) / float(GraphicsConfig::kClientHeight));
	float nearClip = 0.1f;
	float farClip = std::min(distance, 1000.0f);

	// カメラの回転行列を計算
	Matrix4x4 rotMatrix = MakeRotateXYZMatrix(cameraRot);

	// カメラの前方向、右方向、上方向を計算
	Vector3 forward = { 0.0f, 0.0f, 1.0f };
	Vector3 right = { 1.0f, 0.0f, 0.0f };
	Vector3 up = { 0.0f, 1.0f, 0.0f };

	forward = TransformNormal(forward, rotMatrix);
	right = TransformNormal(right, rotMatrix);
	up = TransformNormal(up, rotMatrix);

	// FOVから視錐台の幅と高さを計算
	float halfFovY = fov * 0.5f;
	float halfFovX = halfFovY * aspectRatio;

	// Near面の幅と高さ
	float nearHeight = 2.0f * nearClip * std::tan(halfFovY);
	float nearWidth = nearHeight * aspectRatio;

	// Far面の幅と高さ
	float farHeight = 2.0f * farClip * std::tan(halfFovY);
	float farWidth = farHeight * aspectRatio;

	// Near面の中心
	Vector3 nearCenter = cameraPos + forward * nearClip;

	// Far面の中心
	Vector3 farCenter = cameraPos + forward * farClip;

	// Near面の4つの角
	Vector3 nearRightUp = right * (nearWidth * 0.5f) + up * (nearHeight * 0.5f);
	Vector3 nearRightDown = right * (nearWidth * 0.5f) - up * (nearHeight * 0.5f);
	Vector3 nearLeftUp = -right * (nearWidth * 0.5f) + up * (nearHeight * 0.5f);
	Vector3 nearLeftDown = -right * (nearWidth * 0.5f) - up * (nearHeight * 0.5f);

	frustum.nearCorners[0] = nearCenter + nearRightUp;   // 右上
	frustum.nearCorners[1] = nearCenter + nearRightDown; // 右下
	frustum.nearCorners[2] = nearCenter + nearLeftDown;  // 左下
	frustum.nearCorners[3] = nearCenter + nearLeftUp;    // 左上

	// Far面の4つの角
	Vector3 farRightUp = right * (farWidth * 0.5f) + up * (farHeight * 0.5f);
	Vector3 farRightDown = right * (farWidth * 0.5f) - up * (farHeight * 0.5f);
	Vector3 farLeftUp = -right * (farWidth * 0.5f) + up * (farHeight * 0.5f);
	Vector3 farLeftDown = -right * (farWidth * 0.5f) - up * (farHeight * 0.5f);

	frustum.farCorners[0] = farCenter + farRightUp;   // 右上
	frustum.farCorners[1] = farCenter + farRightDown; // 右下
	frustum.farCorners[2] = farCenter + farLeftDown;  // 左下
	frustum.farCorners[3] = farCenter + farLeftUp;    // 左上

	return frustum;
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
	ImGui::DragFloat("Speed", &speed_, (1.0f / 6000.0f), 0.0001f, 0.0001f);
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

	// ====================== デバッグ機能セクション ======================
	ImGui::Separator();
	ImGui::Text("Debug Features:");

	// フレーム計算機能
	ImGui::Text("Frame Analysis:");
	int currentFrame = GetCurrentFrameFromStart();
	int maxFrames = GetMaxFrames();
	ImGui::Text("Current Frame: %d / %d", currentFrame, maxFrames);
	ImGui::Text("Progress: %.3f (%.1f%%)", t_, t_ * 100.0f);

	// フレーム数入力
	ImGui::PushItemWidth(100);
	if (ImGui::InputInt("Jump to Frame", &debugFrameInput_)) {
		debugFrameInput_ = std::clamp(debugFrameInput_, 0, maxFrames);
	}
	ImGui::PopItemWidth();

	ImGui::SameLine();
	if (ImGui::Button("Jump")) {
		SetProgressFromFrame(debugFrameInput_);
		// 移動を停止してデバッグ操作しやすくする
		StopMovement();
	}

	ImGui::SameLine();
	if (ImGui::Button("Current->Input")) {
		debugFrameInput_ = currentFrame;
	}

	// 進行度スライダー（より細かい制御用）
	float progressSlider = t_;
	if (ImGui::SliderFloat("Precise Progress", &progressSlider, 0.0f, 1.0f, "%.4f")) {
		SetProgress(progressSlider);
		StopMovement();
	}

	// 視錐台表示設定
	ImGui::Separator();
	ImGui::Text("View Frustum Visualization:");
	ImGui::Checkbox("Show View Frustum", &showViewFrustum_);
	// 注意：視錐台は動的に更新されるため、設定変更時の再生成は不要

	if (showViewFrustum_) {
		ImGui::ColorEdit4("Frustum Color", &viewFrustumColor_.x);
		ImGui::DragFloat("Frustum Distance", &viewFrustumDistance_, 1.0f, 5.0f, 200.0f);
		// 注意：これらの設定も動的に反映されるため、即座の再生成は不要
	}

	// 進行方向表示
	ImGui::Separator();
	ImGui::Text("Direction Info:");
	Vector3 forwardDir = GetForwardDirection();
	ImGui::Text("Forward Direction: (%.3f, %.3f, %.3f)", forwardDir.x, forwardDir.y, forwardDir.z);

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

	if (ImGui::ColorEdit4("Selected Point Color", &selectedPointColor_.x)) {
		trackSettingsChanged = true;
	}

	if (ImGui::DragFloat("Control Point Size", &controlPointSize_, 0.01f, 0.1f, 2.0f)) {
		trackSettingsChanged = true;
	}

	// 選択情報表示
	if (selectedPointIndex_ >= 0) {
		ImGui::Text("Selected Point: %d", selectedPointIndex_);
	} else {
		ImGui::Text("Selected Point: None");
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
		// 視錐台表示が有効な場合は、毎フレーム完全に再生成
		if (showViewFrustum_) {
			// 一度リセットして静的な線分を再生成
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

			// 現在のカメラ位置での視錐台を動的に追加
			GenerateViewFrustumLines();
		}

		// 描画実行
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