#pragma once


#include <map>
#include <string>
#include <memory>
#include <vector>

#include <cassert>
#include <algorithm>

#include "DirectXCommon.h"
#include "Logger.h"
#include "Texture.h"

class DirectXCommon;

/// <summary>
/// テクスチャを管理する管理クラス（AudioManagerと同様の設計）
/// </summary>
class TextureManager {
public:
	// ゲームアプリケーション全体で一つのインスタンスを使う（シングルトン）
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
	/// 空いているSRVインデックスを取得
	/// </summary>
	/// <returns>使用可能なインデックス</returns>
	uint32_t GetAvailableSRVIndex();

	/// <summary>
	/// SRVインデックスを解放
	/// </summary>
	/// <param name="index">解放するインデックス</param>
	void ReleaseSRVIndex(uint32_t index);

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

	// テクスチャの管理用マップ（tagName -> Texture）
	std::map<std::string, std::unique_ptr<Texture>> textures_;

	// インデックス管理用（tagName -> SRVインデックス）
	std::map<std::string, uint32_t> textureIndices_;

	// 使用中のSRVインデックスを管理
	std::vector<bool> usedIndices_;

	// 最大テクスチャ数（SRVヒープのサイズ-1）
	static constexpr uint32_t MAX_TEXTURE_COUNT = 127; // ImGui用に1つ予約するため128-1
};