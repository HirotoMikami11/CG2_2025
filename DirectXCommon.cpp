#include "DirectXCommon.h"

void DirectXCommon::Initialize(WinApp* winApp) {

	//あとでイニシャライズに入れる
// テクスチャファイル名配列
	std::vector<std::string> textureFileNames = {
		"resources/uvChecker.png",
		"resources/monsterBall.png"
	};


	///*-----------------------------------------------------------------------*///
	//																			//
	///									DebugLayer							   ///
	//																			//
	///*-----------------------------------------------------------------------*///
	 MakeDebugLayer();

	///*-----------------------------------------------------------------------*///
	//																			//
	///									DXGIFactory							   ///
	//																			//
	///*-----------------------------------------------------------------------*///
	 MakeDXGIFactory();

	///*-----------------------------------------------------------------------*///
	//																			//
	///					CommandQueueとCommandListを生成						   ///
	//																			//
	///*-----------------------------------------------------------------------*///
	 InitializeCommand();

	///*-----------------------------------------------------------------------*///
	//																			//
	///							SwapChainを生成								   ///
	//																			//
	///*-----------------------------------------------------------------------*///
	 MakeSwapChain(winApp);

	///*-----------------------------------------------------------------------*///
	//																			//
	///							DescriptorHeapを生成							   ///
	//																			//
	///*-----------------------------------------------------------------------*///

	 MakeDescriptorHeap();

	///*-----------------------------------------------------------------------*///
	//																			//
	///									RTVを生成							   ///
	//																			//
	///*-----------------------------------------------------------------------*///
	 MakeRTV();

	///*-----------------------------------------------------------------------*///
	//																			//
	///									SRVを生成							   ///
	//																			//
	///*-----------------------------------------------------------------------*///

	 MakeSRV(textureFileNames);

	///*-----------------------------------------------------------------------*///
	//																			//
	///									DSVを生成								///
	//																			//
	///*-----------------------------------------------------------------------*///
	 MakeDSV();

	///*-----------------------------------------------------------------------*///
	//																			//
	///							FenceとEventを生成する							///
	//																			//
	///*-----------------------------------------------------------------------*///

	 MakeFenceEvent();


	///*-----------------------------------------------------------------------*///
	//																			//
	///									DXCの初期化								///
	//																			//
	///*-----------------------------------------------------------------------*///
	 InitalizeDXC();
	///*-----------------------------------------------------------------------*///
	//																			//
	///								PSOを生成する									///
	//																			//
	///*-----------------------------------------------------------------------*///
	 MakePSO();


	///*-----------------------------------------------------------------------*///
	//																			//
	///							ViewportとScissor							   ///
	//																			//
	///*-----------------------------------------------------------------------*///
	 MakeViewport();


}

void DirectXCommon::Finalize() {
	graphicsPipelineState->Release();
	signatureBlob->Release();
	if (errorBlob) {
		errorBlob->Release();
	}
	rootSignature->Release();
	pixelShaderBlob->Release();
	vertexShaderBlob->Release();

	// テクスチャリソースの解放
	for (ID3D12Resource* resource : textureResources) {
		if (resource) {
			resource->Release();
		}
	}
	textureResources.clear();
	for (ID3D12Resource* resource : intermediateResources) {
		if (resource) {
			resource->Release();
		}
	}
	intermediateResources.clear();

	depthStencilResource->Release();

#ifdef _DEBUG
	debugController->Release();
#endif // _DEBUG

	CloseHandle(fenceEvent);
	fence->Release();
	rtvDescriptorHeap->Release();
	srvDescriptorHeap->Release();
	dsvDescriptorHeap->Release();
	for (int i = 0; i <= 1; i++) {//0,1
		swapChainResources[i]->Release();
	}
	swapChain->Release();
	commandList->Release();
	commandAllocator->Release();
	commandQueue->Release();

	device->Release();
	useAdapter->Release();
	dxgiFactory->Release();





}


