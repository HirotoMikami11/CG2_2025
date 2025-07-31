#pragma once
#define NOMINMAX // C++標準のstd::maxを使えるようにするため(windows.hが上書きしてしまっている)
#include <d3d12.h>
#include <wrl.h>
#include <algorithm>

#include "BaseCamera.h"
#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "Managers/Input/inputManager.h"

/// <summary>
/// 球面座標系を表す構造体
/// </summary>
struct SphericalCoordinates {
	float radius;	// ρ=動径	→半径（距離）
	float theta;	// θ=方位角	→方位角（水平方向の角度、ラジアン）
	float phi;		// φ=極角	→仰角（垂直方向の角度、ラジアン）
};

/// <summary>
/// デバッグカメラ
/// </summary>
class DebugCamera : public BaseCamera {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	DebugCamera();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~DebugCamera() override;

	/// <summary>
	/// カメラの初期化
	/// </summary>
	/// <param name="position">初期位置</param>
	/// <param name="rotation">初期回転（デフォルト：{0,0,0}）</param>
	void Initialize(const Vector3& position, const Vector3& rotation = { 0.0f, 0.0f, 0.0f }) override;

	/// <summary>
	/// カメラの更新
	/// </summary>
	void Update() override;


	Matrix4x4 GetViewProjectionMatrix() const override { return viewProjectionMatrix_; }
	Matrix4x4 GetSpriteViewProjectionMatrix() const override;
	Vector3 GetPosition() const override { return cameraTransform_.translate; }
	Vector3 GetRotation() const { return cameraTransform_.rotate; }
	const Vector3Transform& GetTransform() const { return cameraTransform_; }
	Vector3 GetTarget() const { return target_; }
	std::string GetCameraType() const override { return "Debug"; }


	void SetPosition(const Vector3& position) override;
	void SetTarget(const Vector3& target) { target_ = target; UpdateSphericalFromPosition(); }


	/// <summary>
	/// ImGui表示
	/// </summary>
	void ImGui() override;


private:

	/// <summary>
	/// 初期座標・回転を指定してデフォルト値を設定
	/// </summary>
	/// <param name="position">初期座標</param>
	/// <param name="rotation">初期回転</param>
	void SetDefaultCamera(const Vector3& position, const Vector3& rotation = { 0.0f, 0.0f, 0.0f });

	/// <summary>
	/// 座標変換：球面座標系からデカルト座標系へ
	/// </summary>
	/// <param name="spherical">球面座標</param>
	/// <param name="center">中心点</param>
	/// <returns>デカルト座標</returns>
	Vector3 SphericalToCartesian(const SphericalCoordinates& spherical, const Vector3& center) const;

	/// <summary>
	/// 座標変換：デカルト座標系から球面座標系へ
	/// </summary>
	/// <param name="cartesian">デカルト座標</param>
	/// <param name="center">中心点</param>
	/// <returns>球面座標</returns>
	SphericalCoordinates CartesianToSpherical(const Vector3& cartesian, const Vector3& center) const;

	/// <summary>
	/// 現在のカメラ位置から球面座標を更新
	/// </summary>
	void UpdateSphericalFromPosition();

	/// <summary>
	/// 球面座標からカメラ位置を更新
	/// </summary>
	void UpdatePositionFromSpherical();

	/// <summary>
	/// 回転から10m先のターゲット座標を計算
	/// </summary>
	/// <param name="position">カメラ位置</param>
	/// <param name="rotation">カメラ回転</param>
	/// <returns>ターゲット座標</returns>
	Vector3 CalculateTargetFromRotation(const Vector3& position, const Vector3& rotation) const;

	/// <summary>
	/// ピボット回転（中クリックドラッグ）
	/// </summary>
	void HandlePivotRotation();

	/// <summary>
	/// カメラ移動（Shift+中クリックドラッグ）
	/// </summary>
	void HandleCameraMovement();

	/// <summary>
	/// ズーム（マウスホイール）
	/// </summary>
	void HandleZoom();

	/// <summary>
	/// キーボード移動
	/// </summary>
	void HandleKeyboardMovement();

	/// <summary>
	/// デバッグ入力処理
	/// </summary>
	void HandleDebugInput();

	/// <summary>
	/// 行列の更新
	/// </summary>
	void UpdateMatrix();

	/// <summary>
	/// カメラのローカル軸を取得
	/// </summary>
	Vector3 GetCameraForward() const;
	Vector3 GetCameraRight() const;
	Vector3 GetCameraUp() const;



	Vector3Transform cameraTransform_;

	// 行列
	Matrix4x4 viewMatrix_;
	Matrix4x4 projectionMatrix_;
	Matrix4x4 viewProjectionMatrix_;


	Vector3 target_ = { 0.0f, 0.0f, 0.0f };			// ピボットの中心座標
	SphericalCoordinates spherical_;				// 球面座標系での位置

	// デカルト座標系用の変数（ImGui表示・編集用）
	Vector3 cartesianPosition_;						// デカルト座標での位置
	Vector3 cartesianRotation_;						// デカルト座標での回転

	// 操作設定
	bool enableCameraControl_ = true;				// カメラ操作を使うかどうか
	float rotationSensitivity_ = 0.005f;			// 回転の感度
	float movementSensitivity_ = 0.01f;				// マウス移動の感度
	float zoomSensitivity_ = 0.05f;					// ズームの感度
	float keyboardSpeed_ = 0.1f;					// キーボード移動速度
	float mousePanSpeed_ = 0.01f;					// マウスパン移動速度

	// 制限
	float minDistance_ = 0.1f;						// 最小距離
	float maxDistance_ = 100.0f;					// 最大距離
	float minPhi_ = 0.1f;							// 最小仰角（ほぼ真上）
	float maxPhi_ = 3.04159f;						// 最大仰角（ほぼ真下）

	// 初期値保存用
	Vector3 initialPosition_;
	Vector3 initialRotation_;

	InputManager* input_;

};