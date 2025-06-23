#include "Model.h"
#include <fstream>
#include <sstream>

void Model::Initialize(DirectXCommon* dxCommon, const MeshType meshType, const std::string& directoryPath, const std::string& filename)
{
	directXCommon_ = dxCommon;

	//																			//
	//								メッシュの作成									//
	//																			//

	///モデルの場合は、ファイルパスなどを入れる
	if (meshType == MeshType::MODEL_OBJ) {
		//データを読み込む
		modelData_ = LoadObjFile(directoryPath, filename);
		mesh_.InitializeFromData(directXCommon_, modelData_);
		// OBJファイル用の固有タグ名を生成
		textureTagName_ = filename + "_obj_texture";
		filePath_ = directoryPath + "/" + filename;
		TextureManager::GetInstance()->LoadTexture(modelData_.material.textureFilePath, textureTagName_);
	} else {
		///モデル以外の場合は、パス入れないで生成
		mesh_.Initialize(directXCommon_, meshType);
		filePath_ = "primitive_" + Mesh::MeshTypeToString(meshType);
	}

	//																			//
	//							Material用のResourceを作る						//
	//																			//

	// マテリアル用のリソースを作る
	material_.Initialize(directXCommon_);
	//ライト付きオブジェクト用設定
	//material_.SetLitObjectSettings();
}

bool Model::LoadFromOBJ(const std::string& directoryPath, const std::string& filename, DirectXCommon* dxCommon) {
	// 既に読み込み済みの場合はスキップ
	if (IsValid() && filePath_ == directoryPath + "/" + filename) {
		return true;
	}

	directXCommon_ = dxCommon;
	filePath_ = directoryPath + "/" + filename;

	// OBJファイルを読み込み
	modelData_ = LoadObjFile(directoryPath, filename);

	// メッシュを作成
	mesh_.InitializeFromData(dxCommon, modelData_);

	// マテリアルを初期化
	material_.Initialize(dxCommon);

	// テクスチャを読み込み（OBJファイル用の固有タグ名を生成）
	textureTagName_ = filename + "_obj_texture";

	if (!modelData_.material.textureFilePath.empty()) {
		TextureManager* textureManager = TextureManager::GetInstance();
		if (!textureManager->LoadTexture(modelData_.material.textureFilePath, textureTagName_)) {
			Logger::Log(Logger::GetStream(), std::format("Failed to load texture for model: {}\n", filename));
			textureTagName_.clear();
		}
	}

	Logger::Log(Logger::GetStream(), std::format("Model loaded from OBJ: {}\n", filename));
	return true;
}

bool Model::LoadFromPrimitive(MeshType meshType, DirectXCommon* dxCommon) {
	directXCommon_ = dxCommon;

	// プリミティブメッシュを作成
	mesh_.Initialize(dxCommon, meshType);

	// マテリアルを初期化
	material_.Initialize(dxCommon);

	// プリミティブにはテクスチャは無い
	textureTagName_ = "";
	filePath_ = "primitive_" + Mesh::MeshTypeToString(meshType);

	Logger::Log(Logger::GetStream(), std::format("Model loaded from primitive: {}\n", Mesh::MeshTypeToString(meshType)));
	return true;
}

void Model::Unload() {
	mesh_ = Mesh(); // メッシュをリセット
	material_ = Material(); // マテリアルをリセット
	textureTagName_.clear();
	filePath_.clear();
	modelData_ = ModelData(); // モデルデータをリセット
}

MaterialDataModel Model::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename)
{
	//1.中で必要となる変数の宣言
	MaterialDataModel materialData;			//構築するMaterialData
	std::string line;					//ファイルから読んだ1行を格納するもの

	//2.ファイルを開く
	std::ifstream file(directoryPath + "/" + filename);//ファイルを開く
	assert(file.is_open());//開けなかった場合は停止する

	//3.実際にファイルを読み、ModelDataを構築していく
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;	//先頭の識別子を読む

		//identifierに応じた処理
		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			//連結してファイルパスにする
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}
	//4.MaterialDataを返す
	return materialData;
}

ModelData Model::LoadObjFile(const std::string& directoryPath, const std::string& filename) {
	//1.中で必要となる変数の宣言
	ModelData modelData;				//構築するModelData
	std::vector<Vector4> positions;		//位置
	std::vector<Vector3> normals;		//法線
	std::vector<Vector2> texcoords;		//テクスチャ座標
	std::string line;					//ファイルから読んだ1行を格納するもの

	//2.ファイルを開く
	std::ifstream file(directoryPath + "/" + filename);//ファイルを開く
	assert(file.is_open());//開けなかった場合は停止する

	//3.実際にファイルを読み、ModelDataを構築していく
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;	//先頭の識別子を読む

		if (identifier == "v") {		//識別子がvの場合	頂点位置
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;	//w成分は1.0fで初期化
			positions.push_back(position);//位置を格納する

		} else if (identifier == "vt") {//識別子がvtの場合	頂点テクスチャ
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoords.push_back(texcoord);//テクスチャ座標を格納する

		} else if (identifier == "vn") {//識別子がvnの場合	頂点法線
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);//法線を格納する

		} else if (identifier == "f") {	//識別子がfの場合	面
			//面は三角形限定
			VertexData triangle[3];//三角形の頂点データを格納する

			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;	//頂点の情報を読む
				//頂点の要素へのindexは「位置/UV/法線」で格納されているので、分解してindexを取得する
				std::stringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/');// '/'区切りでインデックスを読んでいく
					elementIndices[element] = std::stoi(index);
				}

				//要素へのindexから、実際の要素の値を取得して頂点を構成する
				Vector4 position = positions[elementIndices[0] - 1];	//位置は1始まりなので-1する
				Vector2 texcoord = texcoords[elementIndices[1] - 1];	//テクスチャ座標も-1
				Vector3 normal = normals[elementIndices[2] - 1];		//法線も-1

				//右手座標系から左手座標系に変換する
				position.x *= -1.0f;
				normal.x *= -1.0f;
				texcoord.y = 1.0f - texcoord.y; // Y軸反転
				triangle[faceVertex] = { position, texcoord, normal };//頂点データを構成する
			}
			modelData.vertices.push_back(triangle[2]);//頂点データを格納する
			modelData.vertices.push_back(triangle[1]);//頂点データを格納する
			modelData.vertices.push_back(triangle[0]);//頂点データを格納する
		} else if (identifier == "mtllib") {
			//materialTemplateLibraryファイルの名前を取得する
			std::string materialFilename;
			s >> materialFilename;
			//objファイルとmtlファイルは同じディレクトリにあるので、ディレクトリ・ファイル名を渡す
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}
	//4.ModelDataを返す
	return modelData;
}