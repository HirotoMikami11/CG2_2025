#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <cassert>
#include <optional>

#include "GraphicsConfig.h"
#include "Logger.h"

/// <summary>
/// ディスクリプタヒープの管理を行うクラス
/// RTVヒープ、DSVヒープ、SRVヒープの統一管理を行う
/// </summary>
class DescriptorHeapManager {
public:
	/// <summary>
	/// ディスクリプタヒープの種別
	/// </summary>
	enum class HeapType {
		RTV,		// レンダーターゲットビュー
		DSV,		// デプスステンシルビュー 
		SRV			// シェーダーリソースビュー
	};

	/// <summary>
	/// ディスクリプタハンドル情報
	/// </summary>
	struct DescriptorHandle {
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;  // SRVのみ有効にしたい
		uint32_t index;
		bool isValid;

		DescriptorHandle() : cpuHandle{}, gpuHandle{}, index(0), isValid(false) {}
		DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE cpu, D3D12_GPU_DESCRIPTOR_HANDLE gpu, uint32_t idx)
			: cpuHandle(cpu), gpuHandle(gpu), index(idx), isValid(true) {
		}
	};

public:
	DescriptorHeapManager() = default;
	~DescriptorHeapManager() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="device">D3D12デバイス</param>
	void Initialize(Microsoft::WRL::ComPtr<ID3D12Device> device);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	//=============================================================================
	// ディスクリプタヒープ取得
	//=============================================================================

	/// <summary>
	/// RTVヒープを取得
	/// </summary>
	ID3D12DescriptorHeap* GetRTVHeap() const { return rtvHeap_.Get(); }
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetRTVHeapComPtr() const { return rtvHeap_; }

	/// <summary>
	/// DSVヒープを取得
	/// </summary>
	ID3D12DescriptorHeap* GetDSVHeap() const { return dsvHeap_.Get(); }
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetDSVHeapComPtr() const { return dsvHeap_; }

	/// <summary>
	/// SRVヒープを取得
	/// </summary>
	ID3D12DescriptorHeap* GetSRVHeap() const { return srvHeap_.Get(); }
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetSRVHeapComPtr() const { return srvHeap_; }

	//=============================================================================
	// ディスクリプタサイズ取得
	//=============================================================================

	uint32_t GetRTVDescriptorSize() const { return rtvDescriptorSize_; }
	uint32_t GetDSVDescriptorSize() const { return dsvDescriptorSize_; }
	uint32_t GetSRVDescriptorSize() const { return srvDescriptorSize_; }

	//=============================================================================
	// ディスクリプタ割り当て・解放
	//=============================================================================

	/// <summary>
	/// RTVディスクリプタを割り当て
	/// </summary>
	/// <returns>割り当てられたディスクリプタハンドル（失敗時はisValid=false）</returns>
	DescriptorHandle AllocateRTV();

	/// <summary>
	/// DSVディスクリプタを割り当て
	/// </summary>
	/// <returns>割り当てられたディスクリプタハンドル（失敗時はisValid=false）</returns>
	DescriptorHandle AllocateDSV();

	/// <summary>
	/// SRVディスクリプタを割り当て
	/// </summary>
	/// <returns>割り当てられたディスクリプタハンドル（失敗時はisValid=false）</returns>
	DescriptorHandle AllocateSRV();

	/// <summary>
	/// RTVディスクリプタを解放
	/// </summary>
	/// <param name="index">解放するインデックス</param>
	void ReleaseRTV(uint32_t index);

	/// <summary>
	/// DSVディスクリプタを解放
	/// </summary>
	/// <param name="index">解放するインデックス</param>
	void ReleaseDSV(uint32_t index);

	/// <summary>
	/// SRVディスクリプタを解放
	/// </summary>
	/// <param name="index">解放するインデックス</param>
	void ReleaseSRV(uint32_t index);

	//=============================================================================
	// 特定インデックスでの割り当て（予約済みスロット用）
	//=============================================================================

	/// <summary>
	/// 指定インデックスでRTVを予約
	/// </summary>
	/// <param name="index">予約するインデックス</param>
	/// <returns>成功時はディスクリプタハンドル</returns>
	std::optional<DescriptorHandle> ReserveRTV(uint32_t index);

	/// <summary>
	/// 指定インデックスでDSVを予約
	/// </summary>
	/// <param name="index">予約するインデックス</param>
	/// <returns>成功時はディスクリプタハンドル</returns>
	std::optional<DescriptorHandle> ReserveDSV(uint32_t index);

	/// <summary>
	/// 指定インデックスでSRVを予約
	/// </summary>
	/// <param name="index">予約するインデックス</param>
	/// <returns>成功時はディスクリプタハンドル</returns>
	std::optional<DescriptorHandle> ReserveSRV(uint32_t index);

	//=============================================================================
	// ユーティリティ関数
	//=============================================================================

	/// <summary>
	/// 指定したインデックスのCPUハンドルを取得
	/// </summary>
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(HeapType type, uint32_t index) const;

	/// <summary>
	/// 指定したインデックスのGPUハンドルを取得（SRVのみ）
	/// </summary>
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(HeapType type, uint32_t index) const;

	/// <summary>
	/// 使用可能なディスクリプタ数を取得
	/// </summary>
	uint32_t GetAvailableCount(HeapType type) const;

	/// <summary>
	/// 使用中のディスクリプタ数を取得
	/// </summary>
	uint32_t GetUsedCount(HeapType type) const;

	/// <summary>
	/// 指定したインデックスが使用中かチェック
	/// </summary>
	bool IsIndexUsed(HeapType type, uint32_t index) const;

private:
	//=============================================================================
	// 内部関数
	//=============================================================================

	/// <summary>
	/// ディスクリプタヒープを作成
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
		D3D12_DESCRIPTOR_HEAP_TYPE type,
		uint32_t numDescriptors,
		bool shaderVisible);

	/// <summary>
	/// 空いているインデックスを検索
	/// </summary>
	std::optional<uint32_t> FindAvailableIndex(const std::vector<bool>& usedFlags) const;

	/// <summary>
	/// インデックスの有効性をチェック
	/// </summary>
	bool IsValidIndex(HeapType type, uint32_t index) const;

private:
	// D3D12デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device_;

	// ディスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap_;

	// ディスクリプタサイズ
	uint32_t rtvDescriptorSize_ = 0;
	uint32_t dsvDescriptorSize_ = 0;
	uint32_t srvDescriptorSize_ = 0;

	// 使用状況管理
	std::vector<bool> rtvUsedFlags_;
	std::vector<bool> dsvUsedFlags_;
	std::vector<bool> srvUsedFlags_;

	// 初期化フラグ
	bool isInitialized_ = false;
};