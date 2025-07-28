#define NOMINMAX
#include "Model.h"
#include "Objects/GameObject/MaterialGroup.h"
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
		//複数オブジェクト対応でデータを読み込む
		modelDataList_ = LoadObjFileMulti(directoryPath, filename);

		// 各ModelDataからMeshを作成
		meshes_.clear();
		meshMaterialIndices_.clear();

		for (const auto& modelData : modelDataList_) {
			Mesh mesh;
			mesh.InitializeFromData(directXCommon_, modelData);
			meshes_.push_back(std::move(mesh));
			meshMaterialIndices_.push_back(modelData.materialIndex);
		}

		// 全マテリアル情報を収集してマテリアルとテクスチャを作成
		std::set<std::string> uniqueMaterials;
		std::map<std::string, MaterialDataModel> materialMap;

		for (const auto& modelData : modelDataList_) {
			if (!modelData.materialName.empty()) {
				uniqueMaterials.insert(modelData.materialName);
				materialMap[modelData.materialName] = modelData.material;
			}
		}

		// MaterialGroupを初期化
		size_t materialCount = std::max(uniqueMaterials.size(), size_t(1));
		materialGroup_.Initialize(directXCommon_, materialCount);

		textureTagNames_.clear();
		textureTagNames_.resize(materialCount);

		TextureManager* textureManager = TextureManager::GetInstance();

		// マテリアルを設定
		size_t materialIndex = 0;
		for (const auto& materialName : uniqueMaterials) {
			Material& material = materialGroup_.GetMaterial(materialIndex);
			material.SetLitObjectSettings(); // デフォルトでライティング有効

			// テクスチャを読み込み
			const auto& materialData = materialMap[materialName];
			if (!materialData.textureFilePath.empty()) {
				std::string textureTag = GetTextureFileNameFromPath(materialData.textureFilePath);
				if (textureManager->LoadTexture(materialData.textureFilePath, textureTag)) {
					textureTagNames_[materialIndex] = textureTag;
					Logger::Log(Logger::GetStream(), std::format("Loaded texture: {} as {}\n", materialData.textureFilePath, textureTag));
				} else {
					Logger::Log(Logger::GetStream(), std::format("Failed to load texture: {}\n", materialData.textureFilePath));
				}
			}
			materialIndex++;
		}

		filePath_ = directoryPath + "/" + filename;
	} else {
		///モデル以外の場合は、パス入れないで生成
		Mesh mesh;
		mesh.Initialize(directXCommon_, meshType);
		meshes_.clear();
		meshes_.push_back(std::move(mesh));

		objectNames_.clear();
		objectNames_.push_back("primitive_" + Mesh::MeshTypeToString(meshType));

		// プリミティブ用のシングルマテリアル
		materialGroup_.Initialize(directXCommon_, 1);

		textureTagNames_.clear();
		textureTagNames_.push_back("");

		meshMaterialIndices_.clear();
		meshMaterialIndices_.push_back(0); // 最初のマテリアルを使用

		filePath_ = "primitive_" + Mesh::MeshTypeToString(meshType);
	}
}

