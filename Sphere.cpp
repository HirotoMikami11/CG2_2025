#include "Sphere.h"

Sphere::Sphere() {
	transform.scale = { 1.0f, 1.0f, 1.0f };
	transform.rotate = { 0.0f, 0.0f, 0.0f };
	transform.translate = { 0.0f, 0.0f, 0.0f };
}

Sphere::~Sphere() {

}

void Sphere::Initialize(ID3D12Device* device) {
	//							VertexResourceの作成								//
	vertexResource = CreateBufferResource(device, sizeof(VertexData) * (kSubdivision + 1) * (kSubdivision + 1));

	//							indexResourceの作成								//
	indexResource = CreateBufferResource(device, sizeof(uint32_t) * kSubdivision * kSubdivision * 6);


	//							BufferViewの作成									//
	CreateBufferViews();

	//							Material用のResourceを作る						//
	materialResource = CreateBufferResource(device, sizeof(Material));
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));

	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData->enableLighting = true;
	materialData->useLambertianReflectance = false;
	materialData->uvTransform = MakeIdentity4x4();

	//					TransformationMatrix用のリソースを作る						//
	wvpResource = CreateBufferResource(device, sizeof(TransformationMatrix));
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	wvpData->WVP = MakeIdentity4x4();
	wvpData->World = MakeIdentity4x4();

	// DirectionalLight用のResourceを作る
	directionalLightResource = CreateBufferResource(device, sizeof(DirectionalLight));
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));

	// デフォルトのライト設定
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData->intensity = 1.0f;

	//						Resourceにデータを書き込む								//
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	CreateVertexData();
	CreateIndexData();

}

void Sphere::CreateVertexData() {
	// 球体のデータを作成
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

void Sphere::CreateIndexData() {
	// 球体のインデックスデータを作成
	const float kLonEvery = (2 * float(M_PI)) / kSubdivision; // 経度分割1つ分の角度
	const float kLatEvery = float(M_PI) / kSubdivision; // 緯度分割1つ分の角度
	uint32_t indexCount = 0;
	uint32_t maxIndex = kSubdivision * kSubdivision * 6; // 三角リスト方式の頂点数と同じ数

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

void Sphere::CreateBufferViews() {
	// VertexBufferViewの作成
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VertexData) * (kSubdivision + 1) * (kSubdivision + 1);
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// IndexBufferViewの作成
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = sizeof(uint32_t) * kSubdivision * kSubdivision * 6;
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
}

void Sphere::Update(const Matrix4x4& viewProjectionMatrix) {
	UpdateMatrix4x4(transform, viewProjectionMatrix, wvpData);
}

void Sphere::Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle) {
	// マテリアルのCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	// wvp用のCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
	// テクスチャのSRVの場所を設定
	commandList->SetGraphicsRootDescriptorTable(2, textureHandle);
	// DirectionalLightのCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

	// VBとIBを設定
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->IASetIndexBuffer(&indexBufferView);

	// 描画！（DrawCall／ドローコール）
	commandList->DrawIndexedInstanced(kSubdivision * kSubdivision * 6, 1, 0, 0, 0);
}