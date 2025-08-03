#pragma once
#include <memory>
#include <vector>
#include "CameraController/BaseCamera.h"
#include "RailTrack.h"
#include "RailCameraMovement.h"
#include "RailCameraDebugger.h"
#include "Objects/GameObject/Transform3D.h"
#include "Engine.h"

/// <summary>
/// レールカメラコントローラー
/// 各コンポーネントを統合してカメラ機能を提供
/// </summary>
class RailCamera : public BaseCamera {
public:
	RailCamera();
	~RailCamera() override;

	/// <summary>
	/// カメラの初期化
	/// </summary>
	void Initialize(const Vector3& position, const Vector3& rotation = { 0.0f, 0.0f, 0.0f }) override;

	/// <summary>
	/// カメラの更新
	/// </summary>
	void Update() override;

	/// <summary>
	/// ImGui表示（統合エディタで処理するため最小限）
	/// </summary>
	void ImGui() override;

	// BaseCamera interface
	Matrix4x4 GetViewProjectionMatrix() const override;
	Matrix4x4 GetSpriteViewProjectionMatrix() const override;
	Vector3 GetPosition() const override { return transform_.GetPosition(); }
	void SetPosition(const Vector3& position) override { transform_.SetPosition(position); }
	Vector3 GetRotation() const override { return transform_.GetRotation(); }
	std::string GetCameraType() const override { return "Rail"; }

	// Transform
	Transform3D& GetTransform() { return transform_; }
	const Transform3D& GetTransform() const { return transform_; }

	/// <summary>
	/// レールカメラの軌道描画
	/// </summary>
	void DrawRailTrack(const Matrix4x4& viewProjectionMatrix, const Light& directionalLight);

	/// <summary>
	/// リソースの明示的解放
	/// </summary>
	void ReleaseResources();

	/// <summary>
	/// 制御点を設定
	/// </summary>
	void SetControlPoints(const std::vector<Vector3>& controlPoints);

	/// <summary>
	/// 現在の進行方向ベクトルを取得
	/// </summary>
	Vector3 GetForwardDirection() const;

	// コンポーネントへのアクセス
	RailTrack* GetTrack() { return track_.get(); }
	RailCameraMovement* GetMovement() { return movement_.get(); }
	RailCameraDebugger* GetDebugger() { return debugger_.get(); }

	// 移動制御のショートカット
	void StartMovement() { if (movement_) movement_->StartMovement(); }
	void StopMovement() { if (movement_) movement_->StopMovement(); }
	void ResetPosition() { if (movement_) movement_->ResetPosition(); }
	bool IsMoving() const { return movement_ ? movement_->IsMoving() : false; }

	float GetProgress() const { return movement_ ? movement_->GetProgress() : 0.0f; }
	void SetProgress(float progress) { if (movement_) movement_->SetProgress(progress); }

	float GetSpeed() const { return movement_ ? movement_->GetSpeed() : 0.0f; }
	void SetSpeed(float speed) { if (movement_) movement_->SetSpeed(speed); }

	bool IsLoopEnabled() const { return movement_ ? movement_->IsLoopEnabled() : false; }
	void SetLoopEnabled(bool enabled) { if (movement_) movement_->SetLoopEnabled(enabled); }

	// デバッグ表示のショートカット
	void SetViewFrustumVisible(bool visible) { if (debugger_) debugger_->SetViewFrustumVisible(visible); }
	bool IsViewFrustumVisible() const { return debugger_ ? debugger_->IsViewFrustumVisible() : false; }

	void SetSelectedPointIndex(int index) { if (debugger_) debugger_->SetSelectedPointIndex(index); }
	void GenerateRailTrackLines() { if (debugger_) debugger_->GenerateRailTrackLines(); }

	// フレーム計算のショートカット
	int GetCurrentFrameFromStart() const { return movement_ ? movement_->GetCurrentFrameFromStart() : 0; }
	int GetMaxFrames() const { return movement_ ? movement_->GetMaxFrames() : 0; }
	void SetProgressFromFrame(int frame) { if (movement_) movement_->SetProgressFromFrame(frame); }

	// 等間隔移動のショートカット
	bool IsUniformSpeedEnabled() const {
		return movement_ ? movement_->IsUniformSpeedEnabled() : false;
	}
	void SetUniformSpeedEnabled(bool enabled) {
		if (movement_) movement_->SetUniformSpeedEnabled(enabled);
	}

	// 視錐台色のショートカット
	Vector4 GetViewFrustumColor() const {
		return debugger_ ? debugger_->GetViewFrustumColor() : Vector4{ 1.0f, 1.0f, 0.0f, 1.0f };
	}
	void SetViewFrustumColor(const Vector4& color) {
		if (debugger_) debugger_->SetViewFrustumColor(color);
	}

	// 視錐台距離のショートカット
	float GetViewFrustumDistance() const {
		return debugger_ ? debugger_->GetViewFrustumDistance() : 50.0f;
	}
	void SetViewFrustumDistance(float distance) {
		if (debugger_) debugger_->SetViewFrustumDistance(distance);
	}
private:
	/// <summary>
	/// カメラの位置と回転を更新
	/// </summary>
	void UpdateCameraTransform();

	/// <summary>
	/// デフォルトカメラ設定
	/// </summary>
	void SetDefaultCamera(const Vector3& position, const Vector3& rotation);

	// コンポーネント
	std::unique_ptr<RailTrack> track_;
	std::unique_ptr<RailCameraMovement> movement_;
	std::unique_ptr<RailCameraDebugger> debugger_;

	// トランスフォーム
	Transform3D transform_;

	// カメラパラメータ
	Matrix4x4 projectionMatrix_;
	Matrix4x4 spriteProjectionMatrix_;

	// 初期値保存用
	Vector3 initialPosition_;
	Vector3 initialRotation_;

	// 最後の有効な回転（終端処理用）
	Vector3 lastValidRotation_ = { 0.0f, 0.0f, 0.0f };

	// システム参照
	DirectXCommon* directXCommon_ = nullptr;
};