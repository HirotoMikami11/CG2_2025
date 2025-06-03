#include "DescriptorHeapManager.h"

void DescriptorHeapManager::Initialize(Microsoft::WRL::ComPtr<ID3D12Device> device) {
	device_ = device;

	// ディスクリプタサイズを取得
	rtvDescriptorSize_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	dsvDescriptorSize_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	srvDescriptorSize_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// ディスクリプタヒープを作成
	rtvHeap_ = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, GraphicsConfig::kRTVHeapSize, false);
	dsvHeap_ = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, GraphicsConfig::kDSVHeapSize, false);
	srvHeap_ = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, GraphicsConfig::kSRVHeapSize, true);


	// 使用状況管理配列を初期化(生成したヒープの大きさだけフラグの数を持っておく)
	rtvUsedFlags_.resize(GraphicsConfig::kRTVHeapSize, false);
	dsvUsedFlags_.resize(GraphicsConfig::kDSVHeapSize, false);
	srvUsedFlags_.resize(GraphicsConfig::kSRVHeapSize, false);

	// 予約済みのもののUsedFlagを立てておく
	// スワップチェーンRTVを予約
	rtvUsedFlags_[GraphicsConfig::kSwapChainRTV0Index] = true;
	rtvUsedFlags_[GraphicsConfig::kSwapChainRTV1Index] = true;

	// メインDSVを予約
	dsvUsedFlags_[GraphicsConfig::kMainDSVIndex] = true;

	// ImGui用SRVを予約
	srvUsedFlags_[GraphicsConfig::kImGuiSRVIndex] = true;

	isInitialized_ = true;

	Logger::Log(Logger::GetStream(), "DescriptorHeapManager initialized successfully!\n");
	Logger::Log(Logger::GetStream(), std::format("RTV Heap Size: {}, DSV Heap Size: {}, SRV Heap Size: {}\n",
		GraphicsConfig::kRTVHeapSize, GraphicsConfig::kDSVHeapSize, GraphicsConfig::kSRVHeapSize));
}

void DescriptorHeapManager::Finalize() {
	// 使用状況をクリア
	rtvUsedFlags_.clear();
	dsvUsedFlags_.clear();
	srvUsedFlags_.clear();

	// ヒープをクリア（ComPtrが自動で解放）
	rtvHeap_.Reset();
	dsvHeap_.Reset();
	srvHeap_.Reset();

	device_.Reset();
	isInitialized_ = false;

	Logger::Log(Logger::GetStream(), "DescriptorHeapManager finalized.\n");
}



//																			//
//							割り当てと解放										//
//																			//



DescriptorHeapManager::DescriptorHandle DescriptorHeapManager::AllocateRTV() {
	assert(isInitialized_);
	///あいているインデックスを探す
	auto indexOpt = FindAvailableIndex(rtvUsedFlags_);
	if (!indexOpt.has_value()) {
		// 空きがない場合はログを出力
		Logger::Log(Logger::GetStream(), "Failed to allocate RTV: No available slots\n");
		return DescriptorHandle{};
	}

	uint32_t index = indexOpt.value();
	rtvUsedFlags_[index] = true;	//空きが見つかればそこの使用フラグを立てる

	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = GetCPUHandle(HeapType::RTV, index);
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = {}; // RTVはGPUハンドル不要

	// 生成できたとログを出力
	Logger::Log(Logger::GetStream(), std::format("Allocated RTV at index: {}\n", index));
	return DescriptorHandle(cpuHandle, gpuHandle, index);
}

