#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <string>

#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "BaseSystem/Logger/Logger.h"
#include "MyMath/MyFunction.h"

#include "Objects/Sprite/Sprite.h"				// Spriteクラスを使用
#include "OffscreenRenderer/PostProcessChain.h"	// ポストプロセスチェーン

#include "OffscreenRenderer/PostEffect/RGBShift/RGBShiftPostEffect.h"	// RGBシフトエフェクト
#include "OffscreenRenderer/PostEffect/Grayscale/GrayscalePostEffect.h"	// グレースケールエフェクト
#include "OffscreenRenderer/PostEffect/LineGlitch/LineGlitchPostEffect.h"	// `らいんずらし



/// <summary>
/// オフスクリーンレンダリングを管理するクラス
/// ポストプロセスチェーンで複数エフェクトの重ね掛けできる
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
	/// 通常のUI用Spriteとは異なる、オフスクリーン専用の描画処理
	/// ポストプロセスチェーンを通して複数エフェクトを適用
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
	/// ImGui表示
	/// </summary>
	void ImGui();

	/// <summary>
	/// グリッチエフェクトを取得（PostProcessChain版）
	/// </summary>
	/// <returns>RGBシフトポストエフェクトのポインタ</returns>
	RGBShiftPostEffect* GetGlitchEffect() { return RGBShiftEffect_; }

	/// <summary>
	/// グレースケールエフェクトを取得
	/// </summary>
	/// <returns>グレースケールポストエフェクトのポインタ</returns>
	GrayscalePostEffect* GetGrayscaleEffect() { return grayscaleEffect_; }

	/// <summary>
	/// ポストプロセスチェーンを取得
	/// </summary>
	/// <returns>ポストプロセスチェーンのポインタ</returns>
	PostProcessChain* GetPostProcessChain() { return postProcessChain_.get(); }

	/// <summary>
	/// オフスクリーン用Spriteを取得（デバッグ用）
	/// </summary>
	/// <returns>オフスクリーン用Spriteのポインタ</returns>
	Sprite* GetOffscreenSprite() { return offscreenSprite_.get(); }

private:
	/// <summary>
	/// レンダーターゲット関連の作成
	/// </summary>
	void CreateRenderTargetTexture();
	void CreateDepthStencilTexture();
	void CreateRTV();
	void CreateDSV();
	void CreateSRV();
	void CreatePSO();

	/// <summary>
	/// オフスクリーン描画用のSpriteを初期化
	/// </summary>
	void InitializeOffscreenSprite();

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

	//PSO
	Microsoft::WRL::ComPtr<ID3D12RootSignature> offscreenRootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> offscreenPipelineState_;


	// オフスクリーン描画専用Sprite（UI用Spriteとは役割が異なる）
	std::unique_ptr<Sprite> offscreenSprite_;

	// バリア、ビューポート
	D3D12_RESOURCE_BARRIER barrier_{};
	D3D12_VIEWPORT viewport_{};
	D3D12_RECT scissorRect_{};

	// ポストプロセスチェーン
	std::unique_ptr<PostProcessChain> postProcessChain_;

	// 個別エフェクトへの参照（設定用）
	RGBShiftPostEffect* RGBShiftEffect_ = nullptr;
	GrayscalePostEffect* grayscaleEffect_ = nullptr;
	LineGlitchPostEffect* lineGlitchEffect_ = nullptr;

};