///*-----------------------------------------------------------------------*///
//																			//
///									DebugLayer							   ///
//																			//
///*-----------------------------------------------------------------------*///
void DirectXCommon::MakeDebugLayer()
{


#ifdef _DEBUG
	///DirectX12初期化前に行う必要があるため、ウィンドウを作った後すぐの位置
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		//デバッグレイヤーを有効化する
		debugController->EnableDebugLayer();
		//さらにGPU側でもチェックを行うにする
		debugController->SetEnableGPUBasedValidation(TRUE);
	}

#endif

}

///*-----------------------------------------------------------------------*///
//																			//
///									DXGIFactory							   ///
//																			//
///*-----------------------------------------------------------------------*///
void DirectXCommon::MakeDXGIFactory()
{

	//HRESULTはwindows系のエラーコードであり、
	//関数が成功したかどうかをSUCEEDEDマクロで判定できる
	hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	//初期化の根本的な部分でエラーが出た場合はプログラムが間違っているか、どうにもできない場合が多いため、assertにする
	assert(SUCCEEDED(hr));


	///
	///	使用するアダプタ（大まかにGPU）を決定
	/// 

	//使用するアダプタ用の変数、最初はnullptrを入れる
	useAdapter = nullptr;
	//良い順にアダプタを摘む
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i,
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) !=
		DXGI_ERROR_NOT_FOUND; ++i) {
		//アダプターの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));//取得できないと一大事なのでassert
		//ソフトウェアアダプタでなければ採用する
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)) {
			//採用したアダプタの情報をログに出力。wstringの方なので注意

			///コンバートストリングしてstr変化
			Logger::Log(Logger::GetStream(), Logger::ConvertString(std::format(L"Use Adapater:{}\n", adapterDesc.Description)));
			break;
		}
		useAdapter = nullptr;//ソフトウェアアダプタの場合は見なかったことにする


	}
	//適切なアダプタが見つからなかったので起動できない
	assert(useAdapter != nullptr);

	//																			//
	//							D3D12Deviceの生成								//
	//																			//

	device = nullptr;
	//機能レベルとログ出力用の文字列
	D3D_FEATURE_LEVEL featureLevels[] = {
D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
	};

	const char* featureLevelStrings[] = { "12.2","12.1","12.0" };
	//高い順に生成できるか試していく
	for (size_t i = 0; i < _countof(featureLevels); ++i) {
		//採用したアダプターでデバイスを生成
		hr = D3D12CreateDevice(useAdapter, featureLevels[i], IID_PPV_ARGS(&device));
		//指定した機能レベルでデバイスが生成できたかを確認
		if (SUCCEEDED(hr)) {
			//生成できたのでログ出力を行ってループを抜ける
			Logger::Log(Logger::GetStream(), std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
			break;
		}
	}
	//デバイスの生成がうまくいかなかったので起動できない。
	assert(device != nullptr);
	Logger::Log(Logger::GetStream(), "Complete create D3D12Device!!\n");//初期化完了のログを出す



	///	DirectX12のエラー・警告が出た時止まるようにする

#ifdef _DEBUG	//デバッグ時
	ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		// ヤバイエラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		// エラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		//警告時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true); //解放忘れが判明した時、一時的にコメントアウトすれば、ログに情報を出力できる。確認できたらすぐ元に戻す。
		//解放
		infoQueue->Release();

		//抑制するメッセージのID
		D3D12_MESSAGE_ID denyIds[] = {
			//Windows11のDXGIデバッグレイヤーとDX12デバッグレイヤーの相互作用バグによるエラーメッセージ
		D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		//抑制するレベル
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		//指定したメッセージの表示を抑制する
		infoQueue->PushStorageFilter(&filter);

	}
#endif
}

