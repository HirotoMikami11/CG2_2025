#include "TriangularPrism.h"

TriangularPrism::TriangularPrism() {
	///トライフォース風にしたいので、z方向のサイズをデフォルトで短くする
	transform.scale = { 1.0f, 1.0f, 0.25f };
	transform.rotate = { 0.0f, 0.0f, 0.0f };
	transform.translate = { 0.0f, 0.0f, 0.0f };
}

TriangularPrism::~TriangularPrism() {
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

void TriangularPrism::Initialize(ID3D12Device* device) {
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

void TriangularPrism::CreateVertexData() {
	// 三角柱の頂点座標を定義
	// 正面三角形
	vertexData[0].position = { -0.5f, -0.5f, 0.5f, 1.0f };  // 左下
	vertexData[1].position = { 0.5f, -0.5f, 0.5f, 1.0f };  // 右下
	vertexData[2].position = { 0.0f,  0.5f, 0.5f, 1.0f };  // 上中央

	// 背面三角形
	vertexData[3].position = { -0.5f, -0.5f, -0.5f, 1.0f }; // 左下
	vertexData[4].position = { 0.5f, -0.5f, -0.5f, 1.0f }; // 右下
	vertexData[5].position = { 0.0f,  0.5f, -0.5f, 1.0f }; // 上中央

	// テクスチャ座標を設定
	// 上部の頂点
	vertexData[0].texcoord = { 0.0f, 0.0f };
	vertexData[1].texcoord = { 1.0f, 0.0f };
	vertexData[2].texcoord = { 0.5f, 1.0f };

	// 下部の頂点
	vertexData[3].texcoord = { 0.0f, 0.0f };
	vertexData[4].texcoord = { 1.0f, 0.0f };
	vertexData[5].texcoord = { 0.5f, 1.0f };

	// 法線を計算 (簡易版 - 各面ごとに正確な法線を計算すべき)
	for (int i = 0; i < kVertexCount; i++) {
		// 頂点座標を正規化して法線として使用（近似）
		float length = sqrt(
			vertexData[i].position.x * vertexData[i].position.x +
			vertexData[i].position.y * vertexData[i].position.y +
			vertexData[i].position.z * vertexData[i].position.z
		);

		// ゼロ除算を防ぐ
		if (length > 0.0f) {
			vertexData[i].normal.x = vertexData[i].position.x / length;
			vertexData[i].normal.y = vertexData[i].position.y / length;
			vertexData[i].normal.z = vertexData[i].position.z / length;
		} else {
			vertexData[i].normal = { 0.0f, 1.0f, 0.0f }; // デフォルトの法線
		}
	}
}

void TriangularPrism::CreateIndexData() {
	// 上面の三角形（反時計回り）
	indexData[0] = 0;
	indexData[1] = 2;
	indexData[2] = 1;

	// 下面の三角形（反時計回り、下から見た場合）
	indexData[3] = 3;
	indexData[4] = 4;
	indexData[5] = 5;

	// 側面1（長方形 = 2つの三角形）：左面
	indexData[6] = 0;
	indexData[7] = 3;
	indexData[8] = 2;

	indexData[9] = 2;
	indexData[10] = 3;
	indexData[11] = 5;

	// 側面2（長方形 = 2つの三角形）：右面
	indexData[12] = 2;
	indexData[13] = 5;
	indexData[14] = 1;

	indexData[15] = 1;
	indexData[16] = 5;
	indexData[17] = 4;

	// 側面3（長方形 = 2つの三角形）：背面
	indexData[18] = 0;
	indexData[19] = 1;
	indexData[20] = 3;

	indexData[21] = 3;
	indexData[22] = 1;
	indexData[23] = 4;
}

void TriangularPrism::CreateBufferViews() {
	// 頂点バッファビューの設定
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VertexData) * kVertexCount;
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// インデックスバッファビューの設定
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = sizeof(uint16_t) * kIndexCount;
	indexBufferView.Format = DXGI_FORMAT_R16_UINT; // UINT16のインデックス
}

void TriangularPrism::Update(const Matrix4x4& viewProjectionMatrix) {
	UpdateMatrix4x4(transform, viewProjectionMatrix, wvpData);
}

void TriangularPrism::Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle) {
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

	// 描画（DrawCall/ドローコール）- インデックス使用版
	commandList->DrawIndexedInstanced(kIndexCount, 1, 0, 0, 0);
}