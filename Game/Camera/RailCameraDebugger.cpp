#include "RailCameraDebugger.h"
#include "BaseSystem/GraphicsConfig.h"

void RailCameraDebugger::Initialize(DirectXCommon* dxCommon, RailTrack* track) {
	directXCommon_ = dxCommon;
	track_ = track;

	cameraModel_ = std::make_unique<Model3D>();
	cameraModel_->Initialize(dxCommon, "camera");
	cameraModel_->SetName("RailCamera");

	lineRenderer_ = std::make_unique<LineRenderer>();
	lineRenderer_->Initialize(dxCommon);
}

void RailCameraDebugger::UpdateCameraModel(const Vector3& position, const Vector3& rotation, const Matrix4x4& viewProjectionMatrix) {
	if (cameraModel_) {
		cameraModel_->SetPosition(position);
		cameraModel_->SetRotation(rotation);
		cameraModel_->Update(viewProjectionMatrix);
	}
}

void RailCameraDebugger::Draw(const Matrix4x4& viewProjectionMatrix, const Light& directionalLight,
	const Vector3& cameraPosition, const Vector3& cameraRotation) {
	if (cameraModel_) {
		cameraModel_->Draw(directionalLight);
	}

	if (lineRenderer_ && showRailTrack_) {
		// 視錐台表示が有効な場合は、毎フレーム完全に再生成
		if (showViewFrustum_) {
			lineRenderer_->Reset();
			GenerateRailTrackLines();
			GenerateViewFrustumLines(cameraPosition, cameraRotation);
		}

		lineRenderer_->Draw(viewProjectionMatrix);
	}
}

void RailCameraDebugger::GenerateRailTrackLines() {
	if (!lineRenderer_ || !track_ || !track_->IsValid()) {
		return;
	}

	// 軌道の線分を生成
	std::vector<Vector3> pointsDrawing;
	for (int i = 0; i < railTrackSegments_ + 1; i++) {
		float t = 1.0f / railTrackSegments_ * i;
		Vector3 point = track_->CalculatePosition(t);
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

void RailCameraDebugger::DrawControlPoints() {
	if (!lineRenderer_ || !track_) {
		return;
	}

	const auto& controlPoints = track_->GetControlPoints();

	for (int i = 0; i < static_cast<int>(controlPoints.size()); ++i) {
		const Vector3& point = controlPoints[i];

		Vector4 pointColor = (i == selectedPointIndex_) ? selectedPointColor_ : controlPointColor_;

		Vector3 offset1 = { controlPointSize_, 0.0f, 0.0f };
		Vector3 offset2 = { 0.0f, controlPointSize_, 0.0f };
		Vector3 offset3 = { 0.0f, 0.0f, controlPointSize_ };

		lineRenderer_->AddLine(point - offset1, point + offset1, pointColor);
		lineRenderer_->AddLine(point - offset2, point + offset2, pointColor);
		lineRenderer_->AddLine(point - offset3, point + offset3, pointColor);
	}
}

void RailCameraDebugger::GenerateViewFrustumLines(const Vector3& cameraPos, const Vector3& cameraRot) {
	if (!lineRenderer_ || !showViewFrustum_) {
		return;
	}

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

RailCameraDebugger::ViewFrustum RailCameraDebugger::CalculateViewFrustum(const Vector3& cameraPos, const Vector3& cameraRot, float distance) const {
	ViewFrustum frustum;

	float fov = 0.45f;
	float aspectRatio = (float(GraphicsConfig::kClientWidth) / float(GraphicsConfig::kClientHeight));
	float nearClip = 0.1f;
	float farClip = std::min(distance, 1000.0f);

	Matrix4x4 rotMatrix = MakeRotateXYZMatrix(cameraRot);

	Vector3 forward = { 0.0f, 0.0f, 1.0f };
	Vector3 right = { 1.0f, 0.0f, 0.0f };
	Vector3 up = { 0.0f, 1.0f, 0.0f };

	forward = TransformNormal(forward, rotMatrix);
	right = TransformNormal(right, rotMatrix);
	up = TransformNormal(up, rotMatrix);

	float halfFovY = fov * 0.5f;
	float halfFovX = halfFovY * aspectRatio;

	float nearHeight = 2.0f * nearClip * std::tan(halfFovY);
	float nearWidth = nearHeight * aspectRatio;

	float farHeight = 2.0f * farClip * std::tan(halfFovY);
	float farWidth = farHeight * aspectRatio;

	Vector3 nearCenter = cameraPos + forward * nearClip;
	Vector3 farCenter = cameraPos + forward * farClip;

	Vector3 nearRightUp = right * (nearWidth * 0.5f) + up * (nearHeight * 0.5f);
	Vector3 nearRightDown = right * (nearWidth * 0.5f) - up * (nearHeight * 0.5f);
	Vector3 nearLeftUp = -right * (nearWidth * 0.5f) + up * (nearHeight * 0.5f);
	Vector3 nearLeftDown = -right * (nearWidth * 0.5f) - up * (nearHeight * 0.5f);

	frustum.nearCorners[0] = nearCenter + nearRightUp;
	frustum.nearCorners[1] = nearCenter + nearRightDown;
	frustum.nearCorners[2] = nearCenter + nearLeftDown;
	frustum.nearCorners[3] = nearCenter + nearLeftUp;

	Vector3 farRightUp = right * (farWidth * 0.5f) + up * (farHeight * 0.5f);
	Vector3 farRightDown = right * (farWidth * 0.5f) - up * (farHeight * 0.5f);
	Vector3 farLeftUp = -right * (farWidth * 0.5f) + up * (farHeight * 0.5f);
	Vector3 farLeftDown = -right * (farWidth * 0.5f) - up * (farHeight * 0.5f);

	frustum.farCorners[0] = farCenter + farRightUp;
	frustum.farCorners[1] = farCenter + farRightDown;
	frustum.farCorners[2] = farCenter + farLeftDown;
	frustum.farCorners[3] = farCenter + farLeftUp;

	return frustum;
}