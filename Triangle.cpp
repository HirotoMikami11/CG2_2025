#include "Triangle.h"


Triangle::Triangle() {

	transform.scale = { 1.0f, 1.0f, 1.0f };
	transform.rotate = { 0.0f, 0.0f, 0.0f };
	transform.translate = { 0.0f, 0.0f, 0.0f };
}

Triangle::~Triangle() {

}

void Triangle::Initialize(ID3D12Device* device) {
	//							VertexResourceの作成								//
	vertexResource = CreateBufferResource(device, sizeof(VertexData) * 3);
	//							VertexBufferViewの作成							//
	CreateBufferViews();

	//							Material用のResourceを作る						//
	materialResource = CreateBufferResource(device, sizeof(Material));
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));

	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData->enableLighting = false;
	materialData->useLambertianReflectance = false;
	materialData->uvTransform = MakeIdentity4x4();

	//					TransformationMatrix用のリソースを作る						//
	wvpResource = CreateBufferResource(device, sizeof(TransformationMatrix));
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	wvpData->WVP = MakeIdentity4x4();
	wvpData->World = MakeIdentity4x4();

	//						Resourceにデータを書き込む								//
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	CreateVertexData();



}

void Triangle::CreateVertexData() {
	//一つ目の三角形
	//左下
	vertexData[0].position = { -0.5f,-0.5f,0.0f,1.0f };
	vertexData[0].texcoord = { 0.0f,1.0f };
	//上
	vertexData[1].position = { 0.0f,0.5f,0.0f,1.0f };
	vertexData[1].texcoord = { 0.5f,0.0f };
	//右下
	vertexData[2].position = { 0.5f,-0.5f,0.0f,1.0f };
	vertexData[2].texcoord = { 1.0f,1.0f };

	//法線情報(三角形なので別のを後で用意)
	for (int i = 0; i < 3; i++)
	{
		vertexData[i].normal.x = vertexData[i].position.x;
		vertexData[i].normal.y = vertexData[i].position.y;
		vertexData[i].normal.z = vertexData[i].position.z;
	}
}

void Triangle::CreateBufferViews() {
	//リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	//仕様数リソースのサイズは頂点3つ分のサイズ
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 3; //２つ三角形を作るので６個の頂点データ
	//1頂点当たりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);
}

void Triangle::Update(const Matrix4x4& viewProjectionMatrix) {
	UpdateMatrix4x4(transform, viewProjectionMatrix, wvpData);
}

void Triangle::Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle) {

	//マテリアルのCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	//wvp用のCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
	//テクスチャのSRVの場所を設定
	commandList->SetGraphicsRootDescriptorTable(2, textureHandle);
	//VBを設定
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

	// 描画！（DrawCall／ドローコール）。３頂点で1つのインスタンス
	commandList->DrawInstanced(3, 1, 0, 0);
}
