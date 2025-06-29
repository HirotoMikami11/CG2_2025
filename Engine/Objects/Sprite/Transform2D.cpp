#include "Transform2D.h"

void Transform2D::Initialize(DirectXCommon* dxCommon)
{
	// トランスフォーム用のリソースを作成
	transformResource_ = CreateBufferResource(dxCommon->GetDevice(), sizeof(TransformationMatrix));

	// トランスフォームデータにマップ
	transformResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformData_));

	// デフォルト設定で初期化
	SetDefaultTransform();
}

void Transform2D::UpdateMatrix(const Matrix4x4& viewProjectionMatrix)
{
	// 2D用のワールド行列を作成
	// スケール行列
	Matrix4x4 scaleMatrix = MakeScaleMatrix({ transform_.scale.x, transform_.scale.y, 1.0f });

	// 回転行列（Z軸のみ）
	Matrix4x4 rotateMatrix = MakeRotateZMatrix(transform_.rotateZ);

	// 平行移動行列（Z座標は0固定）
	Matrix4x4 translateMatrix = MakeTranslateMatrix({ transform_.translate.x, transform_.translate.y, 0.0f });

	// ワールド行列を計算（S * R * T の順番）
	transformData_->World = Matrix4x4Multiply(scaleMatrix, rotateMatrix);
	transformData_->World = Matrix4x4Multiply(transformData_->World, translateMatrix);

	// ビュープロジェクション行列を掛け算してWVP行列を計算
	transformData_->WVP = Matrix4x4Multiply(transformData_->World, viewProjectionMatrix);
}

void Transform2D::SetDefaultTransform()
{
	// デフォルト値に設定
	transform_.scale = { 1.0f, 1.0f };
	transform_.rotateZ = 0.0f;
	transform_.translate = { 0.0f, 0.0f };

	// GPU側のデータも単位行列で初期化
	transformData_->World = MakeIdentity4x4();
	transformData_->WVP = MakeIdentity4x4();
}

void Transform2D::AddPosition(const Vector2& position)
{
	transform_.translate.x += position.x;
	transform_.translate.y += position.y;
}

void Transform2D::AddRotation(float rotation)
{
	transform_.rotateZ += rotation;
}

void Transform2D::AddScale(const Vector2& scale)
{
	transform_.scale.x += scale.x;
	transform_.scale.y += scale.y;
}