///*-----------------------------------------------------------------------*///
//																			//
///					CommandQueueとCommandListを生成						   ///
//																			//
///*-----------------------------------------------------------------------*///
void DirectXCommon::InitializeCommand()
{


	//コマンドキュー(GPUに命令をするもの)を作成
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	//コマンドキューの生成が上手くいかなかったので起動できない
	assert(SUCCEEDED(hr));
	Logger::Log(Logger::GetStream(), "Complete create commandQueue!!\n");//コマンドキュー生成完了のログを出す


	//コマンドアロケータ（コマンドリスト（まとまった命令郡）保存用のメモリ管理するもの）
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	//コマンドアロケータの生成が上手くいかなかったので起動できない
	assert(SUCCEEDED(hr));
	Logger::Log(Logger::GetStream(), "Complete create commandAllocator!!\n");//コマンドアロケータ生成完了のログを出す


	// コマンドリスト（まとまった命令郡）を生成する
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList));
	//コマンドリストの生成がうまくいかなかったので起動できない 
	assert(SUCCEEDED(hr));
	Logger::Log(Logger::GetStream(), "Complete create commandList!!\n");//コマンドリスト生成完了のログを出す

}

void DirectXCommon::MakeSwapChain(WinApp* winApp)
{

	//スワップチェーン（バブルバッファリングの２枚の画面を管理するもの）を生成する
	swapChain = nullptr;
	swapChainDesc.Width = kClientWidth;								//画面の幅、ウィンドウのクライアント領域を同じものにしておく
	swapChainDesc.Height = kClientHeight;							//画面の高さ、ウィンドウのクライアント領域を同じものにしておく
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;				//色の形式
	swapChainDesc.SampleDesc.Count = 1;								//マルチサンプル市内
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	//描画のターゲットとして利用する
	swapChainDesc.BufferCount = 2;									//ダブルバッファ
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;		//モニタに移したら、中身を破棄
	//コマンドキュー、ウィンドウハンドル、設定を渡して生成する
	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue, winApp->GetHwnd(), &swapChainDesc, nullptr, nullptr,
		reinterpret_cast<IDXGISwapChain1**>(&swapChain));
	//スワップチェーンの生成がうまくいかなかったので起動できない 
	assert(SUCCEEDED(hr));
	Logger::Log(Logger::GetStream(), "Complete create swapChain!!\n");//スワップチェーン生成完了のログを出す

}

void DirectXCommon::MakeDescriptorHeap()
{


	//DescriptorSizeを取得する（ゲームの実行中に変化することがないため、この時点で）
	descriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	descriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);


	//RTV用ヒープでディスクリプタの数は2，RTVはShader内で触れるものではないのでShaderVisibleはfalse
	rtvDescriptorHeap = CreateDesctiptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
	Logger::Log(Logger::GetStream(), "Complete create RTVDescriptorHeap!!\n");//RTVディスクリプタヒープ生成完了のログを出す

	//SRV用ヒープでディスクリプタの数は128，SRVはShader内で触れるものなのでShaderVisibleはtrue
	srvDescriptorHeap = CreateDesctiptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
	Logger::Log(Logger::GetStream(), "Complete create SRVDescriptorHeap!!\n");//SRVディスクリプタヒープ生成完了のログを出す


	///　SwapChainからResourceを引っ張ってくる
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
	//　うまく取得できなければ起動できない
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	assert(SUCCEEDED(hr));

}

/// <summary>
/// DescriptorHeapを作成する関数
/// </summary>
/// <param name="device"ID3D12Device*</param>
/// <param name="heapType">作成するヒープの種類</param>
/// <param name="numDescriptors">ディスクリプタ数</param>
/// <param name="shaderVisible">シェーダーから参照可能にするかのフラグ</param>
/// <returns></returns>
ID3D12DescriptorHeap* DirectXCommon::CreateDesctiptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible) {
	//ディスクリプタヒープの生成
	ID3D12DescriptorHeap* descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	// ディスクリプタヒープが作れなかったので起動できない
	assert(SUCCEEDED(hr));
	return descriptorHeap;
}

void DirectXCommon::MakeRTV()
{

	//RTVの設定
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;			//出力結果をSRGBに変換して書き込む
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;		//2Dテクスチャとして書き込む
	//ディスクリプタの先頭を取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	//RTVを2つ作るのでディスクリプタを2つ用意
	//1つ目は最初のところに作る。作る場所を指定してあげる必要がある
	rtvHandles[0] = rtvStartHandle;
	device->CreateRenderTargetView(swapChainResources[0], &rtvDesc, rtvHandles[0]);
	//2つ目のディスクリプタハンドルを得る（自力で）
	rtvHandles[1].ptr = rtvHandles[0].ptr + descriptorSizeRTV;
	//2つ目を作る
	device->CreateRenderTargetView(swapChainResources[1], &rtvDesc, rtvHandles[1]);

}

