#pragma once

#include <Windows.h>
#include <d3d12.h>
#include <cstdint>
#include "MyFunction.h"

class Sphere {
public:

	/// コンストラクタ、デストラクタ
	Sphere();
	~Sphere();

	// 初期化
	void Initialize(ID3D12Device* device);

	// 更新
	void Update(const Matrix4x4& viewProjectionMatrix);

	// 描画
	void Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle);

	// アクセッサ

	void SetPosition(const Vector3& position) {
		transform.translate = position;
	}

	void SetRotation(const Vector3& rotation) {
		transform.rotate = rotation;
	}

	void SetScale(const Vector3& scale) {
		transform.scale = scale;
	}

	void SetColor(const Vector4& color) {
		materialData->color = color;
	}

	Vector4& GetColor() {
		return materialData->color;
	}

	const Vector3Transform& GetTransform() const { return transform; }

	// ライティング設定のアクセッサ
	void SetEnableLighting(bool enable) {
		materialData->enableLighting = enable;
	}

	void SetUseLambertianReflectance(bool use) {
		materialData->useLambertianReflectance = use;
	}

	// ライトの設定
	void SetDirectionalLight(const Vector3& direction, const Vector4& color, float intensity) {
		directionalLightData->direction = direction;
		directionalLightData->color = color;
		directionalLightData->intensity = intensity;
	}

private:

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = nullptr;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;

	VertexData* vertexData = nullptr;
	uint32_t* indexData = nullptr;
	Material* materialData = nullptr;
	TransformationMatrix* wvpData = nullptr;
	DirectionalLight* directionalLightData = nullptr;

	Vector3Transform transform;

	// 分割数（デフォルトは16x16）
	static const int32_t kSubdivision = 16;

	void CreateVertexData();
	void CreateIndexData();
	void CreateBufferViews();
};