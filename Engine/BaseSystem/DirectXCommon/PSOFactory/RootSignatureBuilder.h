#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <memory>
#include <cassert>
#include "BaseSystem/Logger/Logger.h"

/// <summary>
/// RootSignatureを簡単に構築するためのクラス
/// </summary>
class RootSignatureBuilder {
public:
	RootSignatureBuilder() = default;
	~RootSignatureBuilder() = default;

	/// <summary>
	/// ConstantBufferViewを追加
	/// </summary>
	/// <param name="shaderRegister">シェーダーレジスタ番号（b0, b1など）</param>
	/// <param name="visibility">シェーダーの可視性</param>
	RootSignatureBuilder& AddCBV(uint32_t shaderRegister,
		D3D12_SHADER_VISIBILITY visibility);

	/// <summary>
	/// ShaderResourceViewのDescriptorTableを追加
	/// </summary>
	/// <param name="baseShaderRegister">開始シェーダーレジスタ番号（t0, t1など）</param>
	/// <param name="count">SRVの数</param>
	/// <param name="visibility">シェーダーの可視性</param>
	RootSignatureBuilder& AddSRV(uint32_t baseShaderRegister,
		uint32_t count,
		D3D12_SHADER_VISIBILITY visibility);

	/// <summary>
	/// UnorderedAccessViewのDescriptorTableを追加
	/// </summary>
	/// <param name="baseShaderRegister">開始シェーダーレジスタ番号（u0, u1など）</param>
	/// <param name="count">UAVの数</param>
	/// <param name="visibility">シェーダーの可視性</param>
	RootSignatureBuilder& AddUAV(uint32_t baseShaderRegister,
		uint32_t count,
		D3D12_SHADER_VISIBILITY visibility);

	/// <summary>
	/// Static Samplerを追加
	/// </summary>
	/// <param name="shaderRegister">シェーダーレジスタ番号（s0, s1など）</param>
	/// <param name="filter">フィルタリング方法</param>
	/// <param name="addressMode">テクスチャアドレスモード</param>
	/// <param name="visibility">シェーダーの可視性</param>
	RootSignatureBuilder& AddStaticSampler(uint32_t shaderRegister,
		D3D12_FILTER filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE addressMode = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_PIXEL);

	/// <summary>
	/// RootSignatureのフラグを設定
	/// </summary>
	RootSignatureBuilder& SetFlags(D3D12_ROOT_SIGNATURE_FLAGS flags);

	/// <summary>
	/// 設定された内容でRootSignatureを生成
	/// </summary>
	/// <param name="device">D3D12デバイス</param>
	/// <returns>生成されたRootSignature</returns>
	Microsoft::WRL::ComPtr<ID3D12RootSignature> Build(ID3D12Device* device);

	/// <summary>
	/// 現在のパラメータ数を取得
	/// </summary>
	size_t GetParameterCount() const { return rootParameters_.size(); }

private:
	/// <summary>
	/// DescriptorRangeを保持する構造体
	/// </summary>
	struct DescriptorRangeHolder {
		std::vector<D3D12_DESCRIPTOR_RANGE> ranges;
	};

	/// <summary>
	/// DescriptorTableのポインタを更新（vectorの再配置に対応するため）
	/// </summary>
	void UpdateDescriptorTablePointers();

private:
	// RootParameterの配列
	std::vector<D3D12_ROOT_PARAMETER> rootParameters_;

	// DescriptorRangeを保持（DescriptorTableのポインタが有効であることを保証）
	std::vector<std::unique_ptr<DescriptorRangeHolder>> descriptorRangeHolders_;

	// Static Samplerの配列
	std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers_;

	// RootSignatureのフラグ
	D3D12_ROOT_SIGNATURE_FLAGS flags_ = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
};