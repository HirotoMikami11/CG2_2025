#include "TextureManager.h"

// シングルトンインスタンス
TextureManager* TextureManager::GetInstance() {
	static TextureManager instance;
	return &instance;
}

TextureManager::~TextureManager() {
	Finalize();
}

void TextureManager::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;

	// 初期化できたらログを出す
	Logger::Log(Logger::GetStream(), "Complete TextureManager initialized !!\n");
}

void TextureManager::Finalize() {
	// 全てのテクスチャを解放
	UnloadAll();
	dxCommon_ = nullptr;
}

bool TextureManager::LoadTexture(const std::string& filename, const std::string& tagName) {
	// 既に同じタグ名で登録されていた場合は古いものを解放
	if (HasTexture(tagName)) {
		UnloadTexture(tagName);
	}

	// DescriptorHeapManagerからSRVを割り当て
	auto descriptorManager = dxCommon_->GetDescriptorManager();
	if (!descriptorManager) {
		Logger::Log(Logger::GetStream(), "DescriptorManager is null\n");
		return false;
	}

	// 使用可能なSRVスロットを割り当て
	auto descriptorHandle = descriptorManager->AllocateSRV();
	if (!descriptorHandle.isValid) {
		Logger::Log(Logger::GetStream(), std::format("Failed to allocate SRV for texture '{}': No available slots.\n", filename));
		return false;
	}

	// 新しいテクスチャを作成し、既に割り当てられたハンドルを使用
	auto texture = std::make_unique<Texture>();
	if (!texture->LoadTextureWithHandle(filename, dxCommon_, descriptorHandle)) {
		// ロードに失敗した場合はSRVを解放
		descriptorManager->ReleaseSRV(descriptorHandle.index);
		return false;
	}

	// マップに登録
	textures_[tagName] = std::move(texture);

	Logger::Log(Logger::GetStream(), std::format("Texture '{}' loaded successfully with tag '{}' (SRV Index: {})\n",
		filename, tagName, descriptorHandle.index));
	return true;
}

Texture* TextureManager::GetTexture(const std::string& tagName) {
	auto it = textures_.find(tagName);
	if (it != textures_.end()) {
		return it->second.get();
	}

	// テクスチャが見つからない場合はnullptr
	Logger::Log(Logger::GetStream(),
		std::format("Texture with tag '{}' not found.\n", tagName));
	return nullptr;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetTextureHandle(const std::string& tagName) {
	Texture* texture = GetTexture(tagName);
	if (texture) {
		return texture->GetGPUHandle();
	}

	// デフォルトハンドル（無効な値）を返す
	D3D12_GPU_DESCRIPTOR_HANDLE invalidHandle = {};
	return invalidHandle;
}

void TextureManager::UnloadTexture(const std::string& tagName) {
	auto textureIt = textures_.find(tagName);
	if (textureIt != textures_.end()) {
		// テクスチャをアンロード（内部でSRVも解放される）
		textureIt->second->Unload(dxCommon_);

		// マップから削除
		textures_.erase(textureIt);

		Logger::Log(Logger::GetStream(), std::format("Texture with tag '{}' unloaded.\n", tagName));
	}
}

void TextureManager::UnloadAll() {
	// 全てのテクスチャを解放
	for (const auto& pair : textures_) {
		Logger::Log(Logger::GetStream(),
			std::format("Unloading texture: {}\n", pair.first));
		pair.second->Unload(dxCommon_);
	}

	textures_.clear();

	Logger::Log(Logger::GetStream(), "All textures unloaded.\n");
}

bool TextureManager::HasTexture(const std::string& tagName) const {
	// 指定されたタグ名のテクスチャが存在するかチェック
	return textures_.find(tagName) != textures_.end();
}

uint32_t TextureManager::GetAvailableSRVCount() const {
	auto descriptorManager = dxCommon_->GetDescriptorManager();
	if (descriptorManager) {
		return descriptorManager->GetAvailableCount(DescriptorHeapManager::HeapType::SRV);
	}
	return 0;
}

uint32_t TextureManager::GetUsedSRVCount() const {
	auto descriptorManager = dxCommon_->GetDescriptorManager();
	if (descriptorManager) {
		return descriptorManager->GetUsedCount(DescriptorHeapManager::HeapType::SRV);
	}
	return 0;
}