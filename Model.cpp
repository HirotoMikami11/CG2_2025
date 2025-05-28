#include "Model.h"

void Model::Initialize(DirectXCommon* dxCommon, const std::string& meshType, const std::string& directoryPath, const std::string& filename)
{

	directXCommon_ = dxCommon;

	//																			//
	//								メッシュの作成									//
	//																			//


	///モデルの場合は、ファイルパスなどを入れる
	if (meshType == "Model") {
		mesh_.Initialize(directXCommon_, meshType, directoryPath, filename);
		//モデルデータからメッシュを作成(中身でメッシュも作成される)
		//modelMesh.LoadFromOBJ("resources", "plane.obj");
		// Meshから読み込んだマテリアル情報を使ってテクスチャを読み込み
		
		//if (modelMesh.HasMaterialInfo()) {//読み込んだデータがあるか？
		//	textureManager->LoadTexture(modelMesh.GetTextureFilePath(), "planeTexture");
		//}
	} else {
		///モデル以外の場合は、パス入れないで生成
		mesh_.Initialize(directXCommon_, meshType);
	}

	//																			//
	//							Material用のResourceを作る						//
	//																			//


	// マテリアル用のリソースを作る
	material_.Initialize(directXCommon_);
	//ライト付きオブジェクト用設定
	//material_.SetLitObjectSettings();

}
