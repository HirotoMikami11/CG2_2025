#include "Line.h"
#include "Managers/Texture/TextureManager.h"

// 静的変数の定義
DirectXCommon* Line::directXCommon_ = nullptr;

void Line::InitializeStatic(DirectXCommon* dxCommon)
{
	directXCommon_ = dxCommon;
}

void Line::Initialize()
{
	// 頂点バッファ作成
	UpdateVertexBuffer();

	// マテリアルバッファ作成
	materialBuffer_ = CreateBufferResource(directXCommon_->GetDevice(), sizeof(LineMaterial));
	materialBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	UpdateMaterialBuffer();

	// トランスフォームバッファ作成
	transformBuffer_ = CreateBufferResource(directXCommon_->GetDevice(), sizeof(TransformationMatrix));
	transformBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&transformData_));

	isInitialized_ = true;
	needsVertexUpdate_ = false;
	needsMaterialUpdate_ = false;
}

void Line::SetPoints(const Vector3& start, const Vector3& end)
{
	start_ = start;
	end_ = end;
	needsVertexUpdate_ = true;
}

void Line::SetColor(const Vector4& color)
{
	color_ = color;
	needsMaterialUpdate_ = true;
}

void Line::Update(const Matrix4x4& viewProjectionMatrix)
{
	if (!isInitialized_) {
		return;
	}

	// 頂点バッファ更新
	if (needsVertexUpdate_) {
		UpdateVertexBuffer();
		needsVertexUpdate_ = false;
	}

	// マテリアル更新
	if (needsMaterialUpdate_) {
		UpdateMaterialBuffer();
		needsMaterialUpdate_ = false;
	}

	// トランスフォーム更新（毎フレーム）
	if (transformData_) {
		transformData_->WVP = viewProjectionMatrix;
		transformData_->World = MakeIdentity4x4();
	}
}

void Line::Draw(const Matrix4x4& viewProjectionMatrix)
{
	if (!isVisible_ || !isInitialized_) {
		return;
	}

	// 更新処理
	Update(viewProjectionMatrix);

	ID3D12GraphicsCommandList* commandList = directXCommon_->GetCommandList();

	// 線分用のPSOを設定
	commandList->SetGraphicsRootSignature(directXCommon_->GetLineRootSignature());
	commandList->SetPipelineState(directXCommon_->GetLinePipelineState());

	// マテリアル設定（RootParameter[0]: PixelShader用）
	commandList->SetGraphicsRootConstantBufferView(0, materialBuffer_->GetGPUVirtualAddress());

	// トランスフォーム設定（RootParameter[1]: VertexShader用）
	commandList->SetGraphicsRootConstantBufferView(1, transformBuffer_->GetGPUVirtualAddress());

	// プリミティブトポロジを線分に設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

	// 頂点バッファをバインド
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);

	// 描画（線分なので頂点数は2）
	commandList->DrawInstanced(2, 1, 0, 0);

	// 3D用のPSOに戻す
	commandList->SetGraphicsRootSignature(directXCommon_->GetRootSignature());
	commandList->SetPipelineState(directXCommon_->GetPipelineState());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Line::GetVertexData(VertexData& startVertex, VertexData& endVertex) const
{
	// 開始点の頂点データ
	startVertex.position = { start_.x, start_.y, start_.z, 1.0f };
	startVertex.texcoord = { 0.0f, 0.0f };
	startVertex.normal = { 0.0f, 1.0f, 0.0f }; // 線分には不要だが構造体として必要

	// 終了点の頂点データ
	endVertex.position = { end_.x, end_.y, end_.z, 1.0f };
	endVertex.texcoord = { 1.0f, 1.0f };
	endVertex.normal = { 0.0f, 1.0f, 0.0f }; // 線分には不要だが構造体として必要
}

void Line::DrawLines(
	ID3D12GraphicsCommandList* commandList,
	const VertexData* vertices,
	uint32_t vertexCount,
	ID3D12Resource* materialBuffer,
	ID3D12Resource* transformBuffer)
{
	if (!commandList || !vertices || vertexCount == 0 || !materialBuffer || !transformBuffer) {
		return;
	}

	// 線分用のPSOを設定
	commandList->SetGraphicsRootSignature(directXCommon_->GetLineRootSignature());
	commandList->SetPipelineState(directXCommon_->GetLinePipelineState());

	// マテリアルを設定
	commandList->SetGraphicsRootConstantBufferView(0, materialBuffer->GetGPUVirtualAddress());

	// トランスフォームを設定
	commandList->SetGraphicsRootConstantBufferView(1, transformBuffer->GetGPUVirtualAddress());

	// プリミティブトポロジを線分に設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

	// 一時的な頂点バッファの作成と設定
	Microsoft::WRL::ComPtr<ID3D12Resource> tempVertexBuffer =
		CreateBufferResource(directXCommon_->GetDevice(), sizeof(VertexData) * vertexCount);

	// データを書き込み
	VertexData* vertexData = nullptr;
	tempVertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, vertices, sizeof(VertexData) * vertexCount);

	// 頂点バッファビューを設定
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	vertexBufferView.BufferLocation = tempVertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VertexData) * vertexCount;
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// 頂点バッファをバインド
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

	// 描画（インデックスバッファは使用しない）
	commandList->DrawInstanced(vertexCount, 1, 0, 0);

	// 3D用のPSOに戻す
	commandList->SetGraphicsRootSignature(directXCommon_->GetRootSignature());
	commandList->SetPipelineState(directXCommon_->GetPipelineState());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Line::UpdateVertexBuffer()
{
	if (!directXCommon_) {
		return;
	}

	// 頂点データを作成
	VertexData vertices[2];
	GetVertexData(vertices[0], vertices[1]);

	// 頂点バッファを作成/再作成
	vertexBuffer_ = CreateBufferResource(directXCommon_->GetDevice(), sizeof(VertexData) * 2);

	// データを書き込み
	VertexData* vertexData = nullptr;
	vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, vertices, sizeof(VertexData) * 2);

	// 頂点バッファビューを設定
	vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 2;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
}

void Line::UpdateMaterialBuffer()
{
	if (materialData_) {
		materialData_->color = color_;
	}
}