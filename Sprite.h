#pragma once

#include <Windows.h>
#include <d3d12.h>
#include <cstdint>
#include "MyFunction.h"

class Sprite {
public:

	/// コンストラクタ、デストラクタ
	Sprite();
	~Sprite();

	// 初期化
	void Initialize(ID3D12Device* device);

	// 更新
	void Update();

	// 描画
	void Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle);

	// アクセッサ
	void SetPosition(const Vector2& position) {
		this->position = position;//名前がかぶっているのでthis
		UpdateVertexData();
	}

	void SetSize(const Vector2& size) {
		this->size = size;
		UpdateVertexData();
	}

	void SetColor(const Vector4& color) {
		materialData->color = color;
	}

	Vector4& GetColor() {
		return materialData->color;
	}

	void SetUVTransform(const Vector3Transform& uvTransform) {
		this->uvTransform = uvTransform;
	}

	const Vector3Transform& GetUVTransform() const { return uvTransform; }

	Vector2 GetPosition() const { return position; }
	Vector2 GetSize() const { return size; }

private:

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource = nullptr;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;

	VertexData* vertexData = nullptr;
	Material* materialData = nullptr;
	TransformationMatrix* transformationMatrixData = nullptr;

	// Spriteは2Dなので2D用の値を持っておく
	Vector2 position = { 0.0f, 0.0f };	// 中心座標
	Vector2 size = { 100.0f, 100.0f };	// サイズ
	Vector3Transform uvTransform{};		// UVアニメーション用

	void CreateVertexData();		// VertexData作成
	void CreateIndexData();			//indexData作成
	void CreateBufferViews();		//BufferView作成
	void UpdateVertexData();		// 座標やサイズ変更時に頂点データを更新
};