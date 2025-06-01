#pragma once
#include <Windows.h>
//時間を扱うライブラリ
#include<chrono>
///DirectX12
#include<d3d12.h>
#pragma comment(lib,"d3d12.lib")
#include<dxgi1_6.h>
#pragma comment(lib,"dxgi.lib")
#include <dxgidebug.h>
#pragma comment(lib,"dxguid.lib")
///DirectXTex
#include"externals/DirectXTex/DirectXTex.h"
#include"externals/DirectXTex/d3dx12.h"
// Microsoft::WRL::ComPtrを使用するためのインクルード
#include <wrl.h>

///DXC
#include <dxcapi.h>
#pragma comment(lib,"dxcompiler.lib")


#include<cassert>
#include"Logger.h"
#include"WinApp.h"

/// <summary>
/// DirectX
/// </summary>
class DirectXCommon
{
public:

	void Initialize(WinApp* winApp);
	void Finalize();


	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index);

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index);




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
	/// descriptorHeapの作成
	/// </summary>
	void MakeDescriptorHeap();

	///desctipotorHeapを生成する関数
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDesctiptorHeap(Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);


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
	static Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(
		const std::wstring& filePath,
		const wchar_t* profile,
		Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils,
		Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler,
		Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler);

	/// <summary>
	/// ViewportとScissor
	/// </summary>
	void MakeViewport();

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
	///ゲッター

	ID3D12Device* GetDevice() const { return device.Get(); }
	ID3D12GraphicsCommandList* GetCommandList() const { return commandList.Get(); }
	ID3D12CommandQueue* GetCommandQueue() const { return commandQueue.Get(); }

	HRESULT GetHR() const { return hr; }//後で消す
	// Setter
	void SetHR(HRESULT result) { hr = result; }

	IDXGISwapChain4* GetSwapChain() const { return swapChain.Get(); }
	DXGI_SWAP_CHAIN_DESC1 GetSwapChainDesc() const { return swapChainDesc; }
	ID3D12Resource* GetSwapChainResource(int index) const { return swapChainResources[index].Get(); }
	D3D12_RENDER_TARGET_VIEW_DESC GetRTVDesc() { return rtvDesc; }
	D3D12_CPU_DESCRIPTOR_HANDLE GetRTVHandle(int index) const { return rtvHandles[index]; }
	ID3D12DescriptorHeap* GetSRVDescriptorHeap() const { return srvDescriptorHeap.Get(); }
	//const std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>& GetTextureGPUSrvHandles() const { return textureGPUSrvHandles; }
	//const std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>& GetTextureCPUSrvHandles() const { return textureCPUSrvHandles; }

	// TextureManagerに使用する
	uint32_t GetDescriptorSizeSRV() const { return descriptorSizeSRV; }
	uint32_t GetDescriptorSizeRTV() const { return descriptorSizeRTV; }
	uint32_t GetDescriptorSizeDSV() const { return descriptorSizeDSV; }
	// ComPtr版のゲッター関数（オフスクリーンレンダリング用）
	const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& GetRTVDescriptorHeapComPtr() const { return rtvDescriptorHeap; }
	const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& GetDSVDescriptorHeapComPtr() const { return dsvDescriptorHeap; }


	///参照で返すゲッター？
	const Microsoft::WRL::ComPtr<ID3D12Device>& GetDeviceComPtr() const { return device; }
	const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& GetCommandListComPtr() const { return commandList; }
	const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& GetSRVDescriptorHeapComPtr() const { return srvDescriptorHeap; }
	// オフスクリーンレンダリング用のゲッター関数
	ID3D12RootSignature* GetRootSignature() const { return rootSignature.Get(); }
	ID3D12PipelineState* GetPipelineState() const { return graphicsPipelineState.Get(); }
	ID3D12DescriptorHeap* GetRTVDescriptorHeap() const { return rtvDescriptorHeap.Get(); }
	ID3D12DescriptorHeap* GetDSVDescriptorHeap() const { return dsvDescriptorHeap.Get(); }

	// DXC関連のゲッター
	IDxcUtils* GetDxcUtils() const { return dxcUtils.Get(); }
	IDxcCompiler3* GetDxcCompiler() const { return dxcCompiler.Get(); }
	IDxcIncludeHandler* GetIncludeHandler() const { return includeHandler.Get(); }

private:

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

	uint32_t descriptorSizeSRV = 0;
	uint32_t descriptorSizeRTV = 0;
	uint32_t descriptorSizeDSV = 0;

	// ディスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap;

	// スワップチェーンのバックバッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources[2];

	//rtv
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};//rtvの設定
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	//srv
	//std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> textureResources;
	//std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> intermediateResources;
	//std::vector<DirectX::TexMetadata> metadatas;
	//std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> textureCPUSrvHandles;
	//std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> textureGPUSrvHandles;

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

	//PSO
	//RootSignature作成
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;//シリアライズしてバイナリする
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
	//バイナリをもとにして作成する
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;

	//RasterizerStateの設定  
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob;
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState;//実際に生成されるPSO

	//ビューポート
	D3D12_VIEWPORT viewport{};
	//シザー矩形
	D3D12_RECT scissorRect{};


	//TransitionBarrier
	D3D12_RESOURCE_BARRIER barrier{};
};