void DirectXCommon::MakeSRV(const std::vector<std::string>& textureFileNames)
{

	//SRVを作成するDescriptorHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	// それぞれのテクスチャに対して処理を行う
	for (size_t i = 0; i < textureFileNames.size(); ++i) {
		// テクスチャを読み込んで転送する
		DirectX::ScratchImage mipImages = LoadTexture(textureFileNames[i]);
		const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
		ID3D12Resource* textureResource = CreateTextureResource(device, metadata);
		ID3D12Resource* intermediateResource = UploadTextureData(textureResource, mipImages, device, commandList);

		// SRV設定
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = metadata.format;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

		// CPU・GPUハンドルをずらしてセット
		D3D12_CPU_DESCRIPTOR_HANDLE currentCpuHandle = textureSrvHandleCPU;
		currentCpuHandle.ptr += descriptorSizeSRV * (i + 1); // ImGuiが先頭を使ってるので i+1
		D3D12_GPU_DESCRIPTOR_HANDLE currentGpuHandle = textureSrvHandleGPU;
		currentGpuHandle.ptr += descriptorSizeSRV * (i + 1);

		// SRVの生成
		device->CreateShaderResourceView(textureResource, &srvDesc, currentCpuHandle);
		// 解放するときのために
		textureResources.push_back(textureResource);
		intermediateResources.push_back(intermediateResource);
		textureSrvHandles.push_back(currentGpuHandle);
	}
}

DirectX::ScratchImage DirectXCommon::LoadTexture(const std::string& filePath)
{
	//テクスチャファイルを読んでプログラムで扱える
	DirectX::ScratchImage image{};
	std::wstring filePathW = Logger::ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	// ミップマップ
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0,
		mipImages);
	assert(SUCCEEDED(hr));
	// ミップマップ付きのデータを返す
	return mipImages;
}

ID3D12Resource* DirectXCommon::CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata)
{
	//metadataを基にResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{ };
	resourceDesc.Width = UINT(metadata.width);								//textureの幅
	resourceDesc.Height = UINT(metadata.height);							//textureの高さ
	resourceDesc.MipLevels = UINT16(metadata.mipLevels);					//mipmapの数
	resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize);				//奥行 or配列Textureの配列数
	resourceDesc.Format = metadata.format;									//textureのFormat
	resourceDesc.SampleDesc.Count = 1;										//サンプリングカウント。1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);	//textureの次元数

	//利用するHeapの設定（非常に特殊な運用方法）
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;							//細かい設定を行う

	//Resourceの生成
	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,							//heapの設定
		D3D12_HEAP_FLAG_NONE,						//heapの特殊な設定。特になし
		&resourceDesc,								//resourceの設定
		D3D12_RESOURCE_STATE_COPY_DEST,			//初回のresourceState。textureは基本読むだけ
		nullptr,									//clear最適地。使わないのでnullptr
		IID_PPV_ARGS(&resource));					//作成するResourceポインタへのポインタ
	assert(SUCCEEDED(hr));
	return resource;

}

ID3D12Resource* DirectXCommon::UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages, ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
	std::vector<D3D12_SUBRESOURCE_DATA> subresource;
	DirectX::PrepareUpload(device, mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresource);
	uint64_t intermediateSize = GetRequiredIntermediateSize(texture, 0, UINT(subresource.size()));
	ID3D12Resource* intermediateResource = CreateBufferResource(device, intermediateSize);
	UpdateSubresources(commandList, texture, intermediateResource, 0, 0, UINT(subresource.size()), subresource.data());
	//textureへの転送後は利用できるよう、D3D12_RESOURCE_STATE_COPY_DESTからD3D12_RESOURCE_STATE_GENERIC_READへResourceStateを変更する
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	commandList->ResourceBarrier(1, &barrier);
	return intermediateResource;


}

