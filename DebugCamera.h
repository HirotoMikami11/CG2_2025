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



	Matrix4x4 GetViewProjectionMatrix() const { return viewProjectionMatrix_; }
	void SetTranslate(const Vector3& position) { cameraTransform_.translate = position; }
	void SetRotation(const Vector3& rotation) { cameraTransform_.rotate = rotation; }


	/// <summary>
	/// ImGui
	/// </summary>
	void ImGui();

private:

	/// <summary>
	/// 行列の更新を行う関数
	/// </summary>
	void UpdateMatrix();


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

	InputManager* input_;

};

