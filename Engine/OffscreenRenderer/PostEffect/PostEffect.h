#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include "BaseSystem/DirectXCommon/DirectXCommon.h"

/// <summary>
/// ポストエフェクトの基底クラス
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
	/// エフェクトを適用
	/// </summary>
	/// <param name="inputSRV">入力テクスチャのSRV</param>
	/// <param name="outputRTV">出力先のRTV</param>
	/// <param name="renderSprite">描画用スプライト</param>
	virtual void Apply(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV, D3D12_CPU_DESCRIPTOR_HANDLE outputRTV, class Sprite* renderSprite) = 0;

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