DescriptorHeapManager::DescriptorHandle DescriptorHeapManager::AllocateDSV() {
	assert(isInitialized_);
	///あいているインデックスを探す
	auto indexOpt = FindAvailableIndex(dsvUsedFlags_);
	if (!indexOpt.has_value()) {
		// 空きがない場合はログを出力
		Logger::Log(Logger::GetStream(), "Failed to allocate DSV: No available slots\n");
		return DescriptorHandle{};
	}

	uint32_t index = indexOpt.value();
	dsvUsedFlags_[index] = true;//空きが見つかればそこの使用フラグを立てる

	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = GetCPUHandle(HeapType::DSV, index);
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = {}; // DSVはGPUハンドル不要

	// 生成できたとログを出力
	Logger::Log(Logger::GetStream(), std::format("Allocated DSV at index: {}\n", index));
	return DescriptorHandle(cpuHandle, gpuHandle, index);
}

DescriptorHeapManager::DescriptorHandle DescriptorHeapManager::AllocateSRV() {
	assert(isInitialized_);
	///あいているインデックスを探す
	auto indexOpt = FindAvailableIndex(srvUsedFlags_);
	if (!indexOpt.has_value()) {
		// 空きがない場合はログを出力
		Logger::Log(Logger::GetStream(), "Failed to allocate SRV: No available slots\n");
		return DescriptorHandle{};
	}

	uint32_t index = indexOpt.value();
	srvUsedFlags_[index] = true;//空きが見つかればそこの使用フラグを立てる

	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = GetCPUHandle(HeapType::SRV, index);
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = GetGPUHandle(HeapType::SRV, index);
	// 生成できたとログを出力
	Logger::Log(Logger::GetStream(), std::format("Allocated SRV at index: {}\n", index));
	return DescriptorHandle(cpuHandle, gpuHandle, index);
}

void DescriptorHeapManager::ReleaseRTV(uint32_t index) {
	assert(isInitialized_);

	if (!IsValidIndex(HeapType::RTV, index)) {
		// 無効なインデックスの場合はログを出力
		Logger::Log(Logger::GetStream(), std::format("Invalid RTV index for release: {}\n", index));
		return;
	}

	if (!rtvUsedFlags_[index]) {
		// 既に解放されている場合は警告を出力
		Logger::Log(Logger::GetStream(), std::format("Warning: RTV index {} is already released\n", index));
		return;
	}

	// 実際の解放処理
	rtvUsedFlags_[index] = false;
	Logger::Log(Logger::GetStream(), std::format("Released RTV at index: {}\n", index));
}

void DescriptorHeapManager::ReleaseDSV(uint32_t index) {
	assert(isInitialized_);

	if (!IsValidIndex(HeapType::DSV, index)) {
		// 無効なインデックスの場合はログを出力
		Logger::Log(Logger::GetStream(), std::format("Invalid DSV index for release: {}\n", index));
		return;
	}

	if (!dsvUsedFlags_[index]) {
		// 既に解放されている場合は警告を出力
		Logger::Log(Logger::GetStream(), std::format("Warning: DSV index {} is already released\n", index));
		return;
	}

	// 実際の解放処理
	dsvUsedFlags_[index] = false;
	Logger::Log(Logger::GetStream(), std::format("Released DSV at index: {}\n", index));
}

void DescriptorHeapManager::ReleaseSRV(uint32_t index) {
	assert(isInitialized_);

	// 無効なインデックスの場合はログを出力
	if (!IsValidIndex(HeapType::SRV, index)) {
		Logger::Log(Logger::GetStream(), std::format("Invalid SRV index for release: {}\n", index));
		return;
	}
	
	// 既に解放されている場合は警告を出力
	if (!srvUsedFlags_[index]) {
		Logger::Log(Logger::GetStream(), std::format("Warning: SRV index {} is already released\n", index));
		return;
	}

	// 実際の解放処理
	srvUsedFlags_[index] = false;
	Logger::Log(Logger::GetStream(), std::format("Released SRV at index: {}\n", index));
}


//																			//
//						指定したインデックスを予約								//
//																			//


