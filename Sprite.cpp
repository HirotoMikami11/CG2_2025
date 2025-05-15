#include "Sprite.h"

Sprite::Sprite() {
	position = { 0.0f, 0.0f };		//
	size = { 100.0f, 100.0f };		//サイズは一応100,100にしておく

	// UVTransformの初期化
	uvTransform.scale = { 1.0f, 1.0f, 1.0f };
	uvTransform.rotate = { 0.0f, 0.0f, 0.0f };
	uvTransform.translate = { 0.0f, 0.0f, 0.0f };
}

Sprite::~Sprite() {

}

void Sprite::Initialize(ID3D12Device* device) {
	// VertexResourceの作成（4頂点分）
	vertexResource = CreateBufferResource(device, sizeof(VertexData) * 4);

	// IndexResourceの作成（2つの三角形 = 6つのインデックス）
	indexResource = CreateBufferResource(device, sizeof(uint32_t) * 6);

	// VertexBufferViewとIndexBufferViewの作成
	CreateBufferViews();

	// Material用のResourceを作る
	materialResource = CreateBufferResource(device, sizeof(Material));
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));

	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData->enableLighting = false;
	materialData->useLambertianReflectance = false;
	materialData->uvTransform = MakeIdentity4x4();

	// TransformationMatrix用のリソースを作る
	transformationMatrixResource = CreateBufferResource(device, sizeof(TransformationMatrix));
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	transformationMatrixData->WVP = MakeIdentity4x4();
	transformationMatrixData->World = MakeIdentity4x4();

	// Resourceにデータを書き込む
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	uint32_t* indexData = nullptr;
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));

	CreateVertexData();
	CreateIndexData();
}

void Sprite::CreateVertexData() {
	// centerは現在の位置、radiusはsize/2として計算
	Vector2 center = position;
	Vector2 radius = { size.x * 0.5f, size.y * 0.5f };

	// 元のCreateSpriteVertexDataと同じ頂点順序で作成
	// 左下
	vertexData[0].position = { center.x - radius.x, center.y + radius.y, 0.0f, 1.0f };
	vertexData[0].texcoord = { 0.0f, 1.0f };

	// 左上
	vertexData[1].position = { center.x - radius.x, center.y - radius.y, 0.0f, 1.0f };
	vertexData[1].texcoord = { 0.0f, 0.0f };

	// 右下
	vertexData[2].position = { center.x + radius.x, center.y + radius.y, 0.0f, 1.0f };
	vertexData[2].texcoord = { 1.0f, 1.0f };

	// 右上
	vertexData[3].position = { center.x + radius.x, center.y - radius.y, 0.0f, 1.0f };
	vertexData[3].texcoord = { 1.0f, 0.0f };

	// 法線情報（元のコードに合わせて-Z方向）
	for (int i = 0; i < 4; i++) {
		vertexData[i].normal = { 0.0f, 0.0f, -1.0f };
	}
}

void Sprite::CreateIndexData() {
	uint32_t* indexData = nullptr;
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));

	// 2つの三角形でスプライトを構成
	// 左上の三角形
	indexData[0] = 0; indexData[1] = 1; indexData[2] = 2;
	// 右下の三角形
	indexData[3] = 1; indexData[4] = 3; indexData[5] = 2;

	indexResource->Unmap(0, nullptr);
}

void Sprite::CreateBufferViews() {
	// VertexBufferViewの作成
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 4;
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// IndexBufferViewの作成
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
}

void Sprite::UpdateVertexData() {
	if (vertexData == nullptr) return;

	// centerは現在の位置、radiusはsize/2として計算
	Vector2 center = position;
	Vector2 radius = { size.x * 0.5f, size.y * 0.5f };

	// 頂点座標を再計算（元のCreateSpriteVertexDataと同じ順序）
	// 左下
	vertexData[0].position = { center.x - radius.x, center.y + radius.y, 0.0f, 1.0f };
	// 左上
	vertexData[1].position = { center.x - radius.x, center.y - radius.y, 0.0f, 1.0f };
	// 右下
	vertexData[2].position = { center.x + radius.x, center.y + radius.y, 0.0f, 1.0f };
	// 右上
	vertexData[3].position = { center.x + radius.x, center.y - radius.y, 0.0f, 1.0f };
}

void Sprite::Update() {
	// スプライト用のViewProjection行列を計算
	Matrix4x4 viewProjectionMatrixSprite = MakeViewProjectionMatrixSprite();

	// WVP行列を更新（スプライトは基本的にワールド変換なし）
	transformationMatrixData->WVP = viewProjectionMatrixSprite;
	transformationMatrixData->World = MakeIdentity4x4();

	// UVTransformをマテリアルに適用
	UpdateUVTransform(uvTransform, materialData);
}

void Sprite::Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle) {
	// マテリアルのCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	// TransformMatrixCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());
	// テクスチャのSRVの場所を設定
	commandList->SetGraphicsRootDescriptorTable(2, textureHandle);

	// VBとIBを設定
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->IASetIndexBuffer(&indexBufferView);

	// 描画（2つの三角形で矩形を描画）
	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}