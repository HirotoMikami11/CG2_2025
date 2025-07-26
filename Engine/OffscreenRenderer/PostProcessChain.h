#pragma once
#include <vector>
#include <memory>
#include <d3d12.h>
#include <wrl.h>

#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "OffscreenRenderer/PostEffect/PostEffect.h"
#include "OffscreenRenderer/OffscreenTriangle/OffscreenTriangle.h"

/// <summary>
/// ポストプロセスエフェクトチェーン管理クラス
/// 複数のエフェクトを順番に適用する
/// 自動深度テクスチャ判定機能付き
/// OffscreenTriangle使用版
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
	/// 深度テクスチャが必要なエフェクトは自動的にスキップされ警告が出力される
	/// </summary>
	/// <param name="inputSRV">入力テクスチャ</param>
	/// <returns>最終結果のテクスチャハンドル</returns>
	D3D12_GPU_DESCRIPTOR_HANDLE ApplyEffects(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV);

	/// <summary>
	/// エフェクトチェーンを適用（深度テクスチャ対応版）
	/// 全てのエフェクトが適用される（深度が必要なものは自動的に深度版Applyが呼ばれる）
	/// </summary>
	/// <param name="inputSRV">入力カラーテクスチャ</param>
	/// <param name="depthSRV">深度テクスチャ</param>
	/// <returns>最終結果のテクスチャハンドル</returns>
	D3D12_GPU_DESCRIPTOR_HANDLE ApplyEffectsWithDepth(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV, D3D12_GPU_DESCRIPTOR_HANDLE depthSRV);

	/// <summary>
	/// エフェクトを追加（自動初期化付き）
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
	/// <param name="from">移動元のインデックス</param>
	/// <param name="to">移動先のインデックス</param>
	void MoveEffect(size_t from, size_t to);

	/// <summary>
	/// 有効なエフェクトの数を取得
	/// </summary>
	/// <returns>有効なエフェクト数</returns>
	size_t GetActiveEffectCount() const;

	/// <summary>
	/// 深度テクスチャが必要なエフェクトの数を取得
	/// </summary>
	/// <returns>深度テクスチャが必要なエフェクト数</returns>
	size_t GetDepthRequiredEffectCount() const;

	/// <summary>
	/// エフェクトリストを取得（読み取り専用）
	/// </summary>
	/// <returns>エフェクトリストの参照</returns>
	const std::vector<std::unique_ptr<PostEffect>>& GetEffects() const { return effects_; }

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

	// エフェクト描画用OffscreenTriangle（Sprite置き換え）
	std::unique_ptr<OffscreenTriangle> offscreenTriangle_;

	// 初期化フラグ
	bool isInitialized_ = false;
};