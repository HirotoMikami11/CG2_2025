#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <string>

#include "DirectXCommon.h"
#include "Logger.h"
#include "MyMath.h"
#include "MyFunction.h"

#include "GlitchEffect.h"	//グリッチエフェクト
#include "NoiseEffect.h"	

/// <summary>
/// オフスクリーンレンダリングを管理するクラス（DescriptorHeapManager対応版）
/// </summary>
class OffscreenRenderer {
public:
	OffscreenRenderer() = default;
	~OffscreenRenderer() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="width">レンダーターゲットの幅</param>
	/// <param name="height">レンダーターゲットの高さ</param>
	void Initialize(DirectXCommon* dxCommon, uint32_t width = GraphicsConfig::kClientWidth, uint32_t height = GraphicsConfig::kClientHeight);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// 更新処理（エフェクトの更新）
	/// </summary>
	/// <param name="deltaTime">フレーム時間</param>
	void Update(float deltaTime = 1.0f / 60.0f);


	/// <summary>
	/// オフスクリーンレンダリング開始（PreDraw）
	/// </summary>
	void PreDraw();

	/// <summary>
	/// オフスクリーンレンダリング終了（PostDraw）
	/// </summary>
	void PostDraw();

	/// <summary>
	/// オフスクリーンテクスチャを描画（通常の描画パスで使用）
	/// </summary>
	/// <param name="x">描画位置X</param>
	/// <param name="y">描画位置Y</param>
	/// <param name="width">描画幅</param>
	/// <param name="height">描画高さ</param>
	void DrawOffscreenTexture(float x = 0.0f, float y = 0.0f, float width = GraphicsConfig::kClientWidth, float height = GraphicsConfig::kClientHeight);

	/// <summary>
	/// オフスクリーンテクスチャのハンドルを取得
	/// </summary>
	/// <returns>GPUハンドル</returns>
	D3D12_GPU_DESCRIPTOR_HANDLE GetOffscreenTextureHandle() const { return srvHandle_.gpuHandle; }

	/// <summary>
	/// オフスクリーンレンダリングが有効かチェック
	/// </summary>
	/// <returns>有効かどうか</returns>
	bool IsValid() const { return renderTargetTexture_ != nullptr; }

	/// <summary>
	/// グリッチエフェクト用のImGui
	/// </summary>
	void ImGui();

	/// <summary>
	/// グリッチエフェクトを取得
	/// </summary>
	GlitchEffect* GetGlitchEffect() { return glitchEffect_.get(); }

	/// <summary>
	/// グリッチエフェクトを設定
	/// </summary>
	void SetGlitchEffect(std::unique_ptr<GlitchEffect> glitchEffect) {
		glitchEffect_ = std::move(glitchEffect);
	}
	///<summary>
	/// ノイズエフェクトを取得
	/// </summary>
	NoiseEffect* GetNoiseEffect() { return noiseEffect_.get(); }

	/// <summary>
	/// ノイズエフェクトを設定
	/// </summary>
	void SetNoiseEffect(std::unique_ptr<NoiseEffect> noiseEffect) {
		noiseEffect_ = std::move(noiseEffect);
	}
private:
	/// <summary>
	/// レンダーターゲットテクスチャを作成
	/// </summary>
	void CreateRenderTargetTexture();

	/// <summary>
	/// 深度ステンシルテクスチャを作成
	/// </summary>
	void CreateDepthStencilTexture();

	/// <summary>
	/// RTVを作成
	/// </summary>
	void CreateRTV();

	/// <summary>
	/// DSVを作成
	/// </summary>
	void CreateDSV();

	/// <summary>
	/// SRVを作成
	/// </summary>
	void CreateSRV();

	/// <summary>
	/// オフスクリーン描画用のPSOを作成
	/// </summary>
	void CreatePSO();

	/// <summary>
	/// フルスクリーン描画用の頂点バッファを作成
	/// </summary>
	void CreateVertexBuffer();

	/// <summary>
	/// シェーダーをコンパイル
	/// </summary>
	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(
		const std::wstring& filePath,
		const wchar_t* profile);

private:
	// DirectXCommonへの参照
	DirectXCommon* dxCommon_ = nullptr;

	// レンダーターゲットのサイズ
	uint32_t width_ = 0;
	uint32_t height_ = 0;

	// レンダーターゲット用リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> renderTargetTexture_;
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilTexture_;

	// DescriptorHeapManagerからのディスクリプタハンドル
	DescriptorHeapManager::DescriptorHandle rtvHandle_;
	DescriptorHeapManager::DescriptorHandle dsvHandle_;
	DescriptorHeapManager::DescriptorHandle srvHandle_;

	// オフスクリーン描画用PSO
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob_;
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob_;

	// フルスクリーン描画用の頂点バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	VertexData* vertexData_ = nullptr;

	// マテリアル用バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> materialBuffer_;
	MaterialData* materialData_ = nullptr;

	// Transform用バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> transformBuffer_;
	TransformationMatrix* transformData_ = nullptr;

	// バリア
	D3D12_RESOURCE_BARRIER barrier_{};

	// ビューポート（オフスクリーン用）
	D3D12_VIEWPORT viewport_{};
	D3D12_RECT scissorRect_{};

	// グリッチエフェクト
	std::unique_ptr<GlitchEffect> glitchEffect_;
	// ノイズエフェクト
	std::unique_ptr<NoiseEffect> noiseEffect_;
};