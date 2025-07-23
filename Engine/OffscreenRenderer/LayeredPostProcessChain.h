#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include <d3d12.h>
#include <wrl.h>

#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "OffscreenRenderer/PostEffect/PostEffect.h"
#include "Objects/Sprite/Sprite.h"

/// <summary>
/// ポストプロセス適用レイヤー
/// </summary>
enum class PostProcessLayer {
	WORLD_3D,    // 3Dオブジェクト用（深度ベースエフェクト含む）
	UI_SPRITE,   // UI Sprite用（深度ベースエフェクト除外）
	GLOBAL       // 全体エフェクト（レイヤー問わず）
};

/// <summary>
/// レイヤー付きエフェクト情報
/// </summary>
struct LayeredEffectInfo {
	std::unique_ptr<PostEffect> effect;
	PostProcessLayer layer;
	bool enabled = true;
	int priority = 0;  // レイヤー内の順序（小さいほど先に実行）

	LayeredEffectInfo(std::unique_ptr<PostEffect> eff, PostProcessLayer lay, int prio = 0)
		: effect(std::move(eff)), layer(lay), priority(prio) {
	}
};

/// <summary>
/// レイヤー別ポストプロセスエフェクトチェーン管理クラス
/// 3つの独立レイヤーで異なる処理を適用
/// - WORLD_3D: 深度ベースエフェクト対応
/// - UI_SPRITE: 深度ベースエフェクト自動除外
/// - GLOBAL: 全体エフェクト（合成後適用）
/// </summary>
class LayeredPostProcessChain {
public:
	LayeredPostProcessChain() = default;
	~LayeredPostProcessChain() = default;

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
	/// レイヤー別エフェクトチェーンを適用
	/// </summary>
	/// <param name="world3DSRV">3D描画結果のテクスチャ</param>
	/// <param name="uiSpriteSRV">UI描画結果のテクスチャ</param>
	/// <param name="depthSRV">深度テクスチャ</param>
	/// <returns>最終結果のテクスチャハンドル</returns>
	D3D12_GPU_DESCRIPTOR_HANDLE ApplyLayeredEffects(
		D3D12_GPU_DESCRIPTOR_HANDLE world3DSRV,
		D3D12_GPU_DESCRIPTOR_HANDLE uiSpriteSRV,
		D3D12_GPU_DESCRIPTOR_HANDLE depthSRV);

	/// <summary>
	/// 簡易版：単一テクスチャに全レイヤーエフェクトを適用
	/// </summary>
	/// <param name="inputSRV">入力テクスチャ</param>
	/// <param name="depthSRV">深度テクスチャ</param>
	/// <returns>最終結果のテクスチャハンドル</returns>
	D3D12_GPU_DESCRIPTOR_HANDLE ApplyEffectsWithDepth(
		D3D12_GPU_DESCRIPTOR_HANDLE inputSRV,
		D3D12_GPU_DESCRIPTOR_HANDLE depthSRV);

	/// <summary>
	/// エフェクトを追加（自動初期化付き）
	/// </summary>
	template<typename T>
	T* AddEffect(PostProcessLayer layer, int priority = 0) {
		auto effect = std::make_unique<T>();
		T* ptr = effect.get();

		if (dxCommon_) {
			effect->Initialize(dxCommon_);
		}

		effects_.emplace_back(std::move(effect), layer, priority);
		SortEffectsInLayer(layer);
		return ptr;
	}

	/// <summary>
	/// ImGui表示（動的順序変更機能付き）
	/// </summary>
	void ImGui();

	/// <summary>
	/// エフェクトを別レイヤーに移動
	/// </summary>
	/// <param name="effectIndex">エフェクトのインデックス</param>
	/// <param name="newLayer">移動先レイヤー</param>
	void MoveEffectToLayer(size_t effectIndex, PostProcessLayer newLayer);

