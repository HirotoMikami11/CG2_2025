#include "TriangularPyramid.h"

TriangularPyramid::TriangularPyramid() {
	transform.scale = { 1.0f, 1.0f, 1.0f };
	transform.rotate = { 0.0f, 0.0f, 0.0f };
	transform.translate = { 0.0f, 0.0f, 0.0f };
}

TriangularPyramid::~TriangularPyramid() {
	// 解放処理
	if (vertexResource) {
		vertexResource->Release();
	}

	if (materialResource) {
		materialResource->Release();
	}

	if (wvpResource) {
		wvpResource->Release();
	}

	if (indexResource) {
		indexResource->Release();
	}
}

void TriangularPyramid::Initialize(ID3D12Device* device) {
	// VertexResourceの作成
	vertexResource = CreateBufferResource(device, sizeof(VertexData) * kVertexCount);

	// IndexResourceの作成
	indexResource = CreateBufferResource(device, sizeof(uint16_t) * kIndexCount);

	// BufferViewの作成
	CreateBufferViews();

	// Material用のResourceを作る
	materialResource = CreateBufferResource(device, sizeof(Material));
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));

	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData->enableLighting = false;
	materialData->useLambertianReflectance = false;
	materialData->uvTransform = MakeIdentity4x4();

	// TransformationMatrix用のリソースを作る
	wvpResource = CreateBufferResource(device, sizeof(TransformationMatrix));
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	wvpData->WVP = MakeIdentity4x4();
	wvpData->World = MakeIdentity4x4();

	// 頂点データの作成
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	CreateVertexData();

	// インデックスデータの作成
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	CreateIndexData();
}

void TriangularPyramid::CreateVertexData() {
	// 三角錐の頂点座標を定義
	// 底面の三角形
	vertexData[0].position = { -0.5f, 0.0f, -0.5f, 1.0f };  // 底面左
	vertexData[1].position = { 0.5f, 0.0f, -0.5f, 1.0f };   // 底面右
	vertexData[2].position = { 0.0f, 0.0f, 0.5f, 1.0f };    // 底面中央前

	// 頂点
	vertexData[3].position = { 0.0f, 1.0f, 0.0f, 1.0f };    // 頂点

	// テクスチャ座標を設定
	// 底面の頂点
	vertexData[0].texcoord = { 0.0f, 1.0f };
	vertexData[1].texcoord = { 1.0f, 1.0f };
	vertexData[2].texcoord = { 0.5f, 0.0f };

	// 頂点
	vertexData[3].texcoord = { 0.5f, 0.5f };

	// 法線を計算
	// 底面の法線（下向きに統一）
	vertexData[0].normal = { 0.0f, -1.0f, 0.0f };
	vertexData[1].normal = { 0.0f, -1.0f, 0.0f };
	vertexData[2].normal = { 0.0f, -1.0f, 0.0f };

	// 頂点の法線は上向き（簡易版 - 実際は各面に対応した法線を設定するべき）
	vertexData[3].normal = { 0.0f, 1.0f, 0.0f };
}

void TriangularPyramid::CreateIndexData() {
	// 底面の三角形（時計回り、下から見た場合）
	//背面カリングで、下から見るとき透明になってしまうため、時計回りに変更
	indexData[0] = 1;
	indexData[1] = 2;
	indexData[2] = 0;

	// 側面1：頂点と底面の左辺と前辺をつなぐ三角形
	indexData[3] = 3;
	indexData[4] = 0;
	indexData[5] = 2;

	// 側面2：頂点と底面の前辺と右辺をつなぐ三角形
	indexData[6] = 3;
	indexData[7] = 2;
	indexData[8] = 1;

	// 側面3：頂点と底面の右辺と左辺をつなぐ三角形
	indexData[9] = 3;
	indexData[10] = 1;
	indexData[11] = 0;
}

void TriangularPyramid::CreateBufferViews() {
	// 頂点バッファビューの設定
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VertexData) * kVertexCount;
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// インデックスバッファビューの設定
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = sizeof(uint16_t) * kIndexCount;
	indexBufferView.Format = DXGI_FORMAT_R16_UINT; // UINT16のインデックス
}

void TriangularPyramid::Update(const Matrix4x4& viewProjectionMatrix) {
	UpdateMatrix4x4(transform, viewProjectionMatrix, wvpData);
}

void TriangularPyramid::Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle) {
	// マテリアルのCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

	// wvp用のCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());

	// テクスチャのSRVの場所を設定
	commandList->SetGraphicsRootDescriptorTable(2, textureHandle);

	// VBを設定
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

	// IBを設定
	commandList->IASetIndexBuffer(&indexBufferView);

	// 描画（DrawCall/ドローコール)
	commandList->DrawIndexedInstanced(kIndexCount, 1, 0, 0, 0);
}