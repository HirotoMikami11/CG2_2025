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

	// インデックス管理用配列を初期化
	usedIndices_.resize(MAX_TEXTURE_COUNT, false);
	//初期化できたらログを出す
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

	// 使用可能なインデックスを取得
	uint32_t srvIndex = GetAvailableSRVIndex();

	//インデックスの数が最大値以上の場合は獲得できないとログを出す
	if (srvIndex >= MAX_TEXTURE_COUNT) {
		Logger::Log(Logger::GetStream(),std::format("Failed to load texture '{}': No available SRV slots.\n", filename));
		return false;
	}

	// 新しいテクスチャを作成
	auto texture = std::make_unique<Texture>();
	if (!texture->LoadTexture(filename, dxCommon_, srvIndex)) {
		ReleaseSRVIndex(srvIndex);
		return false;
	}

	// マップに登録
	textures_[tagName] = std::move(texture);
	textureIndices_[tagName] = srvIndex;

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
	auto indexIt = textureIndices_.find(tagName);

	if (textureIt != textures_.end() && indexIt != textureIndices_.end()) {
		// インデックスを解放
		ReleaseSRVIndex(indexIt->second);

		// マップから削除
		textures_.erase(textureIt);
		textureIndices_.erase(indexIt);

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
}

bool TextureManager::HasTexture(const std::string& tagName) const {
	// 指定されたタグ名のテクスチャが存在するかチェック
	// なぜか画像が読み込めなかった場合に使用したので残しておく
	return textures_.find(tagName) != textures_.end();
}

uint32_t TextureManager::GetAvailableSRVIndex() {
	// 空いているインデックスを探す（0から順番に）
	// falseのインデックスを探して、見つかったらtrueにしてそのインデックスを返す
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