void DirectXCommon::MakeDSV()
{

	//DepthStencilTextureをウィンドウサイズで作成
	depthStencilResource = CreateDepthStencilTextureResource(device, kClientWidth, kClientHeight);
	//DSV用のヒープでディスクリプタヒープの数は1。DSVのShader内で触れないのでfalse
	dsvDescriptorHeap = CreateDesctiptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
	//DSVの設定
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;				//Formatは基本的にResourceに合わせる
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;		//2Dtexture
	//DSVHeapの先頭にDSVをつくる
	device->CreateDepthStencilView(depthStencilResource, &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

}

ID3D12Resource* DirectXCommon::CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height)
{
	//生成するResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;										//Textureの幅
	resourceDesc.Height = height;									//Textureの高さ
	resourceDesc.MipLevels = 1;										//mipmapの数
	resourceDesc.DepthOrArraySize = 1;								//奥行 or配列Textureの配列数
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;			//textureのFormat
	resourceDesc.SampleDesc.Count = 1;								//サンプリングカウント。1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;	//textureの次元数
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;	//DepthStencilとして扱う通知


	//利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;//VRAM上に作る

	//深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;//深度を最大値でクリアする（手前のものを表示したいので、最初は一番遠くしておく）
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//FormatはResourceと合わせる


	//Resuorceの生成
	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,							//Heapの設定
		D3D12_HEAP_FLAG_NONE,						//Heapの特殊な設定なしにする
		&resourceDesc,								//Resourceの設定
		D3D12_RESOURCE_STATE_DEPTH_WRITE,			//深度値を書き込む状態にしておく
		&depthClearValue,							//Clear構造体
		IID_PPV_ARGS(&resource));					//作成するResourceポインタへのポインタ

	assert(SUCCEEDED(hr));

	//データを返す
	return resource;
}

void DirectXCommon::MakeFenceEvent()
{

	//初期値0でFence（CPUとGPUの同期をとれるもの）を作る
	fence = nullptr;
	fenceValue = 0;
	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));	//Fenceが生成できなかったので起動できない
	Logger::Log(Logger::GetStream(), "Complete create fence!!\n");//フェンス生成完了のログを出す

	//FenceのSignal(GPUに指定の位置で指定の値を書き込んでもらう命令)を待つためのEvent(メッセージ)を作成する
	fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent != nullptr);	 //作成が上手くできなかったら起動できない

}

void DirectXCommon::InitalizeDXC()
{

	//dxcCompilerの初期化
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));

	//現時点でincludeはしないが、includeに対応するための設定を行っておく
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));
	Logger::Log(Logger::GetStream(), "Complete: DXC compiler initialization.\n"); //DXC初期化完了

}

