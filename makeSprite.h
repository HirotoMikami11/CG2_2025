#pragma once
///DirectX12
#include<d3d12.h>
#pragma comment(lib,"d3d12.lib")
#include<dxgi1_6.h>
#pragma comment(lib,"dxgi.lib")
#include <dxgidebug.h>
#pragma comment(lib,"dxguid.lib")

#include "MyFunction.h"
#include<cassert>

class makeSprite
{
	//Transform変数を作る
	Vector3Transform transformSprite{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};

	ID3D12Resource* vertexResourceSprite;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	ID3D12Resource* transformationMatrixResourceSprite;
	//データを書き込む
	Matrix4x4* transformationMatrixDataSprite = nullptr;

	/// <summary>
	/// Resourceを生成する関数
	/// </summary>
	/// <param name="device">ID3D12Device</param>
	/// <param name="sizeInBytes">リソースサイズ</param>
	/// <returns></returns>
	static ID3D12Resource* CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);
public:
	~makeSprite();
	//初期化
	void Initialize(ID3D12Device* device, Vector2& centerPosition, Vector2& size);

	// 更新
	void Update(const Matrix4x4& viewProjectionMatrix);

	// 描画
	void Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle);

};

