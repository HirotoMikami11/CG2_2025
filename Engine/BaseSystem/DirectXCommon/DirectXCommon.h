#pragma once
#include <Windows.h>
#include<chrono>						//時間を扱うライブラリ
///DirectX12
#include<d3d12.h>
#pragma comment(lib,"d3d12.lib")
#include<dxgi1_6.h>
#pragma comment(lib,"dxgi.lib")
#include <dxgidebug.h>
#pragma comment(lib,"dxguid.lib")

///DirectXTex
#include"../externals/DirectXTex/DirectXTex.h"
#include"../externals/DirectXTex/d3dx12.h"
///DXC
#include <dxcapi.h>
#pragma comment(lib,"dxcompiler.lib")

#include <wrl.h>						// Microsoft::WRL::ComPtrを使用するためのインクルード

#include"BaseSystem/WinApp/WinApp.h"
#include"BaseSystem/Logger/Logger.h"
#include"BaseSystem/GraphicsConfig.h"	//ウィンドウサイズなど
#include"BaseSystem/DirectXCommon/DescriptorHeapManager.h"		//ディスクリプタヒープ管理

/// <summary>
/// DirectX
/// </summary>
class DirectXCommon
{
public:

	void Initialize(WinApp* winApp);
	void Finalize();

	/// <summary>
	/// 描画前の処理
	/// </summary>
	void PreDraw();

	/// <summary>
	/// 描画後の処理
	/// </summary>
	void PostDraw();
	/// <summary>
	/// フレーム開始処理（コマンドリストの準備）
	/// </summary>
	void BeginFrame();

	/// <summary>
	/// フレーム終了処理（コマンドリストの実行と次フレーム準備）
	/// </summary>
	void EndFrame();

	/// <summary>
	/// シェーダーをコンパイルする関数
	/// </summary>
	static Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(
		const std::wstring& filePath,
		const wchar_t* profile,
		Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils,
		Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler,
		Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler);


	//*-----------------------------------------------------------------------*//
	//									ゲッター  								//
	//*-----------------------------------------------------------------------*//
	///ゲッター

	ID3D12Device* GetDevice() const { return device.Get(); }
	ID3D12GraphicsCommandList* GetCommandList() const { return commandList.Get(); }
	ID3D12CommandQueue* GetCommandQueue() const { return commandQueue.Get(); }
	IDXGISwapChain4* GetSwapChain() const { return swapChain.Get(); }
	ID3D12RootSignature* GetRootSignature() const { return rootSignature.Get(); }
	ID3D12PipelineState* GetPipelineState() const { return graphicsPipelineState.Get(); }
	ID3D12RootSignature* GetSpriteRootSignature() const { return spriteRootSignature.Get(); }
	ID3D12PipelineState* GetSpritePipelineState() const { return spritePipelineState.Get(); }
	ID3D12RootSignature* GetLineRootSignature() const { return lineRootSignature.Get(); }
	ID3D12PipelineState* GetLinePipelineState() const { return linePipelineState.Get(); }

	///参照で返すゲッター？
	const Microsoft::WRL::ComPtr<ID3D12Device>& GetDeviceComPtr() const { return device; }
	const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& GetCommandListComPtr() const { return commandList; }

	// DXC関連のゲッター
	IDxcUtils* GetDxcUtils() const { return dxcUtils.Get(); }
	IDxcCompiler3* GetDxcCompiler() const { return dxcCompiler.Get(); }
	IDxcIncludeHandler* GetIncludeHandler() const { return includeHandler.Get(); }

	// DescriptorHeapManager関連
	DescriptorHeapManager* GetDescriptorManager() const { return descriptorManager_.get(); }
	ID3D12DescriptorHeap* GetSRVDescriptorHeap() const { return descriptorManager_->GetSRVHeap(); }

	DXGI_SWAP_CHAIN_DESC1 GetSwapChainDesc() const { return swapChainDesc; }
	ID3D12Resource* GetSwapChainResource(int index) const { return swapChainResources[index].Get(); }
	D3D12_RENDER_TARGET_VIEW_DESC GetRTVDesc() { return rtvDesc; }
	D3D12_CPU_DESCRIPTOR_HANDLE GetRTVHandle(int index) const { return rtvHandles[index]; }



private:



	/// <summary>
	/// デバッグレイヤーを作成する
	/// </summary>
	void MakeDebugLayer();

	/// <summary>
	/// DXGIFactoryを作成
	/// </summary>
	void MakeDXGIFactory();

	/// <summary>
	/// コマンドQueue,Allocator,Listの作成
	/// </summary>
	void InitializeCommand();

	/// <summary>
	/// SwapChainの作成
	/// </summary>
	void MakeSwapChain(WinApp* winApp);

	/// <summary>
	/// RTVを作成する
	/// </summary>
	void MakeRTV();

	//SRVはテクスチャの部分で作成

	/// <summary>
	/// DSVを作成する
	/// </summary>
	void MakeDSV();
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height);

	/// <summary>
	/// フェンスとイベントを作成する
	/// </summary>
	void MakeFenceEvent();

	/// <summary>
	/// DXC
	/// </summary>
	void InitalizeDXC();

	/// <summary>
	/// PSOを作成する
	/// </summary>
	void MakePSO();

	/// <summary>
	/// 2D用のPSOを作成する
	/// </summary>
	void MakeSpritePSO();

	/// <summary>
	/// 線分描画用のPSOを作成する
	/// </summary>
	void MakeLinePSO();

	/// <summary>
	/// ViewportとScissor
	/// </summary>
	void MakeViewport();



#ifdef _DEBUG
	///デバッグレイヤー
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController;
#endif
	//DXGIFactory
	HRESULT hr;
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter;
	Microsoft::WRL::ComPtr<ID3D12Device> device;

	//initailzeCommand
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;

	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	// スワップチェーンのバッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources[2];

	//rtv
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};//rtvの設定
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];

	//dsv
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;

	//Fence
	Microsoft::WRL::ComPtr<ID3D12Fence> fence;
	uint64_t fenceValue;
	HANDLE fenceEvent;

	//DXC
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler;

	//3D用PSO
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState;

	//スプライト用PSO
	Microsoft::WRL::ComPtr<ID3D12RootSignature> spriteRootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> spritePipelineState;

	//線分用PSO
	Microsoft::WRL::ComPtr<ID3D12RootSignature> lineRootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> linePipelineState;


	//ビューポート
	D3D12_VIEWPORT viewport{};
	//シザー矩形
	D3D12_RECT scissorRect{};
	//TransitionBarrier
	D3D12_RESOURCE_BARRIER barrier{};

	//ディスクリプタヒープの管理をする
	std::unique_ptr<DescriptorHeapManager> descriptorManager_;

};