bool Model::LoadFromOBJ(const std::string& directoryPath, const std::string& filename, DirectXCommon* dxCommon) {
	// 既に読み込み済みの場合はスキップ
	if (IsValid() && filePath_ == directoryPath + "/" + filename) {
		return true;
	}

	directXCommon_ = dxCommon;
	filePath_ = directoryPath + "/" + filename;

	// 複数オブジェクト対応でOBJファイルを読み込み
	modelDataList_ = LoadObjFileMulti(directoryPath, filename);

	if (modelDataList_.empty()) {
		Logger::Log(Logger::GetStream(), std::format("Failed to load model data from: {}\n", filename));
		return false;
	}

	// 各ModelDataからMeshを作成
	meshes_.clear();
	meshMaterialIndices_.clear();

	for (size_t i = 0; i < modelDataList_.size(); ++i) {
		Mesh mesh;
		mesh.InitializeFromData(dxCommon, modelDataList_[i]);
		meshes_.push_back(std::move(mesh));
		meshMaterialIndices_.push_back(modelDataList_[i].materialIndex);
	}

	// 全マテリアル情報を収集してマテリアルとテクスチャを作成
	std::set<std::string> uniqueMaterials;
	std::map<std::string, MaterialDataModel> materialMap;

	for (const auto& modelData : modelDataList_) {
		if (!modelData.materialName.empty()) {
			uniqueMaterials.insert(modelData.materialName);
			materialMap[modelData.materialName] = modelData.material;
		}
	}

	// MaterialGroupを初期化
	size_t materialCount = std::max(uniqueMaterials.size(), size_t(1));
	materialGroup_.Initialize(dxCommon, materialCount);

	textureTagNames_.clear();
	textureTagNames_.resize(materialCount);

	TextureManager* textureManager = TextureManager::GetInstance();

	// マテリアルを設定
	size_t materialIndex = 0;
	for (const auto& materialName : uniqueMaterials) {
		Material& material = materialGroup_.GetMaterial(materialIndex);
		material.SetLitObjectSettings(); // デフォルトでライティング有効

		// テクスチャを読み込み
		const auto& materialData = materialMap[materialName];
		if (!materialData.textureFilePath.empty()) {
			std::string textureTag = GetTextureFileNameFromPath(materialData.textureFilePath);
			if (textureManager->LoadTexture(materialData.textureFilePath, textureTag)) {
				textureTagNames_[materialIndex] = textureTag;
				Logger::Log(Logger::GetStream(), std::format("Loaded texture: {} as {}\n", materialData.textureFilePath, textureTag));
			} else {
				Logger::Log(Logger::GetStream(), std::format("Failed to load texture: {}\n", materialData.textureFilePath));
			}
		}
		materialIndex++;
	}

	Logger::Log(Logger::GetStream(), std::format("Model loaded from OBJ: {} ({} meshes, {} materials)\n",
		filename, meshes_.size(), materialGroup_.GetMaterialCount()));
	return true;
}

bool Model::LoadFromPrimitive(MeshType meshType, DirectXCommon* dxCommon) {
	directXCommon_ = dxCommon;

	// プリミティブメッシュを作成
	Mesh mesh;
	mesh.Initialize(dxCommon, meshType);

	meshes_.clear();
	meshes_.push_back(std::move(mesh));

	objectNames_.clear();
	objectNames_.push_back("primitive_" + Mesh::MeshTypeToString(meshType));

	// プリミティブ用のシングルマテリアル
	materialGroup_.Initialize(dxCommon, 1);

	textureTagNames_.clear();
	textureTagNames_.push_back("");

	meshMaterialIndices_.clear();
	meshMaterialIndices_.push_back(0); // 最初のマテリアルを使用

	// プリミティブにはテクスチャは無い
	filePath_ = "primitive_" + Mesh::MeshTypeToString(meshType);

	Logger::Log(Logger::GetStream(), std::format("Model loaded from primitive: {}\n", Mesh::MeshTypeToString(meshType)));
	return true;
}

void Model::UpdateMaterials() {
	materialGroup_.UpdateAllUVTransforms();
}

void Model::Unload() {
	meshes_.clear(); // 全メッシュをクリア
	objectNames_.clear();
	materialGroup_ = MaterialGroup(); // MaterialGroupをリセット
	textureTagNames_.clear();
	meshMaterialIndices_.clear();
	filePath_.clear();
	modelDataList_.clear(); // モデルデータリストをクリア
}

std::string Model::GetFileNameWithoutExtension(const std::string& filename) {
	// 最後のドット（拡張子の開始位置）を見つける
	size_t lastDotPos = filename.find_last_of('.');

	if (lastDotPos != std::string::npos && lastDotPos > 0) {
		// 拡張子がある場合は、それを除いた部分を返す
		return filename.substr(0, lastDotPos);
	}

	// 拡張子がない場合はそのまま返す
	return filename;
}

