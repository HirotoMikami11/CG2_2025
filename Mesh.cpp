#include "Mesh.h"
void Mesh::Initialize(DirectXCommon* dxCommon, const std::string& meshType,
	const std::string& directoryPath, const std::string& filename) {
	directXCommon_ = dxCommon;

	// メッシュタイプに応じて対応するcreate関数を呼び出す
	if (meshType == "Triangle") {
		CreateTriangle();
	} else if (meshType == "Sphere") {
		CreateSphere(16); // デフォルトで16分割
	} else if (meshType == "Sprite") {
		CreateSprite({ 160.0f, 90.0f }, { 320.0f, 180.0f }); // デフォルトサイズ
	} else if (meshType == "Model") {
		// モデルの場合はOBJファイルを読み込み
		if (!directoryPath.empty() && !filename.empty()) {
			ModelData modelData = LoadObjFile(directoryPath, filename);
			CreateModel(modelData);
		} else {
			// パスが指定されていない場合はエラーまたはデフォルト処理
			assert(false && "Model type requires directoryPath and filename");
		}
	} else {
		// 不明なタイプの場合はデフォルトで三角形を作成
		CreateTriangle();
	}
}

void Mesh::Initialize(DirectXCommon* dxCommon, const std::string& meshType, ModelData modelData)
{

	directXCommon_ = dxCommon;

	// メッシュタイプに応じて対応するcreate関数を呼び出す
	if (meshType == "Triangle") {
		CreateTriangle();
	} else if (meshType == "Sphere") {
		CreateSphere(16); // デフォルトで16分割
	} else if (meshType == "Sprite") {
		CreateSprite({ 0.0f, 0.0f }, { 1.0f, 1.0f }); // デフォルトサイズ
	} else if (meshType == "Model") {
		CreateModel(modelData);
	} else {
		// パスが指定されていない場合はエラーまたはデフォルト処理
		assert(false && "Model type requires directoryPath and filename");
	}


}

void Mesh::CreateTriangle()
{

	// 三角形の頂点データを作成
	vertices_.resize(3);

	// 上部中央
	vertices_[0].position = { 0.0f, 0.5f, 0.0f, 1.0f };
	vertices_[0].texcoord = { 0.5f, 0.0f };
	vertices_[0].normal = { 0.0f, 0.0f, -1.0f };

	// 左下
	vertices_[1].position = { -0.5f, -0.5f, 0.0f, 1.0f };
	vertices_[1].texcoord = { 0.0f, 1.0f };
	vertices_[1].normal = { 0.0f, 0.0f, -1.0f };

	// 右下
	vertices_[2].position = { 0.5f, -0.5f, 0.0f, 1.0f };
	vertices_[2].texcoord = { 1.0f, 1.0f };
	vertices_[2].normal = { 0.0f, 0.0f, -1.0f };

	// インデックスデータ（反時計回り）
	indices_ = { 0, 1, 2 };

	// 法線を位置ベクトルから計算
	for (size_t i = 0; i < vertices_.size(); ++i) {
		vertices_[i].normal.x = vertices_[i].position.x;
		vertices_[i].normal.y = vertices_[i].position.y;
		vertices_[i].normal.z = vertices_[i].position.z;
	}

	// バッファを作成
	CreateVertexBuffer();
	CreateIndexBuffer();
}

void Mesh::CreateSphere(uint32_t subdivision)
{

	const float kLonEvery = (2 * std::numbers::pi_v<float>) / subdivision; // 経度分割1つ分の角度
	const float kLatEvery = std::numbers::pi_v<float> / subdivision; // 緯度分割1つ分の角度

	// 頂点データの作成
	// (subdivision+1) × (subdivision+1) の格子状の頂点を作成
	for (uint32_t latIndex = 0; latIndex <= subdivision; ++latIndex) {
		float lat = (-std::numbers::pi_v<float> / 2.0f) + kLatEvery * latIndex; // 現在の緯度(θ)
		for (uint32_t lonIndex = 0; lonIndex <= subdivision; ++lonIndex) {
			float lon = lonIndex * kLonEvery; // 現在の経度(φ)

			VertexData vertex{};
			// 頂点位置の計算
			vertex.position.x = cos(lat) * cos(lon);
			vertex.position.y = sin(lat);
			vertex.position.z = cos(lat) * sin(lon);
			vertex.position.w = 1.0f;

			// テクスチャ座標の計算
			vertex.texcoord.x = float(lonIndex) / float(subdivision);
			vertex.texcoord.y = 1.0f - float(latIndex) / float(subdivision);

			// 法線は頂点位置と同じ（正規化された位置ベクトル）
			vertex.normal.x = vertex.position.x;
			vertex.normal.y = vertex.position.y;
			vertex.normal.z = vertex.position.z;

			vertices_.push_back(vertex);
		}
	}

	// インデックスデータの作成
	for (uint32_t latIndex = 0; latIndex < subdivision; ++latIndex) {
		for (uint32_t lonIndex = 0; lonIndex < subdivision; ++lonIndex) {
			// 各格子点のインデックスを計算
			uint32_t currentRow = latIndex * (subdivision + 1);
			uint32_t nextRow = (latIndex + 1) * (subdivision + 1);

			uint32_t currentIndex = currentRow + lonIndex;
			uint32_t rightIndex = currentRow + lonIndex + 1;
			uint32_t bottomIndex = nextRow + lonIndex;
			uint32_t bottomRightIndex = nextRow + lonIndex + 1;

			// 1つ目の三角形（左上、左下、右上）
			indices_.push_back(currentIndex);
			indices_.push_back(bottomIndex);
			indices_.push_back(rightIndex);

			// 2つ目の三角形（右上、左下、右下）
			indices_.push_back(rightIndex);
			indices_.push_back(bottomIndex);
			indices_.push_back(bottomRightIndex);
		}
	}

	CreateVertexBuffer();
	CreateIndexBuffer();
}

