#include "Mesh.h"
void Mesh::Initialize(DirectXCommon* dxCommon)
{
	directXCommon_ = dxCommon;
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

void Mesh::LoadFromOBJ(const std::string& directoryPath, const std::string& filename)
{

	std::vector<Vector4> positions;
	std::vector<Vector3> normals;
	std::vector<Vector2> texcoords;
	std::string line;


	// ファイルを開く
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	// ファイルを読み込み
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "v") {
			// 頂点位置
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			positions.push_back(position);

		} else if (identifier == "vt") {
			// テクスチャ座標
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoords.push_back(texcoord);

		} else if (identifier == "vn") {
			// 法線
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);

		} else if (identifier == "f") {
			// 面（三角形限定）
			VertexData triangle[3];

			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;

				std::stringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/');
					elementIndices[element] = std::stoi(index);
				}

				// 頂点データを構成
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];

				// 右手座標系から左手座標系に変換
				position.x *= -1.0f;
				normal.x *= -1.0f;
				texcoord.y = 1.0f - texcoord.y;

				triangle[faceVertex] = { position, texcoord, normal };
			}

			// 頂点順序を調整（時計回り→反時計回り）
			vertices_.push_back(triangle[2]);
			vertices_.push_back(triangle[1]);
			vertices_.push_back(triangle[0]);

		} else if (identifier == "mtllib") {
			//materialTemplateLibraryファイルの名前を取得する
			std::string materialFilename;
			s >> materialFilename;

			//objファイルとmtlファイルは同じディレクトリにあるので、ディレクトリ・ファイル名を渡す
			material_ = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}

	// インデックスデータを生成（順番通り）
	for (uint32_t i = 0; i < vertices_.size(); ++i) {
		indices_.push_back(i);
	}


	CreateVertexBuffer();
	CreateIndexBuffer();
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
