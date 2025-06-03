#include "Material.h"

void Material::Initialize(DirectXCommon* dxCommon)
{
	// マテリアル用のリソースを作成
	materialResource_ = CreateBufferResource(dxCommon->GetDevice(), sizeof(MaterialData));
	// マテリアルデータにマップ
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	// デフォルト設定で初期化
	SetDefaultSettings();

}

void Material::SetDefaultSettings()
{
	// デフォルト設定
	// ライティング無効、白色、UV変換は単位行列
	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData_->enableLighting = false;
	materialData_->useLambertianReflectance = false;
	materialData_->padding[0] = 0.0f;
	materialData_->padding[1] = 0.0f;
	materialData_->uvTransform = MakeIdentity4x4();
}

void Material::SetLitObjectSettings()
{
	// ライト付きオブジェクト用設定
	// ライティング有効、白色、UV変換は単位行列
	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData_->enableLighting = true;
	materialData_->useLambertianReflectance = false;
	materialData_->padding[0] = 0.0f;
	materialData_->padding[1] = 0.0f;
	materialData_->uvTransform = MakeIdentity4x4();
}

void Material::UpdateUVTransform()
{
	Matrix4x4 uvTransformMatrix = MakeScaleMatrix({uvScale_.x,uvScale_.y,0.0f});
	uvTransformMatrix = Matrix4x4Multiply(uvTransformMatrix, MakeRotateZMatrix(uvRotateZ_));
	uvTransformMatrix = Matrix4x4Multiply(uvTransformMatrix, MakeTranslateMatrix({ uvTranslate_.x,uvTranslate_.y,0.0f }));
	materialData_->uvTransform = uvTransformMatrix;

}

