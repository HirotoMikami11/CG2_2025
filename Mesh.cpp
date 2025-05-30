#include "Mesh.h"

void Mesh::Initialize(DirectXCommon* dxCommon, MeshType meshType)
{
	directXCommon_ = dxCommon;
	meshType_ = meshType;

	// メッシュタイプに応じて対応するcreate関数を呼び出す
	switch (meshType_) {
	case MeshType::TRIANGLE:
		CreateTriangle();
		break;

	case MeshType::SPHERE:
		CreateSphere(16); // デフォルトで16分割
		break;

	case MeshType::SPRITE:
		CreateSprite({ 160.0f, 90.0f }, { 320.0f, 180.0f });
		break;

	case MeshType::MODEL_OBJ:
		// OBJファイルの場合はFormDataを使用するのでアサ―ト
		assert(false && "Use InitializeFromOBJ for OBJ files");
		break;

	default:
		// 未対応のタイプ(知らんと返す)
		assert(false && "Unkown");
		CreateTriangle();
		break;
	}
}

void Mesh::InitializeFromData(DirectXCommon* dxCommon, const ModelData& modelData)
{
	directXCommon_ = dxCommon;
	meshType_ = MeshType::MODEL_OBJ;

	// データからモデルを作成
	CreateModel(modelData);
}

std::string Mesh::MeshTypeToString(MeshType type)
{
	switch (type) {
	case MeshType::TRIANGLE:     return "Triangle";
	case MeshType::SPHERE:       return "Sphere";
	case MeshType::SPRITE:       return "Sprite";
	case MeshType::MODEL_OBJ:    return "Model_OBJ";
	default:                     return "Unknown";
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


	// 面法線を計算して設定
	CalculateTriangleNormals();

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

void Mesh::CalculateTriangleNormals()
{
	// 三角形の各頂点位置を取得
	Vector3 v0 = { vertices_[0].position.x, vertices_[0].position.y, vertices_[0].position.z };
	Vector3 v1 = { vertices_[1].position.x, vertices_[1].position.y, vertices_[1].position.z };
	Vector3 v2 = { vertices_[2].position.x, vertices_[2].position.y, vertices_[2].position.z };

	// エッジベクトルを計算
	Vector3 edge1 = Vector3Subtract(v1, v0);
	Vector3 edge2 = Vector3Subtract(v2, v0);

	// 外積で面法線を計算
	Vector3 normal = Cross(edge1, edge2);
	normal = Vector3Normalize(normal);  // 正規化

	// 全ての頂点に同じ法線を設定（平面なので）
	for (size_t i = 0; i < vertices_.size(); ++i) {
		vertices_[i].normal = normal;
	}
}