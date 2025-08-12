#pragma once
#include <memory>
#include "RailTrack.h"
#include "Objects/GameObject/GameObject.h"
#include "Objects/Line/LineRenderer.h"

/// <summary>
/// レールカメラのデバッグ表示クラス
/// </summary>
class RailCameraDebugger {
public:
	/// <summary>
	/// 視錐台の頂点データ
	/// </summary>
	struct ViewFrustum {
		Vector3 nearCorners[4];  // 近クリップ面の4つの角
		Vector3 farCorners[4];   // 遠クリップ面の4つの角
	};

	RailCameraDebugger() = default;
	~RailCameraDebugger() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, RailTrack* track);

	/// <summary>
	/// カメラモデルの更新
	/// </summary>
	void UpdateCameraModel(const Vector3& position, const Vector3& rotation, const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw(const Matrix4x4& viewProjectionMatrix, const Light& directionalLight,
		const Vector3& cameraPosition, const Vector3& cameraRotation);

	/// <summary>
	/// 軌道線分を生成
	/// </summary>
	void GenerateRailTrackLines();

	// 表示制御
	bool IsRailTrackVisible() const { return showRailTrack_; }
	void SetRailTrackVisible(bool visible) { showRailTrack_ = visible; }

	bool IsViewFrustumVisible() const { return showViewFrustum_; }
	void SetViewFrustumVisible(bool visible) { showViewFrustum_ = visible; }

	bool IsControlPointsVisible() const { return showControlPoints_; }
	void SetControlPointsVisible(bool visible) { showControlPoints_ = visible; }

	// 軌道描画設定
	void SetRailTrackColor(const Vector4& color) { railTrackColor_ = color; }
	Vector4 GetRailTrackColor() const { return railTrackColor_; }

	void SetRailTrackSegments(int segments) { railTrackSegments_ = segments; }
	int GetRailTrackSegments() const { return railTrackSegments_; }

	// 制御点描画設定
	void SetControlPointColor(const Vector4& color) { controlPointColor_ = color; }
	Vector4 GetControlPointColor() const { return controlPointColor_; }

	void SetSelectedPointColor(const Vector4& color) { selectedPointColor_ = color; }
	Vector4 GetSelectedPointColor() const { return selectedPointColor_; }

	void SetControlPointSize(float size) { controlPointSize_ = size; }
	float GetControlPointSize() const { return controlPointSize_; }

	void SetSelectedPointIndex(int index) { selectedPointIndex_ = index; }
	int GetSelectedPointIndex() const { return selectedPointIndex_; }

	// 視錐台設定
	void SetViewFrustumColor(const Vector4& color) { viewFrustumColor_ = color; }
	Vector4 GetViewFrustumColor() const { return viewFrustumColor_; }

	void SetViewFrustumDistance(float distance) { viewFrustumDistance_ = distance; }
	float GetViewFrustumDistance() const { return viewFrustumDistance_; }

private:
	/// <summary>
	/// 制御点を描画
	/// </summary>
	void DrawControlPoints();

	/// <summary>
	/// 視錐台線分を生成
	/// </summary>
	void GenerateViewFrustumLines(const Vector3& cameraPos, const Vector3& cameraRot);

	/// <summary>
	/// 視錐台を計算
	/// </summary>
	ViewFrustum CalculateViewFrustum(const Vector3& cameraPos, const Vector3& cameraRot, float distance) const;

	// システム参照
	DirectXCommon* directXCommon_ = nullptr;
	RailTrack* track_ = nullptr;

	// デバッグ用のカメラモデル
	std::unique_ptr<Model3D> cameraModel_;

	// 線描画システム
	std::unique_ptr<LineRenderer> lineRenderer_;


	bool showRailTrack_ = false;
	bool showControlPoints_ = false;
	bool showViewFrustum_ = false;

	// 軌道描画設定
	Vector4 railTrackColor_ = { 1.0f, 0.0f, 0.0f, 1.0f };
	int railTrackSegments_ = 100;

	// 制御点描画設定
	Vector4 controlPointColor_ = { 0.0f, 1.0f, 0.0f, 1.0f };
	Vector4 selectedPointColor_ = { 1.0f, 1.0f, 0.0f, 1.0f };
	float controlPointSize_ = 0.5f;
	int selectedPointIndex_ = -1;

	// 視錐台設定
	Vector4 viewFrustumColor_ = { 1.0f, 1.0f, 0.0f, 1.0f };
	float viewFrustumDistance_ = 50.0f;
};