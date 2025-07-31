#pragma once
#include <d3d12.h>
#include <dxcapi.h>
#include <wrl.h>
#include <memory>
#include <unordered_map>
#include <string>
#include <mutex>

#include "PSODescriptor.h"
#include "RootSignatureBuilder.h"
#include "BaseSystem/Logger/Logger.h"

/// <summary>
/// PipelineStateObjectを生成するファクトリクラス
/// シェーダーのキャッシュ機能付き
/// </summary>
class PSOFactory {
public:
	/// <summary>
	/// PSO情報（RootSignatureとPipelineStateのペア）
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

	/// <summary>
	/// キャッシュされたシェーダーをクリア
	/// </summary>
	void ClearShaderCache();

	/// <summary>
	/// キャッシュ情報を取得
	/// </summary>
	size_t GetCachedShaderCount() const {
		std::lock_guard<std::mutex> lock(cacheMutex_);
		return shaderCache_.size();
	}

private:
	/// <summary>
	/// シェーダーをコンパイル（キャッシュ機能付き）
	/// </summary>
	/// <param name="shaderInfo">シェーダー情報</param>
	/// <returns>コンパイル済みシェーダー</returns>
	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const PSODescriptor::ShaderInfo& shaderInfo);

	/// <summary>
	/// シェーダーキャッシュのキーを生成
	/// </summary>
	/// <param name="shaderInfo">シェーダー情報</param>
	/// <returns>キャッシュキー</returns>
	std::wstring CreateShaderCacheKey(const PSODescriptor::ShaderInfo& shaderInfo) const;

private:
	// D3D12関連
	ID3D12Device* device_ = nullptr;

	// DXC関連
	IDxcUtils* dxcUtils_ = nullptr;
	IDxcCompiler3* dxcCompiler_ = nullptr;
	IDxcIncludeHandler* includeHandler_ = nullptr;

	// コンパイル済みシェーダーのキャッシュ
	std::unordered_map<std::wstring, Microsoft::WRL::ComPtr<IDxcBlob>> shaderCache_;

	// キャッシュアクセス用のミューテックス（スレッドセーフ）
	mutable std::mutex cacheMutex_;

	// 初期化フラグ
	bool isInitialized_ = false;
};