std::string Model::GetTextureFileNameFromPath(const std::string& texturePath) {
	// パス区切り文字を探す
	size_t lastSlashPos = texturePath.find_last_of("/\\");

	// ファイル名部分を抽出
	std::string fileName;
	if (lastSlashPos != std::string::npos) {
		fileName = texturePath.substr(lastSlashPos + 1);
	} else {
		fileName = texturePath; // パス区切りがない場合はそのまま
	}

	// 拡張子を除去
	return GetFileNameWithoutExtension(fileName);
}

std::map<std::string, MaterialDataModel> Model::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename)
{
	//1.中で必要となる変数の宣言
	std::map<std::string, MaterialDataModel> materials;	//構築するMaterialDataのマップ
	std::string line;					//ファイルから読んだ1行を格納するもの
	std::string currentMaterialName;	//現在処理中のマテリアル名
	MaterialDataModel currentMaterial;	//現在処理中のマテリアルデータ

	//2.ファイルを開く
	std::ifstream file(directoryPath + "/" + filename);//ファイルを開く
	assert(file.is_open());//開けなかった場合は停止する

	//3.実際にファイルを読み、MaterialDataを構築していく
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;	//先頭の識別子を読む

		//identifierに応じた処理
		if (identifier == "newmtl") {
			// 前のマテリアルがあれば保存
			if (!currentMaterialName.empty()) {
				materials[currentMaterialName] = currentMaterial;
			}

			// 新しいマテリアルを開始
			s >> currentMaterialName;
			currentMaterial = MaterialDataModel(); // リセット

		} else if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			//連結してファイルパスにする
			currentMaterial.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}

	// 最後のマテリアルを保存
	if (!currentMaterialName.empty()) {
		materials[currentMaterialName] = currentMaterial;
	}

	Logger::Log(Logger::GetStream(), std::format("Loaded {} materials from {}\n", materials.size(), filename));
	for (const auto& pair : materials) {
		Logger::Log(Logger::GetStream(), std::format("  Material: {} -> texture: {}\n",
			pair.first, pair.second.textureFilePath.empty() ? "none" : pair.second.textureFilePath));
	}

	//4.MaterialDataのマップを返す
	return materials;
}

