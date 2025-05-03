#pragma once

#include <Windows.h>
#include <d3d12.h>
#include <cstdint>
#include "MyFunction.h"

class TriangularPyramid {
public:
	// コンストラクタ、デストラクタ
	TriangularPyramid();
	~TriangularPyramid();

	// 初期化
	void Initialize(ID3D12Device* device);

	// 更新
	void Update(const Matrix4x4& viewProjectionMatrix);

	// 描画
	void Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle);

	// アクセッサ
	void SetTransform(const Vector3Transform& vector3Transform) {
		transform = vector3Transform;
	}

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

private:
	// 頂点数とインデックス数の定数
	static const int kVertexCount = 4;  // 三角錐の頂点数（底面の3点 + 頂点の1点）
	static const int kTriangleCount = 4; // 三角錐を構成する三角形の数（底面1 + 側面3）
	static const int kIndexCount = kTriangleCount * 3; // インデックス数 (三角形ごとに3頂点)

	ID3D12Resource* vertexResource = nullptr;
	ID3D12Resource* materialResource = nullptr;
	ID3D12Resource* wvpResource = nullptr;
	ID3D12Resource* indexResource = nullptr;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;

	VertexData* vertexData = nullptr;
	Material* materialData = nullptr;
	TransformationMatrix* wvpData = nullptr;
	uint16_t* indexData = nullptr;

	Vector3Transform transform;

	// 頂点データとインデックスデータの作成
	void CreateVertexData();
	void CreateIndexData();
	void CreateBufferViews();
};