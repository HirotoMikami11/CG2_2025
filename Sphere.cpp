#include "Sphere.h"

void Sphere::Initialize(DirectXCommon* directX) {
	// 基底クラスのリソース作成
	CreateResources(directX);

	// マテリアル初期設定
	material.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	material.enableLighting = true;
	material.useLambertianReflectance = false;
	material.uvTransform = MakeIdentity4x4();
	*materialData = material;

	// 頂点リソース作成
	size_t vertexCount = (kSubdivision + 1) * (kSubdivision + 1);
	vertexResource = CreateBufferResource(directX->GetDevice(), sizeof(VertexData) * vertexCount);

	// 頂点バッファビュー設定
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VertexData) * vertexCount;
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// インデックスリソース作成
	size_t indexCount = kSubdivision * kSubdivision * 6;
	indexResource = CreateBufferResource(directX->GetDevice(), sizeof(uint32_t) * indexCount);

	// インデックスバッファビュー設定
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = sizeof(uint32_t) * indexCount;
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	// ディレクショナルライトリソース作成
	directionalLightResource = CreateBufferResource(directX->GetDevice(), sizeof(DirectionalLight));
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData->intensity = 1.0f;

	// 頂点・インデックスデータ作成
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	CreateSphereVertices();
	CreateSphereIndices();
}

void Sphere::Update() {
	// 回転
	transform.rotate.y += 0.01f;

	// マテリアルデータ更新
	*materialData = material;
}

void Sphere::Draw(DirectXCommon* directX) {
	auto commandList = directX->GetCommandList();

	commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(1, transformResource->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
	//commandList->SetGraphicsRootDescriptorTable(2, directX->GetTextureGPUSrvHandles()[1]); // モンスターボール
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->IASetIndexBuffer(&indexBufferView);
	commandList->DrawIndexedInstanced(kSubdivision * kSubdivision * 6, 1, 0, 0, 0);
}

void Sphere::Finalize() {
	// ComPtrが自動的にリソースを解放
}

void Sphere::SetDirectionalLight(const DirectionalLight& light) {
	*directionalLightData = light;
}

void Sphere::CreateSphereVertices() {

	const float kLonEvery = (2 * float(M_PI)) / kSubdivision; // 経度分割1つ分の角度
	const float kLatEvery = float(M_PI) / kSubdivision; // 緯度分割1つ分の角度
	uint32_t vertexCount = 0;

	// 頂点データの作成
	// (kSubdivision+1) × (kSubdivision+1) の格子状の頂点を作成
	for (uint32_t latIndex = 0; latIndex <= kSubdivision; ++latIndex) {
		float lat = (-float(M_PI) / 2.0f) + kLatEvery * latIndex; // 現在の緯度(θ)
		for (uint32_t lonIndex = 0; lonIndex <= kSubdivision; ++lonIndex) {
			float lon = lonIndex * kLonEvery; // 現在の経度(φ)

			// 頂点位置の計算
			vertexData[vertexCount].position.x = cos(lat) * cos(lon);
			vertexData[vertexCount].position.y = sin(lat);
			vertexData[vertexCount].position.z = cos(lat) * sin(lon);
			vertexData[vertexCount].position.w = 1.0f;

			// テクスチャ座標の計算
			vertexData[vertexCount].texcoord.x = float(lonIndex) / float(kSubdivision);
			vertexData[vertexCount].texcoord.y = 1.0f - float(latIndex) / float(kSubdivision);

			// 法線は頂点位置と同じ（正規化された位置ベクトル）
			vertexData[vertexCount].normal.x = vertexData[vertexCount].position.x;
			vertexData[vertexCount].normal.y = vertexData[vertexCount].position.y;
			vertexData[vertexCount].normal.z = vertexData[vertexCount].position.z;

			vertexCount++;
		}
	}
}

void Sphere::CreateSphereIndices() {
	//int indexCount = 0;

	//for (int latIndex = 0; latIndex < kSubdivision; ++latIndex) {
	//	for (int lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
	//		// 四角形の4つの頂点
	//		int a = latIndex * (kSubdivision + 1) + lonIndex;
	//		int b = a + 1;
	//		int c = (latIndex + 1) * (kSubdivision + 1) + lonIndex;
	//		int d = c + 1;

	//		// 最初の三角形
	//		indexData[indexCount++] = a;
	//		indexData[indexCount++] = c;
	//		indexData[indexCount++] = b;

	//		// 二番目の三角形
	//		indexData[indexCount++] = b;
	//		indexData[indexCount++] = c;
	//		indexData[indexCount++] = d;
	//	}
	//}

	const float kLonEvery = (2 * float(M_PI)) / kSubdivision; // 経度分割1つ分の角度
	const float kLatEvery = float(M_PI) / kSubdivision; // 緯度分割1つ分の角度
	uint32_t indexCount = 0;
	uint32_t maxIndex = kSubdivision * kSubdivision * 6;//三角リスト方式の頂点数と同じ数

	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			// 各格子点のインデックスを計算
			uint32_t currentRow = latIndex * (kSubdivision + 1);
			uint32_t nextRow = (latIndex + 1) * (kSubdivision + 1);

			uint32_t currentIndex = currentRow + lonIndex;
			uint32_t rightIndex = currentRow + lonIndex + 1;
			uint32_t bottomIndex = nextRow + lonIndex;
			uint32_t bottomRightIndex = nextRow + lonIndex + 1;

			// 1つ目の三角形（左上、左下、右上）
			indexData[indexCount++] = currentIndex;
			indexData[indexCount++] = bottomIndex;
			indexData[indexCount++] = rightIndex;

			// 2つ目の三角形（右上、左下、右下）
			indexData[indexCount++] = rightIndex;
			indexData[indexCount++] = bottomIndex;
			indexData[indexCount++] = bottomRightIndex;
		}
	}

}
