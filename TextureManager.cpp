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
	assert(dxCommon != nullptr);
	dxCommon_ = dxCommon;

	// インデックス管理用配列を初期化
	usedIndices_.resize(MAX_TEXTURE_COUNT, false);

	Logger::Log(Logger::GetStream(), "TextureManager initialized successfully.\n");
}

void TextureManager::Finalize() {
	UnloadAll();
	dxCommon_ = nullptr;

	Logger::Log(Logger::GetStream(), "TextureManager finalized.\n");
}

bool TextureManager::LoadTexture(const std::string& filename, const std::string& tagName) {
	assert(dxCommon_ != nullptr && "TextureManager not initialized!");

	// 既に同じタグ名で登録されていた場合は古いものを解放
	if (HasTexture(tagName)) {
		Logger::Log(Logger::GetStream(),
			std::format("Texture with tag '{}' already exists. Replacing it.\n", tagName));
		UnloadTexture(tagName);
	}

	// 使用可能なインデックスを取得
	uint32_t srvIndex = GetAvailableSRVIndex();
	if (srvIndex >= MAX_TEXTURE_COUNT) {
		Logger::Log(Logger::GetStream(),
			std::format("Failed to load texture '{}': No available SRV slots.\n", filename));
		return false;
	}

	// 新しいテクスチャを作成
	auto texture = std::make_unique<Texture>();
	if (!texture->LoadTexture(filename, dxCommon_, srvIndex)) {
		ReleaseSRVIndex(srvIndex);
		Logger::Log(Logger::GetStream(),
			std::format("Failed to load texture: {}\n", filename));
		return false;
	}

	// マップに登録
	textures_[tagName] = std::move(texture);
	textureIndices_[tagName] = srvIndex;

	Logger::Log(Logger::GetStream(),
		std::format("Texture loaded successfully: {} (tag: {}, index: {})\n",
			filename, tagName, srvIndex));

	return true;
}

Texture* TextureManager::GetTexture(const std::string& tagName) {
	auto it = textures_.find(tagName);
	if (it != textures_.end()) {
		return it->second.get();
	}

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
	auto indexIt = textureIndices_.find(tagName);

	if (textureIt != textures_.end() && indexIt != textureIndices_.end()) {
		// インデックスを解放
		ReleaseSRVIndex(indexIt->second);

		// マップから削除
		textures_.erase(textureIt);
		textureIndices_.erase(indexIt);

		Logger::Log(Logger::GetStream(),
			std::format("Texture unloaded: {}\n", tagName));
	}
}

void TextureManager::UnloadAll() {
	// 全てのテクスチャを解放
	for (const auto& pair : textures_) {
		Logger::Log(Logger::GetStream(),
			std::format("Unloading texture: {}\n", pair.first));
	}

	textures_.clear();
	textureIndices_.clear();
	std::fill(usedIndices_.begin(), usedIndices_.end(), false);

	Logger::Log(Logger::GetStream(), "All textures unloaded.\n");
}

bool TextureManager::HasTexture(const std::string& tagName) const {
	return textures_.find(tagName) != textures_.end();
}

void TextureManager::DebugPrintTextureInfo() const {
	Logger::Log(Logger::GetStream(),
		std::format("=== Texture Manager Debug Info ===\n"));
	Logger::Log(Logger::GetStream(),
		std::format("Total textures loaded: {}\n", textures_.size()));

	for (const auto& pair : textures_) {
		const std::string& tagName = pair.first;
		const Texture* texture = pair.second.get();
		auto indexIt = textureIndices_.find(tagName);
		uint32_t index = (indexIt != textureIndices_.end()) ? indexIt->second : 0;

		Logger::Log(Logger::GetStream(),
			std::format("  Tag: {}, Index: {}, FilePath: {}\n",
				tagName, index, texture->GetFilePath()));
	}

	// 使用中のインデックスを表示
	Logger::Log(Logger::GetStream(), "Used SRV indices: ");
	for (uint32_t i = 0; i < usedIndices_.size(); ++i) {
		if (usedIndices_[i]) {
			Logger::Log(Logger::GetStream(), std::format("{} ", i));
		}
	}
	Logger::Log(Logger::GetStream(), "\n");
	Logger::Log(Logger::GetStream(), "=== End Debug Info ===\n");
}

uint32_t TextureManager::GetAvailableSRVIndex() {
	// 空いているインデックスを探す（0から順番に）
	for (uint32_t i = 0; i < MAX_TEXTURE_COUNT; ++i) {
		if (!usedIndices_[i]) {
			usedIndices_[i] = true;
			return i;
		}
	}

	// 空きがない場合は無効な値を返す
	return MAX_TEXTURE_COUNT;
}

void TextureManager::ReleaseSRVIndex(uint32_t index) {
	if (index < MAX_TEXTURE_COUNT) {
		usedIndices_[index] = false;
	}
}