void DirectXCommon::MakePSO()
{

	//																			//
	//								RootSignature作成							//
	//																			//

	//rootSignature=具体的にshaderがどこでデータを読めばいいのかという情報をまとめたもの
	//RootSignatureの作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//DescriptorRange=shaderがアクセスするリソース(テクスチャ、バッファなど)をまとめて定義する
	//DescriptorRangeを作る
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;//0から始まる
	descriptorRange[0].NumDescriptors = 1;//数は一つだけ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//Offsetは自動計算

	//RootParameter作成。(pixelShaderのMaterialとVertexShaderのTransformの2つ)
	D3D12_ROOT_PARAMETER rootParameters[4] = {};
	//PixelShader
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;	//ConstantBufferViewを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixelShaderで使う
	rootParameters[0].Descriptor.ShaderRegister = 0;					//レジスタ番号0とバインド(PSのgMaterial(b0)と結びつける)
	//VertexShader
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;	//ConstantBufferViewを使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; //PixelShaderで使う
	rootParameters[1].Descriptor.ShaderRegister = 0;					//レジスタ番号0とバインド(VSのgTransformationMatrix(b0)と結びつける)

	//DescriptorTableを作成する
	//DescriptorTableはDescriptorRangeをまとめたもの
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;	//DescriptorTableを使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;				//PixelShaderで使う
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;			//Tableの中身の配列を指定
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);//Tableで利用する数

	//DirectionalLight
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;	//ConstantBufferViewを使う
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixelShaderで使う
	rootParameters[3].Descriptor.ShaderRegister = 1;					//レジスタ番号0とバインド(PSのgDirecinallLight(b1)と結びつける)

	//Samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;			//バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		//0~1の範囲外をリピート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;		//比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;						//あるだけmipmapを使用する
	staticSamplers[0].ShaderRegister = 0;									//レジスタ番号を使う
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	//pixelShaderを使う
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);


	descriptionRootSignature.pParameters = rootParameters;				//ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters);	//配列の長さ

	//シリアライズしてバイナリにする

	hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Logger::Log(Logger::GetStream(), reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	//バイナリを元に生成
	hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	//																			//
	//							InputLayoutの設定								//
	//																			//

	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};

	//座標
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	//texcoord
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	//法線
	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	//																			//
	//							BlendStateの設定									//
	//																			//

	D3D12_BLEND_DESC blendDesc{};
	//全ての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;


	//																			//
	//						RasterizerStateの設定								//
	//																			//

	D3D12_RASTERIZER_DESC rasterizerDesc{};
	//裏面（時計回り）を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	//三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	//Shaderをコンパイル
	vertexShaderBlob = CompileShader(L"Object3d.VS.hlsl",
		L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(vertexShaderBlob != nullptr);

	pixelShaderBlob = CompileShader(L"Object3d.PS.hlsl",
		L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(pixelShaderBlob != nullptr);



	//																			//
	//						DepthStencilStateの設定								//
	//																			//

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	//Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	//書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//比較関数はLessEqual（近ければ描画する）
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;


	///PSOの作成

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature;					// RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;					// InputLayout 
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
		vertexShaderBlob->GetBufferSize() };									// VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
		pixelShaderBlob->GetBufferSize() };										// PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc;							// BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;				// RasterizerState

	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジ (形状)のタイプ。三角形 
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定(気にしなくて良い)
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	//depthStencilの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;


	// 実際に生成

	hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));
	//生成できなかったら起動できない
	Logger::Log(Logger::GetStream(), "Complete create PSO!!\n");//PSO生成完了のログを出す


}

IDxcBlob* DirectXCommon::CompileShader(const std::wstring& filePath, const wchar_t* profile, IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler)
{
	//「これからシェーダーをコンパイルする」とログに出す
	Logger::Log(Logger::ConvertString(std::format(L"Begin CompileShader, path:{},profile:{}\n", filePath, profile)));

	///hlslファイルを読み込む
	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	//読めなかったら停止する
	assert(SUCCEEDED(hr));

	//読み込んだファイルの内容を設定する
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;//UTF8の文字コードであることを通知

	///Compileする
	LPCWSTR arguments[] = {
		filePath.c_str(),		//コンパイル対象のhlslファイル名
		L"-E",L"main",			//エントリーポイントの指定。基本的にmain以外にはしない
		L"-T",profile,			//ShaderProfileの設定
		L"-Zi",L"Qembed_debug"	//デバッグの情報を埋め込む	(L"-Qembed_debug"でエラー)
		L"-Od",					//最適化を外しておく
		L"-Zpr",				//メモリレイアウトは行優先
	};
	//実際にShaderをコンパイルする
	IDxcResult* shaderResult = nullptr;
	hr = dxcCompiler->Compile(
		&shaderSourceBuffer,			// 読み込んだファイル
		arguments,			            // コンパイルオプション
		_countof(arguments),            // コンパイルオプションの数
		includeHandler,	            // includeが含まれた諸々
		IID_PPV_ARGS(&shaderResult)     // コンパイル結果
	);

	//コンパイルエラーではなくdxcが起動できないなど致命的な状況のとき停止
	assert(SUCCEEDED(hr));

	///警告・エラーがでていないか確認する
		//警告・エラーが出ていたらログに出して停止する
	IDxcBlobUtf8* shaderError = nullptr;
	if (shaderError != nullptr && shaderError->GetStringLength() != 0)
	{
		Logger::Log(shaderError->GetStringPointer());
		assert(false);
	}
	///Compile結果を受け取って返す
	//コンパイル結果から実行用のバイナリ部分を取得
	IDxcBlob* shaderBlob = nullptr;//Blob = Binary Large OBject(バイナリーデータの塊)
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	//成功したログを出す
	Logger::Log(Logger::ConvertString(std::format(L"Compile Succesed,path:{},profile:{}\n", filePath, profile)));
	//もう使わないリソースを開放
	shaderSource->Release();
	shaderResult->Release();

	//実行用のバイナリを返却
	return shaderBlob;
}

