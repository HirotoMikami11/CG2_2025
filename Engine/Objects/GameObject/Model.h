#pragma once
#include <cassert>
#include <vector>
#include <map>
#include <set>

#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "MyMath/MyFunction.h"
#include "BaseSystem/Logger/Logger.h"
#include "Objects/GameObject/Mesh.h"
#include "Objects/GameObject/Material.h"
#include "Objects/GameObject/MaterialGroup.h"

//マテリアルの情報をtextureManagerの送るため
#include "Managers/Texture/TextureManager.h"

/// <summary>
/// モデルクラス（メッシュ、マテリアル、テクスチャを管理）
/// </summary>
class Model
{
public:
	Model() = default;
	~Model() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="meshType">メッシュタイプ</param>
	/// <param name="directoryPath">ディレクトリパス</param>
	/// <param name="filename">ファイル名</param>
	void Initialize(DirectXCommon* dxCommon, const MeshType meshType,
		const std::string& directoryPath = "", const std::string& filename = "");

	/// <summary>
	/// OBJファイルからモデルを読み込み
	/// </summary>
	/// <param name="directoryPath">ディレクトリパス</param>
	/// <param name="filename">ファイル名</param>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <returns>読み込み成功かどうか</returns>
	bool LoadFromOBJ(const std::string& directoryPath, const std::string& filename, DirectXCommon* dxCommon);

	/// <summary>
	/// プリミティブメッシュから読み込み
	/// </summary>
	/// <param name="meshType">メッシュタイプ</param>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <returns>読み込み成功かどうか</returns>
	bool LoadFromPrimitive(MeshType meshType, DirectXCommon* dxCommon);

	/// <summary>
	/// モデルをアンロード
	/// </summary>
	void Unload();

	/// <summary>
	/// 全マテリアルのUVTransformを更新
	/// </summary>
	void UpdateMaterials();

	// メッシュ関連のアクセサ
	size_t GetMeshCount() const { return meshes_.size(); }

	Mesh& GetMesh(size_t index = 0) {
		if (index >= meshes_.size()) {
			Logger::Log(Logger::GetStream(), std::format("Mesh index {} out of range (max: {})\n", index, meshes_.size() - 1));
			return meshes_[0]; // フォールバック
		}
		return meshes_[index];
	}

	const Mesh& GetMesh(size_t index = 0) const {
		if (index >= meshes_.size()) {
			Logger::Log(Logger::GetStream(), std::format("Mesh index {} out of range (max: {})\n", index, meshes_.size() - 1));
			return meshes_[0]; // フォールバック
		}
		return meshes_[index];
	}

	// 全メッシュへのアクセス
	std::vector<Mesh>& GetMeshes() { return meshes_; }
	const std::vector<Mesh>& GetMeshes() const { return meshes_; }

	// マテリアル関連
	MaterialGroup& GetMaterialGroup() { return materialGroup_; }
	const MaterialGroup& GetMaterialGroup() const { return materialGroup_; }

	size_t GetMaterialCount() const { return materialGroup_.GetMaterialCount(); }

	Material& GetMaterial(size_t index = 0) { return materialGroup_.GetMaterial(index); }
	const Material& GetMaterial(size_t index = 0) const { return materialGroup_.GetMaterial(index); }

	// マテリアル一括操作
	void SetAllMaterialsColor(const Vector4& color, LightingMode lightingMode = LightingMode::HalfLambert) {
		materialGroup_.SetAllMaterials(color, lightingMode);
	}

	// テクスチャタグ名の設定と取得（マルチテクスチャ対応）
	void SetTextureTagName(const std::string& tagName, size_t index = 0) {
		if (index < textureTagNames_.size()) {
			textureTagNames_[index] = tagName;
		}
	}

	const std::string& GetTextureTagName(size_t index = 0) const {
		if (index < textureTagNames_.size()) {
			return textureTagNames_[index];
		}
		static std::string empty;
		return empty;
	}

	bool HasTexture(size_t index = 0) const {
		return index < textureTagNames_.size() && !textureTagNames_[index].empty();
	}

	// 全テクスチャタグ名へのアクセス
	const std::vector<std::string>& GetTextureTagNames() const { return textureTagNames_; }

	/// <summary>
	/// メッシュのマテリアルインデックスを取得
	/// </summary>
	/// <param name="meshIndex">メッシュインデックス</param>
	/// <returns>マテリアルインデックス</returns>
	size_t GetMeshMaterialIndex(size_t meshIndex) const {
		if (meshIndex < meshMaterialIndices_.size()) {
			return meshMaterialIndices_[meshIndex];
		}
		return 0; // デフォルトは最初のマテリアル
	}

	/// <summary>
	/// モデルが有効かどうか
	/// </summary>
	/// <returns>有効かどうか</returns>
	bool IsValid() const { return !meshes_.empty() && meshes_[0].GetVertexCount() > 0; }

	/// <summary>
	/// ファイルパスを取得
	/// </summary>
	/// <returns>ファイルパス</returns>
	const std::string& GetFilePath() const { return filePath_; }

	/// <summary>
	/// オブジェクト名のリストを取得
	/// </summary>
	/// <returns>オブジェクト名のリスト</returns>
	const std::vector<std::string>& GetObjectNames() const { return objectNames_; }

private:
	// DirectXCommon参照
	DirectXCommon* directXCommon_ = nullptr;

	// マテリアルグループ（モデルのマテリアルを全て管理）
	MaterialGroup materialGroup_;

	// 複数メッシュ対応
	std::vector<Mesh> meshes_;
	std::vector<std::string> objectNames_; // 各メッシュのオブジェクト名
	std::vector<size_t> meshMaterialIndices_; // 各メッシュが使用するマテリアルのインデックス

	// ロードしたデータを保持しておく
	std::vector<ModelData> modelDataList_;

	// マルチテクスチャ対応
	std::vector<std::string> textureTagNames_;

	// ファイルパス（デバッグ用）
	std::string filePath_;

	/// <summary>
	/// OBJファイルを読み込む
	/// </summary>
	std::vector<ModelData> LoadObjFileMulti(const std::string& directoryPath, const std::string& filename);

	/// <summary>
	/// マテリアルファイルを読み込む
	/// </summary>
	std::map<std::string, MaterialDataModel> LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

	/// <summary>
	/// ファイル名から拡張子を除去
	/// </summary>
	/// <param name="filename">ファイル名</param>
	/// <returns>拡張子を除いたファイル名</returns>
	std::string GetFileNameWithoutExtension(const std::string& filename);

	/// <summary>
	/// テクスチャファイルパスから画像ファイル名（拡張子なし）を抽出
	/// </summary>
	/// <param name="texturePath">テクスチャファイルのフルパス</param>
	/// <returns>画像ファイル名（拡張子なし）</returns>
	std::string GetTextureFileNameFromPath(const std::string& texturePath);
};