#pragma once
#define NOMINMAX	//C+標準のstd::maxを使えるようにするため(windows.hが上書きしてしまっている)
#include <d3d12.h>
#include <wrl.h>
#include "DirectXCommon.h"
#include "MyFunction.h"
#include "Logger.h"
#include <cassert>
#include "inputManager.h"
#include "ImguiManager.h"

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

	/// <summary>
	/// 入力で移動、回転させる
	/// </summary>
	void Move();

	/// <summary>
	/// 入力で回転させる
	/// </summary>
	void Rotate();

	/// <summary>
	 /// ピボット回転させる
	 /// </summary>
	void PivotRotate();

	/// <summary>
	/// ピボット点を設定
	/// </summary>
	void SetPivotPoint(const Vector3& pivot) { pivotPoint_ = pivot; }

	/// <summary>
	/// ピボット点を取得
	/// </summary>
	Vector3 GetPivotPoint() const { return pivotPoint_; }


	Matrix4x4 GetViewProjectionMatrix() const { return viewProjectionMatrix_; }

	void SetTranslate(const Vector3& position) { translation_ = position; }

	/// <summary>
	/// ImGui
	/// </summary>
	void ImGui();

private:

	/// <summary>
	/// 行列の更新を行う関数
	/// </summary>
	void UpdateMatrix();


	//X,Y,Z軸周りのローカル回転角
	Vector3 rotation_ = { 0,0,0 };
	//ローカル座標
	Vector3 translation_ = { 0,0,0 };

	// デバッグカメラのトランスフォーム
	Vector3Transform cameraTransform_{
		.scale{1.0f, 1.0f, 1.0f},
		.rotate{rotation_},
		.translate{translation_}
	};

	//ビュー行列
	Matrix4x4 viewMatrix_;
	//射影行列
	Matrix4x4 projectionMatrix_;

	Matrix4x4 viewProjectionMatrix_;


	// ピボット回転用の変数
	Vector3 pivotPoint_ = { 0, 0, 0 };		// ピボット点（回転の中心）
	float pivotDistance_ = 10.0f;			// ピボット点からの距離
	Vector3 pivotRotation_ = { 0, 0, 0 };	// ピボット回転角

	//デバッグカメラで移動するかどうか
	bool moveDebugCamera_ = true;

};

