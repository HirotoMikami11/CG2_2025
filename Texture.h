#pragma once
#include <wrl.h>
#include <string>

#include "DirectXCommon.h"
#include "Logger.h"
#include "MyFunction.h" // CreateBufferResource用
#include <cassert>
class DirectXCommon; // 前方宣言

/// <summary>
/// テクスチャクラス - テクスチャの読み込みと管理を行う
/// </summary>
class Texture {
public:
    Texture() = default;
    ~Texture() = default;

    /// <summary>
    /// テクスチャを読み込む
    /// </summary>
    /// <param name="filePath">テクスチャファイルのパス</param>
    /// <param name="dxCommon">DirectXCommonのポインタ</param>
    /// <param name="srvIndex">SRVヒープでのインデックス（ImGui用に+1される）</param>
    /// <returns>読み込み成功かどうか</returns>
    bool LoadTexture(const std::string& filePath, DirectXCommon* dxCommon, uint32_t srvIndex);

    /// <summary>
    /// リソースの取得
    /// </summary>
    ID3D12Resource* GetResource() const { return textureResource_.Get(); }

    /// <summary>
    /// CPU側ディスクリプタハンドルの取得
    /// </summary>
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const { return cpuHandle_; }

    /// <summary>
    /// GPU側ディスクリプタハンドルの取得
    /// </summary>
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const { return gpuHandle_; }

    /// <summary>
    /// テクスチャのメタデータ取得
    /// </summary>
    const DirectX::TexMetadata& GetMetadata() const { return metadata_; }

    /// <summary>
    /// テクスチャが有効かどうか
    /// </summary>
    bool IsValid() const { return textureResource_ != nullptr; }

    /// <summary>
    /// ファイルパスの取得
    /// </summary>
    const std::string& GetFilePath() const { return filePath_; }

private:
    // テクスチャリソース
    Microsoft::WRL::ComPtr<ID3D12Resource> textureResource_;
    // アップロード用の中間リソース（転送完了まで保持が必要）
    Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource_;

    // ディスクリプタハンドル
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle_{};
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle_{};

    // テクスチャの情報
    DirectX::TexMetadata metadata_{};
    std::string filePath_;

    /// <summary>
    /// テクスチャファイルを読み込む（内部関数）
    /// </summary>
    DirectX::ScratchImage LoadTextureFile(const std::string& filePath);

    /// <summary>
    /// テクスチャリソースを作成する（内部関数）
    /// </summary>
    Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(
        const Microsoft::WRL::ComPtr<ID3D12Device>& device,
        const DirectX::TexMetadata& metadata);

    /// <summary>
    /// テクスチャデータをアップロードする（内部関数）
    /// </summary>
    Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(
        Microsoft::WRL::ComPtr<ID3D12Resource> texture,
        const DirectX::ScratchImage& mipImages,
        const Microsoft::WRL::ComPtr<ID3D12Device>& device,
        const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList);

    /// <summary>
    /// SRVを作成する（内部関数）
    /// </summary>
    void CreateSRV(
        const Microsoft::WRL::ComPtr<ID3D12Device>& device,
        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle);
};