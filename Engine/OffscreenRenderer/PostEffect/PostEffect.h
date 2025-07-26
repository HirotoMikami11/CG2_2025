#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "OffscreenRenderer/OffscreenTriangle/OffscreenTriangle.h"

/// <summary>
/// ポストエフェクトの基底クラス
/// 自動深度テクスチャ判定機能付き
/// OffscreenTriangle使用版
/// </summary>
class PostEffect {
public:
	virtual ~PostEffect() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	virtual void Initialize(DirectXCommon* dxCommon) = 0;

	/// <summary>
	/// 終了処理
	/// </summary>
	virtual void Finalize() = 0;

	/// <summary>
	/// 更新処理
	/// </summary>
	virtual void Update(float deltaTime) = 0;

	/// <summary>
	/// エフェクトを適用（通常版）
	/// </summary>
	/// <param name="inputSRV">入力テクスチャのSRV</param>
	/// <param name="outputRTV">出力先のRTV</param>
	/// <param name="renderTriangle">描画用三角形</param>
	virtual void Apply(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV, D3D12_CPU_DESCRIPTOR_HANDLE outputRTV, OffscreenTriangle* renderTriangle) = 0;

	/// <summary>
	/// エフェクトを適用（深度テクスチャ版）
	/// 深度テクスチャが不要なエフェクトは通常版のApplyを呼ぶデフォルト実装
	/// 深度テクスチャが必要なエフェクトはこのメソッドをオーバーライドする
	/// </summary>
	/// <param name="inputSRV">入力カラーテクスチャのSRV</param>
	/// <param name="depthSRV">深度テクスチャのSRV</param>
	/// <param name="outputRTV">出力先のRTV</param>
	/// <param name="renderTriangle">描画用三角形</param>
	virtual void Apply(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV,
		D3D12_GPU_DESCRIPTOR_HANDLE depthSRV,
		D3D12_CPU_DESCRIPTOR_HANDLE outputRTV,
		OffscreenTriangle* renderTriangle) {

		// デフォルト実装：通常のApplyを呼ぶ（深度テクスチャは無視）
		Apply(inputSRV, outputRTV, renderTriangle);
	}

	/// <summary>
	/// このエフェクトが深度テクスチャを必要とするかどうか
	/// 深度テクスチャが必要なエフェクトはこのメソッドをオーバーライドしてtrueを返す
	/// </summary>
	/// <returns>深度テクスチャが必要な場合はtrue</returns>
	virtual bool RequiresDepthTexture() const { return false; }

	/// <summary>
	/// エフェクトが有効かどうか
	/// </summary>
	virtual bool IsEnabled() const = 0;

	/// <summary>
	/// エフェクトの有効/無効を設定
	/// </summary>
	virtual void SetEnabled(bool enabled) = 0;

	/// <summary>
	/// ImGui表示
	/// </summary>
	virtual void ImGui() = 0;

	/// <summary>
	/// エフェクト名を取得
	/// </summary>
	virtual const std::string& GetName() const = 0;

protected:
	DirectXCommon* dxCommon_ = nullptr;
	bool isEnabled_ = false;
	std::string name_ = "Unknown Effect";
};