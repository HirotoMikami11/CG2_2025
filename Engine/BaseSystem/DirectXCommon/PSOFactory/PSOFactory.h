#pragma once
#include <d3d12.h>
#include <dxcapi.h>
#include <wrl.h>
#include <memory>
#include <string>

#include "PSODescriptor.h"
#include "RootSignatureBuilder.h"
#include "BaseSystem/Logger/Logger.h"

/// <summary>
/// PipelineStateObjectを生成するクラス
/// </summary>
class PSOFactory {
public:
	/// <summary>
	/// PSO情報（RootSignatureとPipelineState）
	/// </summary>
	struct PSOInfo {
		Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;

		bool IsValid() const {
			return rootSignature && pipelineState;
		}
	};

public:
	PSOFactory() = default;
	~PSOFactory() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="device">D3D12デバイス</param>
	/// <param name="dxcUtils">DXCユーティリティ</param>
	/// <param name="dxcCompiler">DXCコンパイラ</param>
	/// <param name="includeHandler">インクルードハンドラ</param>
	void Initialize(ID3D12Device* device,
		IDxcUtils* dxcUtils,
		IDxcCompiler3* dxcCompiler,
		IDxcIncludeHandler* includeHandler);

	/// <summary>
	/// PSOを作成（RootSignatureも同時に作成）
	/// </summary>
	/// <param name="descriptor">PSO設定</param>
	/// <param name="rootSignatureBuilder">RootSignature設定</param>
	/// <returns>作成されたPSO情報</returns>
	PSOInfo CreatePSO(const PSODescriptor& descriptor,
		RootSignatureBuilder& rootSignatureBuilder);

	/// <summary>
	/// 既存のRootSignatureを使用してPSOを作成
	/// </summary>
	/// <param name="descriptor">PSO設定</param>
	/// <param name="existingRootSignature">既存のRootSignature</param>
	/// <returns>作成されたPipelineState</returns>
	Microsoft::WRL::ComPtr<ID3D12PipelineState> CreatePSO(
		const PSODescriptor& descriptor,
		ID3D12RootSignature* existingRootSignature);

private:
	// D3D12関連
	ID3D12Device* device_ = nullptr;

	// DXC関連
	IDxcUtils* dxcUtils_ = nullptr;
	IDxcCompiler3* dxcCompiler_ = nullptr;
	IDxcIncludeHandler* includeHandler_ = nullptr;

	// 初期化フラグ
	bool isInitialized_ = false;
};