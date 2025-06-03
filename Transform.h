#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "DirectXCommon.h"
#include "MyFunction.h"
#include "Logger.h"
#include <cassert>

class DirectXCommon;


class Transform final
{

public:
	Transform() = default;
	~Transform() = default;

	/// <summary>
	/// トランスフォームの初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonの初期化</param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 行列の更新
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション</param>
	void UpdateMatrix(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// デフォルトの値を設定する
	/// </summary>
	void SetDefaultTransform();

	//Getter
	const Vector3Transform& GetTransform() const { return transform_; }

	Vector3 GetPosition() const { return transform_.translate; }
	Vector3 GetRotate() const { return transform_.rotate; }
	Vector3 GetScale() const { return transform_.scale; }

	Matrix4x4 GetWorldMatrix() const { return transformData_->World; };
	Matrix4x4 GetWVPMatrix() const { return transformData_->WVP; };
	ID3D12Resource* GetResource() const { return transformResource_.Get(); }
	///トランスフォームデータの直接取得（ImGui用）
	TransformationMatrix* GetTransformDataPtr() const { return transformData_; }


	//Setter
	void SetTransform(const Vector3Transform& newTransform) { transform_ = newTransform; }
	void SetScale(const Vector3& scale) { transform_.scale = scale; }
	void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }
	void SetPosition(const Vector3& translate) { transform_.translate = translate; }

	///指定した値で回転
	void AddPosition(const Vector3& Position);
	void AddRotation(const Vector3& rotation);
	void AddScale(const Vector3& Scale);


private:
	// GPU用トランスフォームリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> transformResource_;
	// トランスフォームデータへのポインタ（Map済み）
	TransformationMatrix* transformData_ = nullptr;

	// CPU側のトランスフォーム値
	Vector3Transform transform_{
		.scale{1.0f, 1.0f, 1.0f},
		.rotate{0.0f, 0.0f, 0.0f},
		.translate{0.0f, 0.0f, 0.0f}
	};


};

