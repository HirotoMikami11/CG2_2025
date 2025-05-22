#pragma once

#include <d3d12.h>
#include <wrl.h>

#include "DirectXCommon.h"
#include "MyFunction.h"
#include "Logger.h"
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
	/// <returns>初期化成功かどうか</returns>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// デフォルト設定で初期化（ライティング無効、白色）
	/// </summary>
	void SetDefaultSettings();

	/// <summary>
	/// ライト付きオブジェクト用の設定（ライティング有効、白色）
	/// </summary>
	void SetLitObjectSettings();

	//Getter

	//色
	Vector4 GetColor() const { return materialData_->color; }
	//ライティング
	bool IsLightingEnabled() const {return materialData_->enableLighting;}
	// ランベルト反射
	bool IsLambertianReflectanceEnabled() const { return materialData_->useLambertianReflectance ; }
	// UVトランスフォーム
	Matrix4x4 GetUVTransform() const { return materialData_->uvTransform; };
	//マテリアルリソース
	ID3D12Resource* GetResource() const { return materialResource_.Get(); }
	// マテリアルデータの直接取得（ImGui用）
	MaterialData* GetMaterialDataPtr() const { return materialData_; }

	//Setter

	void SetColor(const Vector4& color) { materialData_->color = color; }
	// ライティングの有効/無効
	void SetLightingEnable(bool enable) { materialData_->enableLighting = enable ? 1 : 0; }
	//ランベルト反射
	void SetLambertianReflectance(bool enable) {materialData_->useLambertianReflectance = enable ? 1 : 0;}
	// UVトランスフォーム
	void SetUVTransform(const Matrix4x4& uvTransform) { materialData_->uvTransform = uvTransform; }

private:
	// マテリアルリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	// マテリアルデータへのポインタ（Map済み）
	MaterialData* materialData_ = nullptr;

};