void Mesh::CreateSprite(const Vector2& center, const Vector2& size)
{

	// スプライト用の4頂点
	vertices_.resize(4);

	float halfWidth = size.x * 0.5f;
	float halfHeight = size.y * 0.5f;

	// 左下
	vertices_[0].position = { center.x - halfWidth, center.y + halfHeight, 0.0f, 1.0f };
	vertices_[0].texcoord = { 0.0f, 1.0f };


	// 左上
	vertices_[1].position = { center.x - halfWidth, center.y - halfHeight, 0.0f, 1.0f };
	vertices_[1].texcoord = { 0.0f, 0.0f };


	// 右下
	vertices_[2].position = { center.x + halfWidth, center.y + halfHeight, 0.0f, 1.0f };
	vertices_[2].texcoord = { 1.0f, 1.0f };


	// 右上
	vertices_[3].position = { center.x + halfWidth, center.y - halfHeight, 0.0f, 1.0f };
	vertices_[3].texcoord = { 1.0f, 0.0f };


	for (int i = 0; i < 4; i++)
	{
		vertices_[i].normal = { 0.0f,0.0f,-1.0f };
	}

	// インデックスデータ（2つの三角形）
	indices_ = { 0, 1, 2, 1, 3, 2 };

	CreateVertexBuffer();
	CreateIndexBuffer();
}

void Mesh::CreateModel(const ModelData& modelData)
{
	// ModelDataから頂点データをコピー
	vertices_ = modelData.vertices;

	// マテリアル情報をコピー
	material_ = modelData.material;

	// インデックスデータを生成（順番通り）
	indices_.clear();
	for (uint32_t i = 0; i < vertices_.size(); ++i) {
		indices_.push_back(i);
	}

	// バッファを作成
	CreateVertexBuffer();
	CreateIndexBuffer();
}

ModelData Mesh::LoadObjFile(const std::string& directoryPath, const std::string& filename) {
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



void Mesh::SetVertices(const std::vector<VertexData>& vertices)
{
	vertices_ = vertices;
	CreateVertexBuffer();
}

void Mesh::SetIndices(const std::vector<uint32_t>& indices)
{
	indices_ = indices;
	CreateIndexBuffer();
}

void Mesh::Bind(ID3D12GraphicsCommandList* commandList)
{
	// 頂点バッファをバインド
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);

	// インデックスバッファがあればバインド
	if (HasIndices()) {
		commandList->IASetIndexBuffer(&indexBufferView_);
	}
}

void Mesh::Draw(ID3D12GraphicsCommandList* commandList, uint32_t instanceCount)
{
	if (HasIndices()) {
		// インデックス描画
		commandList->DrawIndexedInstanced(GetIndexCount(), instanceCount, 0, 0, 0);
	} else {
		// 通常描画
		commandList->DrawInstanced(GetVertexCount(), instanceCount, 0, 0);
	}
}

void Mesh::UpdateBuffers()
{
	CreateVertexBuffer();
	CreateIndexBuffer();
}

void Mesh::CreateVertexBuffer()
{
	//																			//
	//							VertexResourceの作成								//
	//																			//
	// 頂点バッファを作成
	vertexBuffer_ = CreateBufferResource(directXCommon_->GetDevice(), sizeof(VertexData) * vertices_.size());

	//																			//
	//						Resourceにデータを書き込む								//
	//																			//

	// データを書き込み
	VertexData* vertexData = nullptr;
	vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, vertices_.data(), sizeof(VertexData) * vertices_.size());

	//																			//
	//							VertexBufferViewの作成							//
	//																			//
	// 頂点バッファビューを設定
	vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * vertices_.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);


}

void Mesh::CreateIndexBuffer()
{
	// インデックスデータがない時は何もしない(vertexのみの場合)
	if (indices_.empty()) {
		return;
	}
	//																			//
	//							indexResourceの作成								//
	//																			//

	// インデックスバッファを作成
	indexBuffer_ = CreateBufferResource(directXCommon_->GetDevice(), sizeof(uint32_t) * indices_.size());

	//																			//
	//						Resourceにデータを書き込む								//
	//																			//
	// データを書き込み
	uint32_t* indexData = nullptr;
	indexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	std::memcpy(indexData, indices_.data(), sizeof(uint32_t) * indices_.size());

	//																			//
	//							indexBufferViewの作成							//
	//																			//
	// インデックスバッファビューを設定
	indexBufferView_.BufferLocation = indexBuffer_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(uint32_t) * indices_.size());
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;


}

MaterialDataModel Mesh::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename)
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
