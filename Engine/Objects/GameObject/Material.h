#pragma once

#include <d3d12.h>
#include <wrl.h>

#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "MyMath/MyFunction.h"
#include "BaseSystem/Logger/Logger.h"
#include <cassert>

// ライティングモード定義
enum class LightingMode {
	None = 0,       // ライティングなし
	Lambert = 1,    // ランバート反射
	HalfLambert = 2 // ハーフランバート反射
};

class Material
{
public:
	Material() = default;
	~Material() = default;

	/// <summary>
	/// マテリアルを初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// デフォルト設定で初期化（ライティング無効、白色）
	/// </summary>
	void SetDefaultSettings();

	/// <summary>
	/// ライト付きオブジェクト用の設定（ライティング有効、白色）
	/// </summary>
	void SetLitObjectSettings();

	/// <summary>
	/// UVTransformの行列更新
	/// </summary>
	void UpdateUVTransform();

	//Getter

	//色
	Vector4 GetColor() const { return materialData_->color; }
	//ライティングモード
	LightingMode GetLightingMode() const { return lightingMode_; }
	// UVトランスフォーム
	Matrix4x4 GetUVTransform() const { return materialData_->uvTransform; };

	Vector2 GetUVTransformScale()const { return uvScale_; };
	float GetUVTransformRotateZ()const { return uvRotateZ_; };
	Vector2 GetUVTransformTranslate()const { return uvTranslate_; };

	//マテリアルリソース
	ID3D12Resource* GetResource() const { return materialResource_.Get(); }
	// マテリアルデータの直接取得（ImGui用）
	MaterialData* GetMaterialDataPtr() const { return materialData_; }

	//Setter

	void SetColor(const Vector4& color) { materialData_->color = color; }

	// ライティングモード設定
	void SetLightingMode(LightingMode mode);

	// UVトランスフォーム
	void SetUVTransform(const Matrix4x4& uvTransform) { materialData_->uvTransform = uvTransform; }
	void SetUVTransformScale(const Vector2 uvScale) { uvScale_ = uvScale; UpdateUVTransform(); };
	void SetUVTransformRotateZ(const float uvRotateZ) { uvRotateZ_ = uvRotateZ; UpdateUVTransform(); };
	void SetUVTransformTranslate(const Vector2 uvTranslate) { uvTranslate_ = uvTranslate; UpdateUVTransform(); };

private:
	// マテリアルリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	// マテリアルデータへのポインタ（Map済み）
	MaterialData* materialData_ = nullptr;

	// ライティングモード
	LightingMode lightingMode_ = LightingMode::None;

	// UVTransformを変更するための変数
	Vector2 uvTranslate_ = { 0.0f,0.0f };
	Vector2 uvScale_ = { 1.0f,1.0f };
	float uvRotateZ_ = { 0.0f };
};