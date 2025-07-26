#include "OffscreenTriangle.h"
#include "BaseSystem/Logger/Logger.h"
#include <cassert>

void OffscreenTriangle::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;

	// 大きな三角形の頂点データを作成
	CreateFullscreenTriangle();

	// 頂点バッファを作成
	CreateVertexBuffer();

	Logger::Log(Logger::GetStream(), "OffscreenTriangle initialized successfully!\n");
}

void OffscreenTriangle::Finalize() {
	// 特に解放処理は不要（ComPtrが自動的に解放）
	dxCommon_ = nullptr;
	Logger::Log(Logger::GetStream(), "OffscreenTriangle finalized.\n");
}

void OffscreenTriangle::DrawWithCustomPSO(
	ID3D12RootSignature* rootSignature,
	ID3D12PipelineState* pipelineState,
	D3D12_GPU_DESCRIPTOR_HANDLE textureHandle,
	D3D12_GPU_VIRTUAL_ADDRESS materialBufferGPUAddress) {

	if (!IsValid()) {
		return;
	}

	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	// 外部で指定されたPSOを設定
	commandList->SetGraphicsRootSignature(rootSignature);
	commandList->SetPipelineState(pipelineState);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// パラメータ設定（ルートパラメータの順序に注意）
	int parameterIndex = 0;

	// マテリアルバッファが指定されていれば設定
	if (materialBufferGPUAddress != 0) {
		commandList->SetGraphicsRootConstantBufferView(parameterIndex++, materialBufferGPUAddress);
	}

	// テクスチャを設定
	commandList->SetGraphicsRootDescriptorTable(parameterIndex, textureHandle);

	// 頂点バッファをバインド
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);

	// 大きな三角形を描画（3頂点）
	commandList->DrawInstanced(3, 1, 0, 0);
}

void OffscreenTriangle::DrawWithCustomPSOAndDepth(
	ID3D12RootSignature* rootSignature,
	ID3D12PipelineState* pipelineState,
	D3D12_GPU_DESCRIPTOR_HANDLE colorTextureHandle,
	D3D12_GPU_DESCRIPTOR_HANDLE depthTextureHandle,
	D3D12_GPU_VIRTUAL_ADDRESS materialBufferGPUAddress) {

	if (!IsValid()) {
		return;
	}

	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	// 外部で指定されたPSOを設定
	commandList->SetGraphicsRootSignature(rootSignature);
	commandList->SetPipelineState(pipelineState);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// マテリアルバッファが指定されていれば設定
	if (materialBufferGPUAddress != 0) {
		commandList->SetGraphicsRootConstantBufferView(0, materialBufferGPUAddress);
	}

	// カラーテクスチャを設定（RootParameter 1番目）
	commandList->SetGraphicsRootDescriptorTable(1, colorTextureHandle);

	// 深度テクスチャを設定（RootParameter 2番目）
	commandList->SetGraphicsRootDescriptorTable(2, depthTextureHandle);

	// 頂点バッファをバインド
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);

	// 大きな三角形を描画（3頂点）
	commandList->DrawInstanced(3, 1, 0, 0);
}

void OffscreenTriangle::Draw(D3D12_GPU_DESCRIPTOR_HANDLE textureHandle) {
	if (!IsValid()) {
		return;
	}

	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	// デフォルトのオフスクリーンPSOを使用
	// TODO：この部分は後でOffscreenRendererから適切なPSOを取得するように修正予定

	// 頂点バッファをバインド
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);

	// テクスチャを設定
	commandList->SetGraphicsRootDescriptorTable(2, textureHandle);

	// 大きな三角形を描画（3頂点）
	commandList->DrawInstanced(3, 1, 0, 0);
}

void OffscreenTriangle::CreateFullscreenTriangle() {
	vertices_.resize(3);

	// 時計回りに変更
	vertices_[0].position = { -1.0f, -1.0f, 0.0f, 1.0f };  // 左下
	vertices_[0].texcoord = { 0.0f, 1.0f };

	vertices_[1].position = { -1.0f, 3.0f, 0.0f, 1.0f };   // 左上 
	vertices_[1].texcoord = { 0.0f, -1.0f };

	vertices_[2].position = { 3.0f, -1.0f, 0.0f, 1.0f };   // 右下
	vertices_[2].texcoord = { 2.0f, 1.0f };
}

void OffscreenTriangle::CreateVertexBuffer() {
	// 頂点バッファを作成
	vertexBuffer_ = CreateBufferResource(dxCommon_->GetDeviceComPtr(), sizeof(FullscreenVertex) * vertices_.size());

	// 頂点データをマップしてコピー
	FullscreenVertex* vertexData = nullptr;
	vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, vertices_.data(), sizeof(FullscreenVertex) * vertices_.size());
	vertexBuffer_->Unmap(0, nullptr);

	// 頂点バッファビューを設定
	vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(FullscreenVertex) * vertices_.size());
	vertexBufferView_.StrideInBytes = sizeof(FullscreenVertex);

	Logger::Log(Logger::GetStream(), "Created fullscreen triangle vertex buffer.\n");
}