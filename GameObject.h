#pragma once
#include "DirectXCommon.h"
#include "MyFunction.h"

/// <summary>
/// ゲームオブジェクトの基底クラス
/// </summary>
class GameObject
{
public:
	GameObject() = default;
	virtual ~GameObject() = default;

	virtual void Initialize(DirectXCommon* directX) = 0;
	virtual void Update() = 0;
	virtual void Draw(DirectXCommon* directX) = 0;
	virtual void Finalize() = 0;

	// Transform操作
	void SetTransform(const Vector3Transform& newTransform) { transform = newTransform; }
	const Vector3Transform& GetTransform() const { return transform; }

	// Material操作
	void SetMaterial(const Material& newMaterial) { material = newMaterial; }
	const Material& GetMaterial() const { return material; }

protected:
	void CreateResources(DirectXCommon* directX);
	void UpdateTransform(const Matrix4x4& viewProjection);

	///オブジェクトの共通部分の変数

	Vector3Transform transform;
	Material material;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> transformResource;
	TransformationMatrix* transformData;
	Material* materialData;
};