	/// <summary>
	/// レイヤー内でエフェクトの順序を変更
	/// </summary>
	/// <param name="effectIndex">エフェクトのインデックス</param>
	/// <param name="direction">移動方向（1で下、-1で上）</param>
	void MoveEffectInLayer(size_t effectIndex, int direction);

	/// <summary>
	/// 指定レイヤーの有効なエフェクト数を取得
	/// </summary>
	/// <param name="layer">対象レイヤー</param>
	/// <returns>有効なエフェクト数</returns>
	size_t GetActiveEffectCount(PostProcessLayer layer) const;

	/// <summary>
	/// 指定レイヤーの深度要求エフェクト数を取得
	/// </summary>
	/// <param name="layer">対象レイヤー</param>
	/// <returns>深度要求エフェクト数</returns>
	size_t GetDepthRequiredEffectCount(PostProcessLayer layer) const;

	/// <summary>
	/// エフェクトリストを取得（読み取り専用）
	/// </summary>
	/// <returns>エフェクトリストの参照</returns>
	const std::vector<LayeredEffectInfo>& GetEffects() const { return effects_; }

	/// <summary>
	/// レイヤー名を取得
	/// </summary>
	/// <param name="layer">レイヤー</param>
	/// <returns>レイヤー名</returns>
	static const char* GetLayerName(PostProcessLayer layer);

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

	/// <summary>
	/// 指定レイヤーのエフェクトをソート
	/// </summary>
	/// <param name="layer">対象レイヤー</param>
	void SortEffectsInLayer(PostProcessLayer layer);

	/// <summary>
	/// 指定レイヤーのエフェクトを適用
	/// </summary>
	/// <param name="layer">適用するレイヤー</param>
	/// <param name="inputSRV">入力テクスチャ</param>
	/// <param name="depthSRV">深度テクスチャ（必要な場合）</param>
	/// <returns>処理結果のテクスチャハンドル</returns>
	D3D12_GPU_DESCRIPTOR_HANDLE ApplyLayerEffects(
		PostProcessLayer layer,
		D3D12_GPU_DESCRIPTOR_HANDLE inputSRV,
		D3D12_GPU_DESCRIPTOR_HANDLE depthSRV = {});

	/// <summary>
	/// 2つのテクスチャを合成
	/// </summary>
	/// <param name="baseSRV">ベーステクスチャ</param>
	/// <param name="overlaySRV">オーバーレイテクスチャ</param>
	/// <returns>合成結果のテクスチャハンドル</returns>
	D3D12_GPU_DESCRIPTOR_HANDLE CompositeTextures(
		D3D12_GPU_DESCRIPTOR_HANDLE baseSRV,
		D3D12_GPU_DESCRIPTOR_HANDLE overlaySRV);

	/// <summary>
	/// レイヤー別ImGui描画
	/// </summary>
	/// <param name="layer">対象レイヤー</param>
	void DrawLayerImGui(PostProcessLayer layer);

private:
	// システム参照
	DirectXCommon* dxCommon_ = nullptr;

	// バッファサイズ
	uint32_t width_ = 0;
	uint32_t height_ = 0;

	// レイヤー付きエフェクトリスト
	std::vector<LayeredEffectInfo> effects_;

	// 中間バッファ（レイヤー処理用に4つのバッファを使用）
	static constexpr int kIntermediateBufferCount = 4;
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateBuffers_[kIntermediateBufferCount];

	// 中間バッファ用のハンドル
	DescriptorHeapManager::DescriptorHandle intermediateSRVHandles_[kIntermediateBufferCount];
	DescriptorHeapManager::DescriptorHandle intermediateRTVHandles_[kIntermediateBufferCount];

	// エフェクト描画用Sprite
	std::unique_ptr<Sprite> effectSprite_;

	// 合成用PSO（後で実装）
	Microsoft::WRL::ComPtr<ID3D12RootSignature> compositeRootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> compositePipelineState_;

	// 初期化フラグ
	bool isInitialized_ = false;

	// ImGui用の状態
	int selectedLayer_ = 0;  // 現在選択中のレイヤー
};