#include "Triangle.h"

void Triangle::Initialize(DirectXCommon* directX) {
	// 基底クラスのリソース作成
	CreateResources(directX);

	// マテリアル初期設定
	material.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	material.enableLighting = false;
	material.useLambertianReflectance = false;
	material.uvTransform = MakeIdentity4x4();
	*materialData = material;

	// 頂点リソース作成
	vertexResource = CreateBufferResource(directX->GetDevice(), sizeof(VertexData) * 3);

	// 頂点バッファビュー作成
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 3;
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// 頂点データ設定
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	CreateVertexData();
}

void Triangle::Update() {
	// 回転
	//transform.rotate.y += 0.03f;

	// マテリアルデータ更新
	*materialData = material;
}

void Triangle::Draw(DirectXCommon* directX) {
	// コマンドリストに描画コマンドを積む
	auto commandList = directX->GetCommandList();

	commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(1, transformResource->GetGPUVirtualAddress());
	//commandList->SetGraphicsRootDescriptorTable(2, directX->GetTextureGPUSrvHandles()[0]);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->DrawInstanced(3, 1, 0, 0);
}

void Triangle::Finalize() {
	// リソースの解放はComPtrが自動的に行う
}

void Triangle::CreateVertexData() {
	// 一つ目の三角形
	vertexData[0].position = { -0.5f, -0.5f, 0.0f, 1.0f };
	vertexData[0].texcoord = { 0.0f, 1.0f };
	vertexData[1].position = { 0.0f, 0.5f, 0.0f, 1.0f };
	vertexData[1].texcoord = { 0.5f, 0.0f };
	vertexData[2].position = { 0.5f, -0.5f, 0.0f, 1.0f };
	vertexData[2].texcoord = { 1.0f, 1.0f };

	// 法線設定
	for (int i = 0; i < 3; i++) {
		vertexData[i].normal.x = vertexData[i].position.x;
		vertexData[i].normal.y = vertexData[i].position.y;
		vertexData[i].normal.z = vertexData[i].position.z;
	}
}
