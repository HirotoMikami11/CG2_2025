#pragma once
#define NOMINMAX	//C+標準のstd::maxを使えるようにするため(windows.hが上書きしてしまっている)
#include <d3d12.h>
#include <wrl.h>

#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "MyMath/MyFunction.h"
#include "Managers/inputManager.h"


/// <summary>
/// カメラの回転モード
/// </summary>
enum class CameraRotationMode {
	Default,		// カメラ移動
	Pivot			// ピボット回転
};

/// <summary>
/// デバッグカメラのクラス
/// </summary>
class DebugCamera
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// デフォルトの値を設定する
	/// </summary>
	void SetDefaultCamera();


	//通常回転モード

	/// <summary>
	/// 入力で移動、回転させる（オイラー角モード）
	/// </summary>
	void Move();

	/// <summary>
	/// 入力で回転させる（オイラー角モード）
	/// </summary>
	void Rotate();

	//ピボット回転モード

	/// <summary>
	/// ピボット回転での移動
	/// </summary>
	void PivotMove();

	/// <summary>
	/// ピボット回転での回転
	/// </summary>
	void PivotRotate();


	Matrix4x4 GetViewProjectionMatrix() const { return viewProjectionMatrix_; }
	void SetPositon(const Vector3& position) { cameraTransform_.translate = position; }
	void SetRotation(const Vector3& rotation) { cameraTransform_.rotate = rotation; }
	void SetTarget(const Vector3& target) { target_ = target; }
	void SetRotationMode(CameraRotationMode mode) { rotationMode_ = mode; }

	/// <summary>
	/// ImGui
	/// </summary>
	void ImGui();

private:

	/// <summary>
	/// 行列の更新を行う関数
	/// </summary>
	void UpdateMatrix();
	/// <summary>
	/// ピボット回転用の行列更新
	/// </summary>
	void UpdatePivotMatrix();

	/// <summary>
	/// 回転モードを変更する関数
	/// </summary>
	void ChangeRotationMode();

	// デバッグカメラのトランスフォーム
	Vector3Transform cameraTransform_{
	.scale{1.0f, 1.0f, 1.0f},
	.rotate{0.0f, 0.0f, 0.0f},
	.translate{0.0f, 0.0f, -10.0f}
	};

	//行列
	Matrix4x4 viewMatrix_;
	//プロジェクション行列
	Matrix4x4 projectionMatrix_;
	//ビュープロジェクション行列
	Matrix4x4 viewProjectionMatrix_;

	//デバッグカメラで移動するかどうか
	bool moveDebugCamera_ = false;

	//回転モード
	CameraRotationMode rotationMode_ = CameraRotationMode::Default;

	//ピボット回転用のメンバ変数
	Vector3 target_ = { 0.0f, 0.0f, 0.0f };	// ピボットの中心座標
	Matrix4x4 matRot_ = MakeIdentity4x4();	// 累積回転行列
	float distance_ = 10.0f;				// ターゲットからの距離

	//移動用
	float speed;
	float wheelSpeed;

	//移動ベクトル
	Vector3 move = { 0, 0, 0 };

	InputManager* input_;
};