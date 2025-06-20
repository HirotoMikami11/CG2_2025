#pragma once

#include <d3d12.h>
#include <wrl.h>

#include "DirectXCommon.h"
#include "MyMath/MyFunction.h"
#include "BaseSystem/Logger/Logger.h"
#include <cassert>

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
	//ライティング(halfLambert)
	bool IsLightingEnabled() const { return materialData_->enableLighting; }
	// Lambert
	bool IsLambertianReflectanceEnabled() const { return materialData_->useLambertianReflectance; }
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
	// ライティングの有効/無効
	void SetLightingEnable(bool enable) { materialData_->enableLighting = enable ? 1 : 0; }
	//Lambert
	void SetLambertianReflectance(bool enable) { materialData_->useLambertianReflectance = enable ? 1 : 0; }
	// UVトランスフォーム
	void SetUVTransform(const Matrix4x4& uvTransform) { materialData_->uvTransform = uvTransform; }
	void SetUVTransformScale(const Vector2 uvScale) { uvScale_ = uvScale; UpdateUVTransform(); };
	void SetUVTransformRotateZ(const float uvRotateZ) { uvRotateZ_ = uvRotateZ; UpdateUVTransform();};
	void SetUVTransformTranslate(const Vector2 uvTranslate) { uvTranslate_ = uvTranslate; UpdateUVTransform();};

private:
	// マテリアルリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	// マテリアルデータへのポインタ（Map済み）
	MaterialData* materialData_ = nullptr;


	// UVTransformを変更するための変数
	Vector2 uvTranslate_ = { 0.0f,0.0f };
	Vector2 uvScale_ = { 1.0f,1.0f };
	float uvRotateZ_ = { 0.0f };

};

