#pragma once

#include <d3d12.h>
#include <wrl.h>
#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "MyMath/MyFunction.h"

/// <summary>
/// オフスクリーン描画用の大きな三角形クラス
/// </summary>
class OffscreenTriangle {
public:
	OffscreenTriangle() = default;
	~OffscreenTriangle() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// カスタムPSOを使用した描画（テクスチャ1枚）
	/// </summary>
	/// <param name="rootSignature">使用するルートシグネチャ</param>
	/// <param name="pipelineState">使用するパイプラインステート</param>
	/// <param name="textureHandle">使用するテクスチャハンドル</param>
	/// <param name="materialBufferGPUAddress">マテリアルバッファのGPUアドレス（オプション）</param>
	void DrawWithCustomPSO(
		ID3D12RootSignature* rootSignature,
		ID3D12PipelineState* pipelineState,
		D3D12_GPU_DESCRIPTOR_HANDLE textureHandle,
		D3D12_GPU_VIRTUAL_ADDRESS materialBufferGPUAddress = 0
	);

	/// <summary>
	/// 深度テクスチャ対応のカスタムPSOを使用した描画（テクスチャ2枚）
	/// </summary>
	/// <param name="rootSignature">使用するルートシグネチャ</param>
	/// <param name="pipelineState">使用するパイプラインステート</param>
	/// <param name="colorTextureHandle">カラーテクスチャハンドル</param>
	/// <param name="depthTextureHandle">深度テクスチャハンドル</param>
	/// <param name="materialBufferGPUAddress">マテリアルバッファのGPUアドレス（オプション）</param>
	void DrawWithCustomPSOAndDepth(
		ID3D12RootSignature* rootSignature,
		ID3D12PipelineState* pipelineState,
		D3D12_GPU_DESCRIPTOR_HANDLE colorTextureHandle,
		D3D12_GPU_DESCRIPTOR_HANDLE depthTextureHandle,
		D3D12_GPU_VIRTUAL_ADDRESS materialBufferGPUAddress = 0
	);

	/// <summary>
	/// 基本的な描画（デフォルトのオフスクリーンPSO使用）
	/// </summary>
	/// <param name="textureHandle">使用するテクスチャハンドル</param>
	void Draw(D3D12_GPU_DESCRIPTOR_HANDLE textureHandle);

	/// <summary>
	/// 有効かどうか
	/// </summary>
	bool IsValid() const { return vertexBuffer_ != nullptr; }

private:
	/// <summary>
	/// 大きな三角形の頂点データを作成
	/// </summary>
	void CreateFullscreenTriangle();

	/// <summary>
	/// 頂点バッファを作成
	/// </summary>
	void CreateVertexBuffer();

private:
	// システム参照
	DirectXCommon* dxCommon_ = nullptr;

	// 頂点データ（大きな三角形用）
	struct FullscreenVertex {
		Vector4 position;   // 座標
		Vector2 texcoord;   // UV座標
	};

	std::vector<FullscreenVertex> vertices_;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
};