std::optional<DescriptorHeapManager::DescriptorHandle> DescriptorHeapManager::ReserveRTV(uint32_t index) {
	assert(isInitialized_);

	if (!IsValidIndex(HeapType::RTV, index)) {
		// 無効なインデックスの場合はログを出力
		Logger::Log(Logger::GetStream(), std::format("Invalid RTV index for reservation: {}\n", index));
		return std::nullopt;
	}

	if (rtvUsedFlags_[index]) {
		// 既に使用中の場合はログを出力
		Logger::Log(Logger::GetStream(), std::format("RTV index {} is already in use\n", index));
		return std::nullopt;
	}

	// 使用フラグを立てる
	rtvUsedFlags_[index] = true;

	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = GetCPUHandle(HeapType::RTV, index);
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = {};

	// 予約できたとログを出力
	Logger::Log(Logger::GetStream(), std::format("Reserved RTV at index: {}\n", index));
	return DescriptorHandle(cpuHandle, gpuHandle, index);
}

std::optional<DescriptorHeapManager::DescriptorHandle> DescriptorHeapManager::ReserveDSV(uint32_t index) {
	assert(isInitialized_);

	if (!IsValidIndex(HeapType::DSV, index)) {
		// 無効なインデックスの場合はログを出力
		Logger::Log(Logger::GetStream(), std::format("Invalid DSV index for reservation: {}\n", index));
		return std::nullopt;
	}

	if (dsvUsedFlags_[index]) {
		// 既に使用中の場合はログを出力
		Logger::Log(Logger::GetStream(), std::format("DSV index {} is already in use\n", index));
		return std::nullopt;
	}

	// 使用フラグを立てる
	dsvUsedFlags_[index] = true;

	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = GetCPUHandle(HeapType::DSV, index);
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = {};

	// 予約できたとログを出力
	Logger::Log(Logger::GetStream(), std::format("Reserved DSV at index: {}\n", index));
	return DescriptorHandle(cpuHandle, gpuHandle, index);
}

std::optional<DescriptorHeapManager::DescriptorHandle> DescriptorHeapManager::ReserveSRV(uint32_t index) {
	assert(isInitialized_);

	if (!IsValidIndex(HeapType::SRV, index)) {
		Logger::Log(Logger::GetStream(), std::format("Invalid SRV index for reservation: {}\n", index));
		return std::nullopt;
	}

	if (srvUsedFlags_[index]) {
		Logger::Log(Logger::GetStream(), std::format("SRV index {} is already in use\n", index));
		return std::nullopt;
	}

	srvUsedFlags_[index] = true;

	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = GetCPUHandle(HeapType::SRV, index);
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = GetGPUHandle(HeapType::SRV, index);

	Logger::Log(Logger::GetStream(), std::format("Reserved SRV at index: {}\n", index));
	return DescriptorHandle(cpuHandle, gpuHandle, index);
}

