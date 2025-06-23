#pragma once
#include <cassert>

#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "MyMath/MyFunction.h"
#include "BaseSystem/Logger/Logger.h"
#include "Objects/GameObject/Mesh.h"
#include "Objects/GameObject/Material.h"

//マテリアルの情報をtextureManagerの送るため
#include "Managers/Texture/TextureManager.h"

/// <summary>
/// 共有可能なモデルクラス - TextureManagerと同様の設計
/// </summary>
class Model
{
public:
	Model() = default;
	~Model() = default;

	/// <summary>
	/// 初期化（従来通り）
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="meshType">メッシュタイプ</param>
	/// <param name="directoryPath">ディレクトリパス</param>
	/// <param name="filename">ファイル名</param>
	void Initialize(DirectXCommon* dxCommon, const MeshType meshType,
		const std::string& directoryPath = "", const std::string& filename = "");

	/// <summary>
	/// OBJファイルからモデルを読み込み（共有用）
	/// </summary>
	/// <param name="directoryPath">ディレクトリパス</param>
	/// <param name="filename">ファイル名</param>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <returns>読み込み成功かどうか</returns>
	bool LoadFromOBJ(const std::string& directoryPath, const std::string& filename, DirectXCommon* dxCommon);

	/// <summary>
	/// プリミティブメッシュから読み込み（共有用）
	/// </summary>
	/// <param name="meshType">メッシュタイプ</param>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <returns>読み込み成功かどうか</returns>
	bool LoadFromPrimitive(MeshType meshType, DirectXCommon* dxCommon);

	/// <summary>
	/// モデルをアンロード
	/// </summary>
	void Unload();

	//非const
	Material& GetMaterial() { return material_; }
	Mesh& GetMesh() { return mesh_; }
	//const
	const Material& GetMaterial() const { return material_; }
	const Mesh& GetMesh() const { return mesh_; }

	// テクスチャタグ名の設定と取得
	void SetTextureTagName(const std::string& tagName) { textureTagName_ = tagName; }
	const std::string& GetTextureTagName() const { return textureTagName_; }
	bool HasTexture() const { return !textureTagName_.empty(); }

	/// <summary>
	/// モデルが有効かどうか
	/// </summary>
	/// <returns>有効かどうか</returns>
	bool IsValid() const { return mesh_.GetVertexCount() > 0; }

	/// <summary>
	/// ファイルパスを取得
	/// </summary>
	/// <returns>ファイルパス</returns>
	const std::string& GetFilePath() const { return filePath_; }

private:
	// DirectXCommon参照
	DirectXCommon* directXCommon_ = nullptr;

	///material（bool型でlight用のセットか、defaultかどうかをinitilizeで決める。通常はdefault）
	//実際のマテリアル
	Material material_;

	///mesh(meeshTypeを引数に入れたらinitilizeでcreateTriangleできるようにする（他の図形でも）)
	//実際のメッシュ
	Mesh mesh_;

	//ロードしたデータを保持しておく、material meshに渡すためのもの
	ModelData modelData_;
	// テクスチャのタグ名を保存
	std::string textureTagName_;
	// ファイルパス（デバッグ用）
	std::string filePath_;

	/// <summary>
	/// OBJファイルを読み込む
	/// </summary>
	ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);

	/// <summary>
	/// マテリアルファイルを読み込む
	/// </summary>
	MaterialDataModel LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);
};