std::vector<ModelData> Model::LoadObjFileMulti(const std::string& directoryPath, const std::string& filename) {
	//1.中で必要となる変数の宣言
	std::vector<ModelData> modelDataList;	//構築するModelDataのリスト
	std::vector<Vector4> positions;			//位置（全オブジェクト共通）
	std::vector<Vector3> normals;			//法線（全オブジェクト共通）
	std::vector<Vector2> texcoords;			//テクスチャ座標（全オブジェクト共通）

	std::string line;						//ファイルから読んだ1行を格納するもの

	// マルチマテリアル対応
	std::map<std::string, MaterialDataModel> materials;	// マテリアル名 -> マテリアルデータ
	std::map<std::string, size_t> materialIndexMap;		// マテリアル名 -> インデックス
	std::string currentMaterialName = "";				// 現在使用中のマテリアル名
	size_t materialIndexCounter = 0;					// マテリアルインデックスカウンター

	ModelData currentModel;					//現在処理中のモデル
	std::string currentObjectName = "default"; // 現在のオブジェクト名
	bool hasCurrentObject = false;			//現在処理中のオブジェクトがあるか
	bool hasExplicitObjects = false;		//明示的にオブジェクト名が指定されているか

	//2.ファイルを開く
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	//3.実際にファイルを読み、ModelDataを構築していく
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "v") {		//識別子がvの場合	頂点位置
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			positions.push_back(position);

		} else if (identifier == "vt") {//識別子がvtの場合	頂点テクスチャ
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoords.push_back(texcoord);

		} else if (identifier == "vn") {//識別子がvnの場合	頂点法線
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);

		} else if (identifier == "o") {	//識別子がoの場合	オブジェクト名
			hasExplicitObjects = true; // 明示的なオブジェクト定義があることを記録

			// 前のオブジェクトがあれば保存
			if (hasCurrentObject && !currentModel.vertices.empty()) {
				// 現在のマテリアル情報を設定
				if (!currentMaterialName.empty() && materials.find(currentMaterialName) != materials.end()) {
					currentModel.material = materials[currentMaterialName];
					currentModel.materialName = currentMaterialName;
					currentModel.materialIndex = materialIndexMap[currentMaterialName];
				}
				modelDataList.push_back(currentModel);
				objectNames_.push_back(currentObjectName);
			}

			// 新しいオブジェクトを開始
			s >> currentObjectName;
			currentModel = ModelData(); // リセット
			hasCurrentObject = true;

		} else if (identifier == "usemtl") { // マテリアル切り替え
			s >> currentMaterialName;

			// 新しいマテリアルの場合はインデックスを割り当て
			if (materialIndexMap.find(currentMaterialName) == materialIndexMap.end()) {
				materialIndexMap[currentMaterialName] = materialIndexCounter++;
				Logger::Log(Logger::GetStream(), std::format("Material assigned: {} -> index {}\n",
					currentMaterialName, materialIndexMap[currentMaterialName]));
			}

		} else if (identifier == "f") {	//識別子がfの場合	面
			// オブジェクト名が指定されていない場合はデフォルトオブジェクトを作成
			if (!hasCurrentObject) {
				hasCurrentObject = true;
				// 明示的なオブジェクト定義がない場合は "default" を使用
				currentObjectName = hasExplicitObjects ? "unnamed" : "default";
			}

			//面は三角形限定
			VertexData triangle[3];

			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;

				std::stringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/');
					if (!index.empty()) {
						elementIndices[element] = std::stoi(index);
					} else {
						elementIndices[element] = 1; // デフォルト値
					}
				}

				//要素へのindexから、実際の要素の値を取得して頂点を構成する
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = (elementIndices[1] <= texcoords.size()) ? texcoords[elementIndices[1] - 1] : Vector2{ 0.0f, 0.0f };
				Vector3 normal = (elementIndices[2] <= normals.size()) ? normals[elementIndices[2] - 1] : Vector3{ 0.0f, 0.0f, -1.0f };

				//右手座標系から左手座標系に変換する
				position.x *= -1.0f;
				normal.x *= -1.0f;
				texcoord.y = 1.0f - texcoord.y;
				triangle[faceVertex] = { position, texcoord, normal };
			}
			currentModel.vertices.push_back(triangle[2]);
			currentModel.vertices.push_back(triangle[1]);
			currentModel.vertices.push_back(triangle[0]);

		} else if (identifier == "mtllib") {
			//materialTemplateLibraryファイルの名前を取得する
			std::string materialFilename;
			s >> materialFilename;
			materials = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}

	// 最後のオブジェクトを保存
	if (hasCurrentObject && !currentModel.vertices.empty()) {
		// 現在のマテリアル情報を設定
		if (!currentMaterialName.empty() && materials.find(currentMaterialName) != materials.end()) {
			currentModel.material = materials[currentMaterialName];
			currentModel.materialName = currentMaterialName;
			currentModel.materialIndex = materialIndexMap[currentMaterialName];
		}
		modelDataList.push_back(currentModel);
		objectNames_.push_back(currentObjectName);
	}

	Logger::Log(Logger::GetStream(), std::format("Loaded {} objects from {} with {} materials\n",
		modelDataList.size(), filename, materials.size()));
	for (size_t i = 0; i < objectNames_.size(); ++i) {
		Logger::Log(Logger::GetStream(), std::format("  Object {}: {} ({} vertices, material: {})\n",
			i, objectNames_[i], modelDataList[i].vertices.size(),
			modelDataList[i].materialName.empty() ? "none" : modelDataList[i].materialName));
	}

	return modelDataList;
}