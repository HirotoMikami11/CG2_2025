#pragma once
#include "GameObject.h"

/// <summary>
/// 三角形
/// </summary>
class Triangle : public GameObject {
private:
	
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	VertexData* vertexData;

public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="directX"></param>
	void Initialize(DirectXCommon* directX) override;
	
	/// <summary>
	/// 更新
	/// </summary>
	void Update() override;
	
	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="directX"></param>
	void Draw(DirectXCommon* directX) override;
	
	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize() override;

private:
	/// <summary>
	/// vertexDataを作成
	/// </summary>
	void CreateVertexData();
};