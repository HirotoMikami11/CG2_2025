#pragma once

#include <Windows.h>
#include <d3d12.h>
#include <cstdint>
#include "MyFunction.h"
class Triangle {
public:

	///コンストラクタ、デストラクタ
	Triangle();
	~Triangle();

	//初期化
	void Initialize(ID3D12Device* device);

	// 更新
	void Update(const Matrix4x4& viewProjectionMatrix);

	// 描画
	void Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle);

	//アクセッサ

	void  SetPosition(const Vector3& position) {
		transform.translate = position;
	}

	void  SetRotation(const Vector3& rotation) {
		transform.rotate = rotation;
	}

	void  SetScale(const Vector3& scale) {
		transform.scale = scale;
	}

	void  SetColor(const Vector4& color) {
		materialData->color = color;
	}

	Vector4& GetColor() {
		return materialData->color;
	}

	const Vector3Transform& GetTransform() const { return transform; }

private:

	ID3D12Resource* vertexResource = nullptr;
	ID3D12Resource* materialResource = nullptr;
	ID3D12Resource* wvpResource = nullptr;


	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;


	VertexData* vertexData = nullptr;
	Material* materialData = nullptr;
	TransformationMatrix* wvpData = nullptr;

	Vector3Transform transform;

	void CreateVertexData();
	void CreateBufferViews();
};