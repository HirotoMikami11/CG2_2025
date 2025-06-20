#pragma once

#include <map>
#include <string>
#include <memory>
#include <vector>
#include <cassert>
#include <algorithm>

#include "DirectXCommon.h"
#include "BaseSystem/Logger/Logger.h"
#include "Texture.h"

class DirectXCommon;

/// <summary>
/// テクスチャを管理する管理クラス
/// </summary>
class TextureManager {
public:
	//シングルトン
	static TextureManager* GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// テクスチャの読み込み
	/// </summary>
	/// <param name="filename">テクスチャファイルのパス</param>
	/// <param name="tagName">識別用のタグ名</param>
	/// <returns>読み込み成功かどうか</returns>
	bool LoadTexture(const std::string& filename, const std::string& tagName);

	/// <summary>
	/// テクスチャの取得
	/// </summary>
	/// <param name="tagName">識別用のタグ名</param>
	/// <returns>テクスチャのポインタ（存在しない場合はnullptr）</returns>
	Texture* GetTexture(const std::string& tagName);

	/// <summary>
	/// テクスチャのGPUハンドルを取得（描画で直接使用）
	/// </summary>
	/// <param name="tagName">識別用のタグ名</param>
	/// <returns>GPUハンドル</returns>
	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureHandle(const std::string& tagName);

	/// <summary>
	/// テクスチャの解放
	/// </summary>
	/// <param name="tagName">識別用のタグ名</param>
	void UnloadTexture(const std::string& tagName);

	/// <summary>
	/// 全てのテクスチャを解放
	/// </summary>
	void UnloadAll();

	/// <summary>
	/// テクスチャが存在するかチェック
	/// </summary>
	/// <param name="tagName">識別用のタグ名</param>
	/// <returns>存在するかどうか</returns>
	bool HasTexture(const std::string& tagName) const;

	/// <summary>
	/// 読み込まれているテクスチャの数を取得
	/// </summary>
	/// <returns>テクスチャ数</returns>
	size_t GetTextureCount() const { return textures_.size(); }

	/// <summary>
	/// 利用可能なSRVスロット数を取得
	/// </summary>
	/// <returns>利用可能なスロット数</returns>
	uint32_t GetAvailableSRVCount() const;

	/// <summary>
	/// 使用中のSRVスロット数を取得
	/// </summary>
	/// <returns>使用中のスロット数</returns>
	uint32_t GetUsedSRVCount() const;

private:
	// コンストラクタ
	TextureManager() = default;
	// デストラクタ
	~TextureManager();

	// コピーを禁止
	TextureManager(const TextureManager&) = delete;
	TextureManager& operator=(const TextureManager&) = delete;

	// DirectXCommonへのポインタ
	DirectXCommon* dxCommon_ = nullptr;

	// テクスチャの管理用マップ（tagNameからTextureを見つける）
	std::map<std::string, std::unique_ptr<Texture>> textures_;
};