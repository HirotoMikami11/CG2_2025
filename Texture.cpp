#include "Texture.h"

bool Texture::LoadTexture(const std::string& filePath, DirectXCommon* dxCommon, uint32_t srvIndex) {
	// 既に読み込み済みの場合はスキップ
	if (IsValid() && filePath_ == filePath) {
		return true;
	}

	filePath_ = filePath;

	// テクスチャファイルを読み込み
	DirectX::ScratchImage mipImages = LoadTextureFile(filePath);
	if (mipImages.GetImageCount() == 0) {
		return false;
	}

	// メタデータを保存
	metadata_ = mipImages.GetMetadata();

	// テクスチャリソースを作成
	textureResource_ = CreateTextureResource(dxCommon->GetDeviceComPtr(), metadata_);
	if (!textureResource_) {
		return false;
	}

	// テクスチャデータをアップロード
	intermediateResource_ = UploadTextureData(
		textureResource_,
		mipImages,
		dxCommon->GetDeviceComPtr(),
		dxCommon->GetCommandListComPtr());

	// DescriptorHeapManagerからSRVを割り当て
	auto descriptorManager = dxCommon->GetDescriptorManager();
	if (!descriptorManager) {
		Logger::Log(Logger::GetStream(), "DescriptorManager is null\n");
		return false;
	}

	// 指定されたインデックスでSRVを予約
	auto handleOpt = descriptorManager->ReserveSRV(srvIndex);
	if (!handleOpt.has_value()) {
		Logger::Log(Logger::GetStream(), std::format("Failed to reserve SRV at index: {}\n", srvIndex));
		return false;
	}

	// ハンドルを保存
	descriptorHandle_ = handleOpt.value();
	cpuHandle_ = descriptorHandle_.cpuHandle;
	gpuHandle_ = descriptorHandle_.gpuHandle;
	srvIndex_ = srvIndex;

	// SRVを作成
	CreateSRV(dxCommon->GetDeviceComPtr(), cpuHandle_);

	// ロード完了のログ
	Logger::Log(Logger::GetStream(), std::format("Complete Texture loaded : {} (SRV Index: {})\n", filePath, srvIndex));
	return true;
}

bool Texture::LoadTextureWithHandle(const std::string& filePath, DirectXCommon* dxCommon,
	const DescriptorHeapManager::DescriptorHandle& descriptorHandle) {
	// 既に読み込み済みの場合はスキップ
	if (IsValid() && filePath_ == filePath) {
		return true;
	}

	filePath_ = filePath;

	// テクスチャファイルを読み込み
	DirectX::ScratchImage mipImages = LoadTextureFile(filePath);
	if (mipImages.GetImageCount() == 0) {
		return false;
	}

	// メタデータを保存
	metadata_ = mipImages.GetMetadata();

	// テクスチャリソースを作成
	textureResource_ = CreateTextureResource(dxCommon->GetDeviceComPtr(), metadata_);
	if (!textureResource_) {
		return false;
	}

	// テクスチャデータをアップロード
	intermediateResource_ = UploadTextureData(
		textureResource_,
		mipImages,
		dxCommon->GetDeviceComPtr(),
		dxCommon->GetCommandListComPtr());

	// ハンドルを保存（既に割り当て済み）
	descriptorHandle_ = descriptorHandle;
	cpuHandle_ = descriptorHandle_.cpuHandle;
	gpuHandle_ = descriptorHandle_.gpuHandle;
	srvIndex_ = descriptorHandle_.index;

	// SRVを作成
	CreateSRV(dxCommon->GetDeviceComPtr(), cpuHandle_);

	// ロード完了のログ
	Logger::Log(Logger::GetStream(), std::format("Complete Texture loaded with handle : {} (SRV Index: {})\n", filePath, srvIndex_));
	return true;
}

