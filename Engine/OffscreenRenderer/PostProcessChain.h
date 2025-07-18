#pragma once
#include <vector>
#include <memory>
#include <d3d12.h>
#include <wrl.h>

#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "OffscreenRenderer/PostEffect/PostEffect.h"
#include "Objects/Sprite/Sprite.h"

/// <summary>
/// ポストプロセスエフェクトチェーン管理クラス
/// 複数のエフェクトを順番に適用する
/// </summary>
class PostProcessChain {
public:
	PostProcessChain() = default;
	~PostProcessChain() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="width">処理対象の幅</param>
	/// <param name="height">処理対象の高さ</param>
	void Initialize(DirectXCommon* dxCommon, uint32_t width, uint32_t height);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="deltaTime">フレーム時間</param>
	void Update(float deltaTime);

	/// <summary>
	/// エフェクトチェーンを適用（通常版）
	/// </summary>
	/// <param name="inputSRV">入力テクスチャ</param>
	/// <returns>最終結果のテクスチャハンドル</returns>
	D3D12_GPU_DESCRIPTOR_HANDLE ApplyEffects(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV);

	/// <summary>
	/// エフェクトチェーンを適用（深度テクスチャ対応版）
	/// </summary>
	/// <param name="inputSRV">入力カラーテクスチャ</param>
	/// <param name="depthSRV">深度テクスチャ</param>
	/// <returns>最終結果のテクスチャハンドル</returns>
	D3D12_GPU_DESCRIPTOR_HANDLE ApplyEffectsWithDepth(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV, D3D12_GPU_DESCRIPTOR_HANDLE depthSRV);

	/// <summary>
	/// エフェクトを追加
	/// </summary>
	template<typename T>
	T* AddEffect() {
		auto effect = std::make_unique<T>();
		T* ptr = effect.get();

		if (dxCommon_) {
			effect->Initialize(dxCommon_);
		}

		effects_.push_back(std::move(effect));
		return ptr;
	}

	/// <summary>
	/// ImGui表示
	/// </summary>
	void ImGui();

	/// <summary>
	/// エフェクトの順序を変更
	/// </summary>
	void MoveEffect(size_t from, size_t to);

	/// <summary>
	/// 有効なエフェクトの数を取得
	/// </summary>
	/// <returns>有効なエフェクト数</returns>
	size_t GetActiveEffectCount() const;

private:
	/// <summary>
	/// 中間バッファを作成
	/// </summary>
	void CreateIntermediateBuffers();

	/// <summary>
	/// 中間バッファのSRVを作成
	/// </summary>
	void CreateIntermediateSRVs();

	/// <summary>
	/// 中間バッファのRTVを作成
	/// </summary>
	void CreateIntermediateRTVs();

private:
	// システム参照
	DirectXCommon* dxCommon_ = nullptr;

	// バッファサイズ
	uint32_t width_ = 0;
	uint32_t height_ = 0;

	// エフェクトリスト
	std::vector<std::unique_ptr<PostEffect>> effects_;

	// 中間バッファ（2つのバッファを交互に使用してピンポン処理）
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateBuffers_[2];

	// 中間バッファ用のハンドル
	DescriptorHeapManager::DescriptorHandle intermediateSRVHandles_[2];
	DescriptorHeapManager::DescriptorHandle intermediateRTVHandles_[2];

	// エフェクト描画用Sprite
	std::unique_ptr<Sprite> effectSprite_;

	// 初期化フラグ
	bool isInitialized_ = false;
};