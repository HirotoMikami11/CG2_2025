#pragma once
#include <vector>
#include <memory>
#include <d3d12.h>
#include <wrl.h>
#include "OffscreenRenderer/PostEffect/PostEffect.h"
#include "../DirectXCommon.h"
#include "../Sprite.h"
#include "BaseSystem/Logger/Logger.h"
#include "MyMath/MyFunction.h"
#include "../ImGuiManager.h"

/// <summary>
/// ポストプロセスチェーンを管理するクラス
/// 複数のポストエフェクトを順次適用する
/// </summary>
class PostProcessChain {
public:
	PostProcessChain() = default;
	~PostProcessChain() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, uint32_t width, uint32_t height);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(float deltaTime);

	/// <summary>
	/// エフェクトを追加
	/// </summary>
	template<typename T>
	T* AddEffect() {
		auto effect = std::make_unique<T>();
		effect->Initialize(dxCommon_);
		T* ptr = effect.get();
		effects_.push_back(std::move(effect));
		return ptr;
	}

	/// <summary>
	/// ポストプロセスチェーンを適用
	/// </summary>
	/// <param name="inputSRV">入力テクスチャ</param>
	/// <returns>最終結果のSRV</returns>
	D3D12_GPU_DESCRIPTOR_HANDLE ApplyEffects(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV);

	/// <summary>
	/// エフェクトの順序を変更
	/// </summary>
	void MoveEffect(size_t from, size_t to);

	/// <summary>
	/// ImGui表示
	/// </summary>
	void ImGui();

private:
	/// <summary>
	/// ping-pong用のレンダーターゲットを作成
	/// </summary>
	void CreatePingPongTargets();

	/// <summary>
	/// フルスクリーン描画用のSpriteを作成
	/// </summary>
	void CreateFullscreenSprite();

private:
	DirectXCommon* dxCommon_ = nullptr;
	uint32_t width_ = 0;
	uint32_t height_ = 0;

	// エフェクトリスト
	std::vector<std::unique_ptr<PostEffect>> effects_;

	// ping-pong用レンダーターゲット
	Microsoft::WRL::ComPtr<ID3D12Resource> pingPongTextures_[2];
	DescriptorHeapManager::DescriptorHandle pingPongRTVs_[2];
	DescriptorHeapManager::DescriptorHandle pingPongSRVs_[2];

	// フルスクリーン描画用スプライト
	std::unique_ptr<Sprite> fullscreenSprite_;

	// ビューポートとシザー矩形
	D3D12_VIEWPORT viewport_{};
	D3D12_RECT scissorRect_{};
};
