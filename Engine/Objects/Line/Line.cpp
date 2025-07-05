#include "Line.h"
#include "Managers/Texture/TextureManager.h"

// 静的変数の定義
DirectXCommon* Line::directXCommon_ = nullptr;

void Line::InitializeStatic(DirectXCommon* dxCommon)
{
	directXCommon_ = dxCommon;
}

void Line::Initialize() {}

void Line::SetPoints(const Vector3& start, const Vector3& end)
{
	start_ = start;
	end_ = end;
}

void Line::SetColor(const Vector4& color)
{
	color_ = color;
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

	// 白テクスチャを設定（線分にはテクスチャは不要だが、シェーダー互換性のため）
	TextureManager* textureManager = TextureManager::GetInstance();
	commandList->SetGraphicsRootDescriptorTable(2, textureManager->GetTextureHandle("white"));

	// ライト設定（シェーダー互換性のため、実際には使用されない）
	commandList->SetGraphicsRootConstantBufferView(3, materialBuffer->GetGPUVirtualAddress());

	// プリミティブトポロジを線分に設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

	// 一時的な頂点バッファの作成と設定
	// 警告：このバッファは関数終了時に削除される可能性があります
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