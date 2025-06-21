#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <cassert>

#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "MyMath/MyFunction.h"
#include "BaseSystem/Logger/Logger.h"

/// <summary>
/// cameraのクラス
/// </summary>
class Camera
{
public:
	Camera() = default;
	~Camera() = default;

	/// <summary>
	/// カメラの初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	void Initialize();


	/// <summary>
	/// cameraの更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// ビュープロジェクション行列の更新
	/// </summary>
	void UpdateMatrix();

	/// <summary>
	/// スプライト用ビュープロジェクション行列の更新
	/// </summary>
	void UpdateSpriteMatrix();

	/// <summary>
	/// デフォルトの値を設定する
	/// </summary>
	void SetDefaultCamera();

	/// <summary>
	/// imguiの表示
	/// </summary>
	void ImGui();

	//Getter

	// 3Dカメラ用
	const Vector3Transform& GetTransform() const { return cameraTransform_; }
	Vector3 GetTranslate() const { return cameraTransform_.translate; }
	Vector3 GetRotate() const { return cameraTransform_.rotate; }
	//3D用
	Matrix4x4 GetViewProjectionMatrix() const { return viewProjectionMatrix_; }
	// スプライト用
	Matrix4x4 GetSpriteViewProjectionMatrix() const { return spriteViewProjectionMatrix_; }

	//Setter

	// 3Dカメラ用
	void SetTransform(const Vector3Transform& newTransform) { cameraTransform_ = newTransform; }
	void SetPositon(const Vector3& position) { cameraTransform_.translate = position; }
	void SetRotate(const Vector3& rotation) { cameraTransform_.rotate = rotation; }
	
private:

	// 3Dカメラ用のトランスフォーム
	Vector3Transform cameraTransform_{
		.scale{1.0f, 1.0f, 1.0f},
		.rotate{0.0f, 0.0f, 0.0f},
		.translate{0.0f, 0.0f, -10.0f}
	};

	// カメラパラメータ
	float fov_ = 0.45f;
	float nearClip_ = 0.1f;
	float farClip_ = 100.0f;
	float aspectRatio = (float(GraphicsConfig::kClientWidth) / float(GraphicsConfig::kClientHeight));

	// 行列 
	Matrix4x4 viewMatrix_;

	//プロジェクション行列
	Matrix4x4 projectionMatrix_;			//3D用のプロジェクション行列
	Matrix4x4 spriteProjectionMatrix_;		//スプライト用のプロジェクション行列
	//ビュープロジェクション行列
	Matrix4x4 viewProjectionMatrix_;		// 3D用ビュープロジェクション行列
	Matrix4x4 spriteViewProjectionMatrix_;	// スプライト用ビュープロジェクション行列


	bool useSpriteViewProjectionMatrix_;	// スプライト用ビュープロジェクション行列を使用するか

};

