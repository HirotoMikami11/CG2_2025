#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <string>

#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "BaseSystem/Logger/Logger.h"
#include "MyMath/MyFunction.h"

#include "OffscreenRenderer/OffscreenTriangle/OffscreenTriangle.h"				// OffscreenTriangleクラスを使用
#include "OffscreenRenderer/PostProcessChain.h"	// ポストプロセスチェーン

#include "OffscreenRenderer/PostEffect/RGBShift/RGBShiftPostEffect.h"	// RGBシフトエフェクト
#include "OffscreenRenderer/PostEffect/LineGlitch/LineGlitchPostEffect.h"	// ラインズラシ
#include "OffscreenRenderer/PostEffect/Grayscale/GrayscalePostEffect.h"	// グレースケールエフェクト
#include "OffscreenRenderer/PostEffect/Vignette/VignettePostEffect.h"	// ビネットエフェクト

#include "OffscreenRenderer/PostEffect/DepthOfField/DepthOfFieldPostEffect.h"	// 深度ぼかしエフェクト
#include "OffscreenRenderer/PostEffect/DepthFog/DepthFogPostEffect.h"	// 深度フォグエフェクト


/// <summary>
/// オフスクリーンレンダリングを管理するクラス
/// ポストプロセスチェーンで複数エフェクトの重ね掛けできる
/// OffscreenTriangle使用版（Sprite脱却）
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
	/// 更新処理（ポストプロセスチェーンの更新など）
	/// </summary>
	/// <param name="deltaTime">フレーム時間</param>
	void Update(float deltaTime = 1.0f / 60.0f);

	/// <summary>
	/// オフスクリーンレンダリング開始
	/// </summary>
	void PreDraw();

	/// <summary>
	/// オフスクリーンレンダリング終了
	/// </summary>
	void PostDraw();

	/// <summary>
	/// オフスクリーンテクスチャを描画（ポストプロセスチェーン対応）
	/// ポストプロセスチェーンを通して複数エフェクトを適用
	void DrawOffscreenTexture();

	/// <summary>
	/// オフスクリーンテクスチャのハンドルを取得
	/// </summary>
	/// <returns>GPUハンドル</returns>
	D3D12_GPU_DESCRIPTOR_HANDLE GetOffscreenTextureHandle() const { return srvHandle_.gpuHandle; }

	/// <summary>
	/// 深度テクスチャのハンドルを取得
	/// </summary>
	/// <returns>深度テクスチャのGPUハンドル</returns>
	D3D12_GPU_DESCRIPTOR_HANDLE GetDepthTextureHandle() const { return depthSrvHandle_.gpuHandle; }

	/// <summary>
	/// オフスクリーンレンダリングが有効かチェック
	/// </summary>
	/// <returns>有効かどうか</returns>
	bool IsValid() const { return renderTargetTexture_ != nullptr; }

	/// <summary>
	/// ImGui表示
	/// </summary>
	void ImGui();


	/// <summary>
	/// RGBShiftを取得
	/// </summary>
	/// <returns>RGBシフトポストエフェクトのポインタ</returns>
	RGBShiftPostEffect* GetRGBShiftEffect() { return RGBShiftEffect_; }

	/// <summary>
	/// ラインずらしを取得
	/// </summary>
	/// <returns>ラインずらしのポインタ</returns>
	LineGlitchPostEffect* GetLineGlitchEffect() { return lineGlitchEffect_; }

	/// <summary>
	/// グレースケールエフェクトを取得
	/// </summary>
	/// <returns>グレースケールポストエフェクトのポインタ</returns>
	GrayscalePostEffect* GetGrayscaleEffect() { return grayscaleEffect_; }

	/// <summary>
	/// ビネットエフェクトを取得
	/// </summary>
	/// <returns>ビネットポストエフェクトのポインタ</returns>
	VignettePostEffect* GetVignetteEffect() { return vignetteEffect_; }

	/// <summary>
	/// 深度ぼかしエフェクトを取得
	/// </summary>
	/// <returns>深度ぼかしポストエフェクトのポインタ</returns>
	DepthOfFieldPostEffect* GetDepthOfFieldEffect() { return depthOfFieldEffect_; }

	/// <summary>
	/// 深度フォグエフェクトを取得
	/// </summary>
	/// <returns>深度フォグポストエフェクトのポインタ</returns>
	DepthFogPostEffect* GetDepthFogEffect() { return depthFogEffect_; }


	/// <summary>
	/// ポストプロセスチェーンを取得
	/// </summary>
	/// <returns>ポストプロセスチェーンのポインタ</returns>
	PostProcessChain* GetPostProcessChain() { return postProcessChain_.get(); }

	/// <summary>
	/// オフスクリーン用OffscreenTriangleを取得（デバッグ用）
	/// </summary>
	/// <returns>オフスクリーン用OffscreenTriangleのポインタ</returns>
	OffscreenTriangle* GetOffscreenTriangle() { return offscreenTriangle_.get(); }

private:
	/// <summary>
	/// レンダーターゲット関連の作成
	/// </summary>
	void CreateRenderTargetTexture();
	void CreateDepthStencilTexture();
	void CreateRTV();
	void CreateDSV();
	void CreateSRV();
	void CreateDepthSRV();  // 深度用SRV作成
	void CreatePSO();

	/// <summary>
	/// オフスクリーン描画用のOffscreenTriangleを初期化
	/// </summary>
	void InitializeOffscreenTriangle();

private:
	// DirectXCommonへの参照
	DirectXCommon* dxCommon_ = nullptr;

	// レンダーターゲットのサイズ
	uint32_t width_ = 0;
	uint32_t height_ = 0;

	// レンダーターゲット用リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> renderTargetTexture_;
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilTexture_;
	// クリアカラー
	//float clearColor_[4] = { 0.05f, 0.05f, 0.05f, 1.0f };  
	float clearColor_[4] = { 0.1f, 0.25f, 0.5f, 1.0f };

	// ディスクリプタハンドル
	DescriptorHeapManager::DescriptorHandle rtvHandle_;
	DescriptorHeapManager::DescriptorHandle dsvHandle_;
	DescriptorHeapManager::DescriptorHandle srvHandle_;
	DescriptorHeapManager::DescriptorHandle depthSrvHandle_;

	//PSO
	Microsoft::WRL::ComPtr<ID3D12RootSignature> offscreenRootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> offscreenPipelineState_;

	// オフスクリーン描画専用OffscreenTriangle（Sprite置き換え）
	std::unique_ptr<OffscreenTriangle> offscreenTriangle_;

	// バリア、ビューポート
	D3D12_RESOURCE_BARRIER barrier_{};
	D3D12_RESOURCE_BARRIER depthBarrier_{};
	D3D12_VIEWPORT viewport_{};
	D3D12_RECT scissorRect_{};

	// ポストプロセスチェーン
	std::unique_ptr<PostProcessChain> postProcessChain_;

	// 個別エフェクトへの参照（設定用）
	RGBShiftPostEffect* RGBShiftEffect_ = nullptr;
	LineGlitchPostEffect* lineGlitchEffect_ = nullptr;
	GrayscalePostEffect* grayscaleEffect_ = nullptr;
	VignettePostEffect* vignetteEffect_ = nullptr;

	DepthFogPostEffect* depthFogEffect_ = nullptr;
	DepthOfFieldPostEffect* depthOfFieldEffect_ = nullptr;
};