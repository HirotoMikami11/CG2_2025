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
	static ID3D12DescriptorHeap* CreateDesctiptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

	/// <summary>
	/// SRVを作成する
	/// </summary>
	/// <param name="textureFileNames"></param>
	void MakeSRV(const std::vector<std::string>& textureFileNames);

	/// <summary>
	/// RTVを作成する
	/// </summary>
	void MakeRTV();
	//static ID3D12Resource* CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);			MYFunctionに移動
	static DirectX::ScratchImage LoadTexture(const std::string& filePath);
	static ID3D12Resource* CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata);
	[[nodiscard]]
	static ID3D12Resource* UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages, ID3D12Device* device, ID3D12GraphicsCommandList* commandList);

	/// <summary>
	/// DSVを作成する
	/// </summary>
	void MakeDSV();
	ID3D12Resource* CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height);


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
	static IDxcBlob* CompileShader(
		const std::wstring& filePath,
		const wchar_t* profile,
		IDxcUtils* dxcUtils,
		IDxcCompiler3* dxcCompiler,
		IDxcIncludeHandler* includeHandler);



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

	///ゲッター

	ID3D12Device* GetDevice() const { return device; }
	ID3D12GraphicsCommandList* GetCommandList() const { return commandList; }
	ID3D12CommandQueue* GetCommandQueue() const { return commandQueue; }

	HRESULT GetHR() const { return hr; }//後で消す
	// Setter
	void SetHR(HRESULT result) { hr = result; }



	IDXGISwapChain4* GetSwapChain() const { return swapChain; }
	DXGI_SWAP_CHAIN_DESC1 GetSwapChainDesc() const { return swapChainDesc; }
	ID3D12Resource* GetSwapChainResource(int index) const { return swapChainResources[index]; }
	D3D12_RENDER_TARGET_VIEW_DESC GetRTVDesc() { return rtvDesc; }
	D3D12_CPU_DESCRIPTOR_HANDLE GetRTVHandle(int index) const { return rtvHandles[index]; }
	ID3D12DescriptorHeap* GetSRVDescriptorHeap() const { return srvDescriptorHeap; }
	const std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>& GetTextureSrvHandles() const { return textureSrvHandles; }

private:

#ifdef _DEBUG
	///デバッグレイヤー
	ID3D12Debug1* debugController = nullptr;
#endif
	//DXGIFactory
	HRESULT hr;
	IDXGIFactory7* dxgiFactory = nullptr;
	IDXGIAdapter4* useAdapter = nullptr;
	ID3D12Device* device = nullptr;

	//initailzeCommand
	ID3D12CommandQueue* commandQueue = nullptr;
	ID3D12CommandAllocator* commandAllocator = nullptr;
	ID3D12GraphicsCommandList* commandList = nullptr;

	IDXGISwapChain4* swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};

	uint32_t descriptorSizeSRV = 0;
	uint32_t descriptorSizeRTV = 0;
	uint32_t descriptorSizeDSV = 0;

	// ディスクリプタヒープ
	ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;
	ID3D12DescriptorHeap* srvDescriptorHeap = nullptr;

	// スワップチェーンのバックバッファ
	ID3D12Resource* swapChainResources[2] = { nullptr };

	//rtv
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};//rtvの設定
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	//srv
	std::vector<ID3D12Resource*> textureResources;
	std::vector<ID3D12Resource*> intermediateResources;
	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> textureSrvHandles;

	//dsv
	ID3D12Resource* depthStencilResource = nullptr;
	ID3D12DescriptorHeap* dsvDescriptorHeap = nullptr;

	//Fence
	ID3D12Fence* fence = nullptr;
	uint64_t fenceValue;
	HANDLE fenceEvent;


	//DXC
	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	IDxcIncludeHandler* includeHandler = nullptr;

	//PSO
	//RootSignature作成
	ID3DBlob* signatureBlob = nullptr;//シリアライズしてバイナリする
	ID3DBlob* errorBlob = nullptr;
	//バイナリをもとにして作成する
	ID3D12RootSignature* rootSignature = nullptr;

	//RasterizerStateの設定	
	IDxcBlob* vertexShaderBlob = nullptr;
	IDxcBlob* pixelShaderBlob = nullptr;
	ID3D12PipelineState* graphicsPipelineState = nullptr;//実際に生成されるPSO

	//ビューポート
	D3D12_VIEWPORT viewport{};
	//シザー矩形
	D3D12_RECT scissorRect{};


	//TransitionBarrier
	D3D12_RESOURCE_BARRIER barrier{};
};