void DirectXCommon::MakeViewport()
{
	//ビューポート
	//クライアント領域のサイズと一緒にして画面全体に表示
	viewport.Width = kClientWidth;
	viewport.Height = kClientHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	//シザー矩形

	//基本的にビューポートと同じ矩形が構成されるようにする
	scissorRect.left = 0;
	scissorRect.right = kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = kClientHeight;
}

void DirectXCommon::PreDraw()
{
	///*-----------------------------------------------------------------------*///
	//																			//
	///				画面をクリアする処理が含まれたコマンドリストを作る				   ///
	//																			//
	///*-----------------------------------------------------------------------*///
	//これから書き込むバックバッファのインデックスの取得
	UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();


	///TransitionBarrierの設定
	//今回のバリアはTransition
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	// Noneにしておく
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//バリアを張る対象のリソース。現在のバックバッファに対して行う
	barrier.Transition.pResource = swapChainResources[backBufferIndex];
	// 遷移前（現在）のResourceState
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	//遷移後のResourceState
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	//TransitionBarrierを張る
	commandList->ResourceBarrier(1, &barrier);


	//描画先のRTVとDSVを設定する
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, &dsvHandle);

	// 指定した色で画面全体をクリアする
	float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };//青っぽい色。RGBAの順
	commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	//	描画用のDesctiptorHeapの設定
	ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap };
	commandList->SetDescriptorHeaps(1, descriptorHeaps);
	///*-----------------------------------------------------------------------*///
	//																			//
	///							描画に必要コマンドを積む						   ///
	//																			//
	///*-----------------------------------------------------------------------*///

	commandList->RSSetViewports(1, &viewport);					//Viewportを設定	
	commandList->RSSetScissorRects(1, &scissorRect);			//Scissorを設定
	// RootSignatureを設定。PSOに設定しているけど別途設定（PSOと同じもの）が必要
	commandList->SetGraphicsRootSignature(rootSignature);
	commandList->SetPipelineState(graphicsPipelineState);		//PSOを設定
	// 形状を設定。PSOに設定しているものとはまた別。RootSignatureと同じように同じものを設定すると考えておけばいい
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

}

void DirectXCommon::PostDraw()
{

	//	画面に描く処理はすべて終わり、画面に映すので、状態を遷移
	//	今回はRenerTargetからPresentにする
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	//TransitionBarrierを張る
	commandList->ResourceBarrier(1, &barrier);


	//コマンドリストの内容を確定させる。全てのコマンドを積んでからCloseすること
	hr = commandList->Close();
	assert(SUCCEEDED(hr));//ダメなら起動できない

	///コマンドをキックする
	ID3D12CommandList* commandLists[] = { commandList };
	commandQueue->ExecuteCommandLists(1, commandLists);
	//GPUとOSに画面の交換を行うように通知
	swapChain->Present(1, 0);

	///GPUにSignalを送る
	//Fenceの値を更新
	fenceValue++;
	//GPUがここまでたどり着いたときに、Fenceの値を指定した値に代入するようにSignalを送る
	commandQueue->Signal(fence, fenceValue);

	//Fenceの値が指定したSignal値にたどり着いているか確認する
	//GetCompleteValueの初期値はFence作成時に渡した初期値
	if (fence->GetCompletedValue() < fenceValue) {
		//指定したSignalにたどり着いていないので，たどり着くまで待つようにイベントを設定する
		fence->SetEventOnCompletion(fenceValue, fenceEvent);
		//イベント待つ
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	//次のフレーム用のコマンドリストを準備
	hr = commandAllocator->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList->Reset(commandAllocator, nullptr);
	assert(SUCCEEDED(hr));

}

