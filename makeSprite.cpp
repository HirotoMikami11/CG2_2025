#include "makeSprite.h"
makeSprite::~makeSprite()
{
	vertexResourceSprite->Release();
	transformationMatrixResourceSprite->Release();
}

void makeSprite::Initialize(ID3D12Device* device, Vector2& centerPosition, Vector2& size)
{

	//																			//
	//							VertexResourceの作成								//
	//																			//

	//実際に頂点リソースを生成
	vertexResourceSprite = CreateBufferResource(device, sizeof(VertexData) * 6); //２つ三角形で矩形を作るので頂点データ6つ
	//																			//
	//							VertexBufferViewの作成							//
	//																			//

	//頂点バッファビューを作成
	//リソースの戦闘のアドレスから使う
	vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	//仕様数リソースのサイズは頂点3つ分のサイズ
	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 6; //２つ三角形を作るので６個の頂点データ
	//1頂点当たりのサイズ
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);

	//																			//
	//					TransformationMatrix用のリソースを作る						//
	//																			//

//WVP用のリソースを作る、Matrix4x4　１つ分のサイズを用意する
	transformationMatrixResourceSprite = CreateBufferResource(device, sizeof(Matrix4x4));
	//データを書き込む
	transformationMatrixDataSprite = nullptr;
	//書き込むためのアドレスを取得
	transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));
	//単位行列を書き込んでおく
	*transformationMatrixDataSprite = MakeIdentity4x4();
	//																			//
	//						Resourceにデータを書き込む								//
	//																			//

	//頂点リソースにデータを書き込む
	VertexData* vertexDataSprite = nullptr;
	//書き込むためのアドレスを取得
	vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));
	SetVertexDataSpriteSquare(vertexDataSprite, centerPosition, size);



}

void makeSprite::Update(const Matrix4x4& viewProjectionMatrix)
{
	//viewprojectionを計算
	Matrix4x4 viewProjectionMatrixSprite = MakeViewProjectionMatrixSprite();
	//行列の更新
	UpdateMatrix4x4(transformSprite, viewProjectionMatrix, transformationMatrixDataSprite);

}

void makeSprite::Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle)
{

	//TransformMatrixCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(2, textureHandle);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);
	// 描画（DrawCall／ドローコール)
	//三角形を二つ描画するので6つ
	commandList->DrawInstanced(6, 1, 0, 0);


}




/// <summary>
/// Resourceを生成する関数
/// </summary>
/// <param name="device">ID3D12Device</param>
/// <param name="sizeInBytes">リソースサイズ</param>
/// <returns></returns>
ID3D12Resource* makeSprite::CreateBufferResource(ID3D12Device* device, size_t sizeInBytes) {
	//リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;//UploadHeapを使う
	//リソースの設定
	D3D12_RESOURCE_DESC ResourceDesc{};
	//バッファリソース。テクスチャの場合はまた別の設定をする
	ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	ResourceDesc.Width = sizeInBytes;//リソースサイズ
	//バッファの場合はこれらは1にする決まり
	ResourceDesc.Height = 1;
	ResourceDesc.DepthOrArraySize = 1;
	ResourceDesc.MipLevels = 1;
	ResourceDesc.SampleDesc.Count = 1;
	//バッファの場合はこれにする決まり
	ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//実際にリソースを生成
	ID3D12Resource* Resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&ResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&Resource)
	);
	assert(SUCCEEDED(hr));


	return Resource;
}
