#pragma once
#include <string>
#include <map>
#include <memory>
#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "Objects/GameObject/Model.h"
#include "Managers/Texture/TextureManager.h"
#include "BaseSystem/Logger/Logger.h"

/// <summary>
/// モデルリソースを管理する
/// </summary>
class ModelManager {
public:
	static ModelManager* GetInstance();

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
	/// OBJモデルの読み込み
	/// </summary>
	/// <param name="directoryPath">ディレクトリパス</param>
	/// <param name="filename">ファイル名</param>
	/// <param name="tagName">識別用のタグ名</param>
	/// <returns>読み込み成功かどうか</returns>
	bool LoadModel(const std::string& directoryPath, const std::string& filename, const std::string& tagName);

	/// <summary>
	/// プリミティブモデルの読み込み
	/// </summary>
	/// <param name="meshType">メッシュタイプ</param>
	/// <param name="tagName">識別用のタグ名</param>
	/// <returns>読み込み成功かどうか</returns>
	bool LoadPrimitive(MeshType meshType, const std::string& tagName);

	/// <summary>
	/// モデルの取得
	/// </summary>
	/// <param name="tagName">識別用のタグ名</param>
	/// <returns>モデルのポインタ（存在しない場合はnullptr）</returns>
	Model* GetModel(const std::string& tagName);

	/// <summary>
	/// モデルの解放
	/// </summary>
	/// <param name="tagName">識別用のタグ名</param>
	void UnloadModel(const std::string& tagName);

	/// <summary>
	/// 全てのモデルを解放
	/// </summary>
	void UnloadAll();

	/// <summary>
	/// モデルが存在するかチェック
	/// </summary>
	/// <param name="tagName">識別用のタグ名</param>
	/// <returns>存在するかどうか</returns>
	bool HasModel(const std::string& tagName) const;

	/// <summary>
	/// 読み込まれているモデルの数を取得
	/// </summary>
	/// <returns>モデル数</returns>
	size_t GetModelCount() const { return models_.size(); }

private:
	ModelManager() = default;
	~ModelManager();
	ModelManager(const ModelManager&) = delete;
	ModelManager& operator=(const ModelManager&) = delete;

	DirectXCommon* dxCommon_ = nullptr;
	TextureManager* textureManager_ = nullptr;

	// モデルの管理用マップ（tagNameからModelを見つける）
	std::map<std::string, std::unique_ptr<Model>> models_;


};