//=============================================================================
// ユーティリティ関数
//=============================================================================

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeapManager::GetCPUHandle(HeapType type, uint32_t index) const {
	assert(isInitialized_);

	D3D12_CPU_DESCRIPTOR_HANDLE handle = {};
	uint32_t descriptorSize = 0;

	// ヒープの種類に応じてハンドルとサイズを設定
	switch (type) {
	case HeapType::RTV:
		assert(IsValidIndex(type, index));
		handle = rtvHeap_->GetCPUDescriptorHandleForHeapStart();
		descriptorSize = rtvDescriptorSize_;
		break;
	case HeapType::DSV:
		assert(IsValidIndex(type, index));
		handle = dsvHeap_->GetCPUDescriptorHandleForHeapStart();
		descriptorSize = dsvDescriptorSize_;
		break;
	case HeapType::SRV:
		assert(IsValidIndex(type, index));
		handle = srvHeap_->GetCPUDescriptorHandleForHeapStart();
		descriptorSize = srvDescriptorSize_;
		break;
	default:
		assert(false);
		break;
	}

	//先頭からインデックス分だけポインタを進める
	handle.ptr += (descriptorSize * index);
	return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeapManager::GetGPUHandle(HeapType type, uint32_t index) const {
	assert(isInitialized_);
	assert(type == HeapType::SRV); // GPUハンドルはSRVのみなので違ったらアサ―ト

	assert(IsValidIndex(type, index));
	
	D3D12_GPU_DESCRIPTOR_HANDLE handle = srvHeap_->GetGPUDescriptorHandleForHeapStart();
	//先頭からインデックス分だけポインタを進める
	handle.ptr += (srvDescriptorSize_ * index);
	return handle;
}

uint32_t DescriptorHeapManager::GetAvailableCount(HeapType type) const {
	assert(isInitialized_);

	const std::vector<bool>* usedFlags = nullptr;
	switch (type) {
	case HeapType::RTV: usedFlags = &rtvUsedFlags_; break;
	case HeapType::DSV: usedFlags = &dsvUsedFlags_; break;
	case HeapType::SRV: usedFlags = &srvUsedFlags_; break;
	default: assert(false); return 0;
	}

	uint32_t availableCount = 0;
	for (bool used : *usedFlags) {
		if (!used) {
			availableCount++;
		}
	}
	return availableCount;
}

uint32_t DescriptorHeapManager::GetUsedCount(HeapType type) const {
	assert(isInitialized_);

	const std::vector<bool>* usedFlags = nullptr;
	uint32_t totalCount = 0;

	switch (type) {
	case HeapType::RTV:
		usedFlags = &rtvUsedFlags_;
		totalCount = GraphicsConfig::kRTVHeapSize;
		break;
	case HeapType::DSV:
		usedFlags = &dsvUsedFlags_;
		totalCount = GraphicsConfig::kDSVHeapSize;
		break;
	case HeapType::SRV:
		usedFlags = &srvUsedFlags_;
		totalCount = GraphicsConfig::kSRVHeapSize;
		break;
	default:
		assert(false);
		return 0;
	}

	return totalCount - GetAvailableCount(type);
}

bool DescriptorHeapManager::IsIndexUsed(HeapType type, uint32_t index) const {
	assert(isInitialized_);

	if (!IsValidIndex(type, index)) {
		return false;
	}

	switch (type) {
	case HeapType::RTV: return rtvUsedFlags_[index];
	case HeapType::DSV: return dsvUsedFlags_[index];
	case HeapType::SRV: return srvUsedFlags_[index];
	default: assert(false); return false;
	}
}

//=============================================================================
// 内部関数
//=============================================================================

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DescriptorHeapManager::CreateDescriptorHeap(
	D3D12_DESCRIPTOR_HEAP_TYPE type,
	uint32_t numDescriptors,
	bool shaderVisible) {

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = type;
	desc.NumDescriptors = numDescriptors;
	desc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	HRESULT hr = device_->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap));
	assert(SUCCEEDED(hr));

	const char* typeName = "";
	switch (type) {
	case D3D12_DESCRIPTOR_HEAP_TYPE_RTV: typeName = "RTV"; break;
	case D3D12_DESCRIPTOR_HEAP_TYPE_DSV: typeName = "DSV"; break;
	case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV: typeName = "SRV"; break;
	default: typeName = "Unknown"; break;
	}

	Logger::Log(Logger::GetStream(), std::format("Created {} DescriptorHeap: {} descriptors\n", typeName, numDescriptors));
	return descriptorHeap;
}

std::optional<uint32_t> DescriptorHeapManager::FindAvailableIndex(const std::vector<bool>& usedFlags) const {
	
	// 空いているインデックスを先頭から走査する
	for (size_t i = 0; i < usedFlags.size(); ++i) {
		if (!usedFlags[i]) {
			return static_cast<uint32_t>(i);// 空いているインデックスを返す
		}
	}
	return std::nullopt;
}

bool DescriptorHeapManager::IsValidIndex(HeapType type, uint32_t index) const {
	switch (type) {
	case HeapType::RTV: return index < GraphicsConfig::kRTVHeapSize;
	case HeapType::DSV: return index < GraphicsConfig::kDSVHeapSize;
	case HeapType::SRV: return index < GraphicsConfig::kSRVHeapSize;
	default: return false;
	}
}