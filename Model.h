#pragma once
#include "DirectXCommon.h"
#include "MyFunction.h"
#include "Logger.h"
#include <cassert>


#include "Mesh.h"
#include "Material.h"

//マテリアルの情報をtextureManagerの送るため
#include "TextureManager.h"




class Model
{
public:
	Model() = default;
	~Model() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	void Initialize(DirectXCommon* dxCommon, const MeshType meshType,
		const std::string& directoryPath = "", const std::string& filename = "");

	//非const
	Material& GetMaterial() { return material_; }
	Mesh& GetMesh() { return mesh_; }
	//const
	const Material& GetMaterial() const { return material_; }
	const Mesh& GetMesh() const { return mesh_; }

private:


	// DirectXCommon参照
	DirectXCommon* directXCommon_ = nullptr;

	///material（bool型でlight用のセットか、defaultかどうかをinitilizeで決める。通常はdefault）
	//実際のマテリアル
	Material material_;

	///mesh(meeshTypeを引数に入れたらinitilizeでcreateTriangleできるようにする（他の図形でも）)
	//実際のメッシュ
	Mesh mesh_;

	///メッシュの中にあるloadObjをここに移動させる

	//ロードしたデータを保持しておく、material meshに渡すためのもの
	ModelData modelData_;

	MaterialDataModel LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);
	ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);

};

