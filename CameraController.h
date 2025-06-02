#pragma once

#include "DebugCamera.h"
#include "Camera.h"

class CameraController
{
public:
	// シングルトンパターン
	static CameraController* GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();


	//Setter

	void SetTransform(const Vector3& newTransform) { cameraTranslation_ = newTransform; }

	//Getter

	Matrix4x4 GetViewProjectionMatrix() const { return viewProjectionMatrix_; }
	Matrix4x4 GetViewProjectionMatrixSprite() const { return viewProjectionMatrixSprite_; }

private:
	// シングルトン用
	CameraController() = default;
	~CameraController() = default;
	CameraController(const CameraController&) = delete;
	CameraController& operator=(const CameraController&) = delete;


	/// <summary>
	/// imGui
	/// </summary>
	void ImGui();


	Camera camera_;
	DebugCamera debugCamera_;

	// カメラの初期座標
	Vector3 cameraTranslation_;

	Matrix4x4 viewProjectionMatrix_;
	Matrix4x4 viewProjectionMatrixSprite_;

	// デバッグカメラを使用するかどうか
	bool useDebugCamera_;



};

