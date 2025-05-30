#include "Transform.h"

void Transform::Initialize(DirectXCommon* dxCommon)
{
	// トランスフォーム用のリソースを作成
	transformResource_ = CreateBufferResource(dxCommon->GetDevice(), sizeof(TransformationMatrix));

	// トランスフォームデータにマップ
	transformResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformData_));

	// デフォルト設定で初期化
	SetDefaultTransform();
}

void Transform::UpdateMatrix(const Matrix4x4& viewProjectionMatrix)
{
	// トランスフォームデータを更新
	transformData_->World = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	// ビュープロジェクション行列を掛け算してWVP行列を計算
	transformData_->WVP = Matrix4x4Multiply(transformData_->World, viewProjectionMatrix);
}

void Transform::SetDefaultTransform() {

	// デフォルト値に設定
	transform_.scale = { 1.0f, 1.0f, 1.0f };
	transform_.rotate = { 0.0f, 0.0f, 0.0f };
	transform_.translate = { 0.0f, 0.0f, 0.0f };

	// GPU側のデータも単位行列で初期化
	transformData_->World = MakeIdentity4x4();
	transformData_->WVP = MakeIdentity4x4();
}

void Transform::AddPosition(const Vector3& Position)
{
	transform_.translate.x += Position.x;
	transform_.translate.y += Position.y;
	transform_.translate.z += Position.z;
}

void Transform::AddRotation(const Vector3& rotation)
{
	transform_.rotate.x += rotation.x;
	transform_.rotate.y += rotation.y;
	transform_.rotate.z += rotation.z;

}

void Transform::AddScale(const Vector3& Scale)
{
	transform_.scale.x += Scale.x;
	transform_.scale.y += Scale.y;
	transform_.scale.z += Scale.z;
}