void Texture::Unload(DirectXCommon* dxCommon) {
	if (!IsValid()) {
		return;
	}

	// SRVを解放
	if (srvIndex_ != INVALID_INDEX) {
		auto descriptorManager = dxCommon->GetDescriptorManager();
		if (descriptorManager) {
			descriptorManager->ReleaseSRV(srvIndex_);
		}
		srvIndex_ = INVALID_INDEX;
	}

	// リソースをクリア
	textureResource_.Reset();
	intermediateResource_.Reset();

	// ハンドルをクリア
	cpuHandle_ = {};
	gpuHandle_ = {};
	descriptorHandle_ = {};

	// その他の情報をクリア
	metadata_ = {};
	filePath_.clear();

	Logger::Log(Logger::GetStream(), std::format("Texture unloaded: {}\n", filePath_));
}

DirectX::ScratchImage Texture::LoadTextureFile(const std::string& filePath) {
	// テクスチャファイルを読んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = Logger::ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);

	if (FAILED(hr)) {
		// ロード失敗のログ
		Logger::Log(Logger::GetStream(), std::format("Failed to load texture: {}\n", filePath));
		return DirectX::ScratchImage{};
	}

	// ミップマップ生成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(
		image.GetImages(),
		image.GetImageCount(),
		image.GetMetadata(),
		DirectX::TEX_FILTER_SRGB,
		0,
		mipImages);

	if (FAILED(hr)) {
		// ミップマップ生成失敗のログ
		Logger::Log(Logger::GetStream(), std::format("Failed to generate mipmaps for: {}\n", filePath));
		return image; // ミップマップ生成に失敗しても元の画像を返す
	}

	return mipImages;
}

Microsoft::WRL::ComPtr<ID3D12Resource> Texture::CreateTextureResource(
	const Microsoft::WRL::ComPtr<ID3D12Device>& device,
	const DirectX::TexMetadata& metadata) {

	// metadataを基にResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metadata.width);								// textureの幅
	resourceDesc.Height = UINT(metadata.height);							// textureの高さ
	resourceDesc.MipLevels = UINT16(metadata.mipLevels);					// mipmapの数
	resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize);				// 奥行 or配列Textureの配列数
	resourceDesc.Format = metadata.format;									// textureのFormat
	resourceDesc.SampleDesc.Count = 1;										// サンプリングカウント。1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);	// textureの次元数

	// 利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	// Resourceの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,						// heapの設定
		D3D12_HEAP_FLAG_NONE,					// heapの特殊な設定。特になし
		&resourceDesc,							// resourceの設定
		D3D12_RESOURCE_STATE_COPY_DEST,			// 初回のresourceState。textureは基本読むだけ
		nullptr,								// clear最適値。使わないのでnullptr
		IID_PPV_ARGS(&resource));				// 作成するResourceポインタへのポインタ

	if (FAILED(hr)) {
		// Resourceの生成失敗のログ
		Logger::Log(Logger::GetStream(), "Failed to create texture resource\n");
		return nullptr;
	}

	return resource;
}

Microsoft::WRL::ComPtr<ID3D12Resource> Texture::UploadTextureData(
	Microsoft::WRL::ComPtr<ID3D12Resource> texture,
	const DirectX::ScratchImage& mipImages,
	const Microsoft::WRL::ComPtr<ID3D12Device>& device,
	const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList) {

	std::vector<D3D12_SUBRESOURCE_DATA> subresource;
	DirectX::PrepareUpload(device.Get(), mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresource);

	uint64_t intermediateSize = GetRequiredIntermediateSize(texture.Get(), 0, UINT(subresource.size()));
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = CreateBufferResource(device.Get(), intermediateSize);

	UpdateSubresources(commandList.Get(), texture.Get(), intermediateResource.Get(), 0, 0, UINT(subresource.size()), subresource.data());

	// textureへの転送後は利用できるよう、D3D12_RESOURCE_STATE_COPY_DESTからD3D12_RESOURCE_STATE_GENERIC_READへResourceStateを変更する
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	commandList->ResourceBarrier(1, &barrier);

	return intermediateResource;
}

void Texture::CreateSRV(const Microsoft::WRL::ComPtr<ID3D12Device>& device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle) {
	// SRV設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata_.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(metadata_.mipLevels);

	// SRVの生成
	device->CreateShaderResourceView(textureResource_.Get(), &srvDesc, cpuHandle);
}