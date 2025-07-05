#define NOMINMAX // C+標準のstd::maxを使えるようにするため(windows.hが上書きしてしまっている)
#include "GridLine.h"
#include "Managers/ImGuiManager.h"
#include "Managers/Texture/TextureManager.h"
#include <algorithm>
#include <cmath>

void GridLine::Initialize(DirectXCommon* dxCommon)
{
	directXCommon_ = dxCommon;

	// Lineクラスを静的初期化（一度だけ）
	Line::InitializeStatic(dxCommon);

	// Transformを初期化
	transform_.Initialize(dxCommon);

	// LineMaterialバッファを作成（線分専用シェーダーと一致）
	CreateMaterialBuffer();

	// デフォルトでグリッドを作成
	CreateGrid();
}

void GridLine::CreateGrid(
	float size,
	float interval,
	float majorInterval,
	const Vector4& normalColor,
	const Vector4& majorColor)
{
	Clear();

	// 設定を保存
	gridSize_ = size;
	gridInterval_ = interval;
	gridMajorInterval_ = majorInterval;
	gridNormalColor_ = normalColor;
	gridMajorColor_ = majorColor;

	float halfSize = size * 0.5f;

	// X方向の線（Z軸に沿って）
	for (float x = -halfSize; x <= halfSize; x += interval) {
		Vector4 color = (std::fmod(std::abs(x), majorInterval) < 0.001f) ? majorColor : normalColor;

		Vector3 start = { x, 0.0f, -halfSize };
		Vector3 end = { x, 0.0f, halfSize };
		AddLine(start, end, color);
	}

	// Z方向の線（X軸に沿って）
	for (float z = -halfSize; z <= halfSize; z += interval) {
		Vector4 color = (std::fmod(std::abs(z), majorInterval) < 0.001f) ? majorColor : normalColor;

		Vector3 start = { -halfSize, 0.0f, z };
		Vector3 end = { halfSize, 0.0f, z };
		AddLine(start, end, color);
	}
}

void GridLine::AddLine(const Vector3& start, const Vector3& end, const Vector4& color)
{
	auto line = std::make_unique<Line>();
	line->Initialize();  // インスタンス初期化
	line->SetPoints(start, end);
	line->SetColor(color);
	lines_.push_back(std::move(line));
	needsBufferUpdate_ = true;
}

void GridLine::Clear()
{
	lines_.clear();
	vertices_.clear();
	needsBufferUpdate_ = true;
}

void GridLine::Update(const Matrix4x4& viewProjectionMatrix)
{
	if (!isActive_) {
		return;
	}

	// Transform行列を更新
	transform_.UpdateMatrix(viewProjectionMatrix);

	// バッファが更新必要な場合は更新
	if (needsBufferUpdate_) {
		UpdateBuffers();
		needsBufferUpdate_ = false;
	}
}

void GridLine::Draw()
{
	if (!isVisible_ || !isActive_ || lines_.empty() || vertices_.empty()) {
		return;
	}

	// 直接描画処理を行う
	ID3D12GraphicsCommandList* commandList = directXCommon_->GetCommandList();

	// 線分用のPSOを設定
	commandList->SetGraphicsRootSignature(directXCommon_->GetLineRootSignature());
	commandList->SetPipelineState(directXCommon_->GetLinePipelineState());

	// LineMaterialを設定（RootParameter[0]: PixelShader用）
	commandList->SetGraphicsRootConstantBufferView(0, materialBuffer_->GetGPUVirtualAddress());

	// TransformationMatrixを設定（RootParameter[1]: VertexShader用）
	commandList->SetGraphicsRootConstantBufferView(1, transform_.GetResource()->GetGPUVirtualAddress());

	// プリミティブトポロジを線分に設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

	// GridLineクラスが持つ頂点バッファを使用
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);

	// 描画（インデックスバッファは使用しない）
	commandList->DrawInstanced(static_cast<UINT>(vertices_.size()), 1, 0, 0);

	// 3D用のPSOに戻す
	commandList->SetGraphicsRootSignature(directXCommon_->GetRootSignature());
	commandList->SetPipelineState(directXCommon_->GetPipelineState());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void GridLine::ImGui()
{
#ifdef _DEBUG

#endif
}

void GridLine::UpdateBuffers()
{
	if (lines_.empty()) {
		return;
	}

	// 頂点データを作成（各線分につき2頂点）
	vertices_.clear();
	vertices_.reserve(lines_.size() * 2);

	for (const auto& line : lines_) {
		VertexData startVertex, endVertex;
		line->GetVertexData(startVertex, endVertex);

		vertices_.push_back(startVertex);
		vertices_.push_back(endVertex);
	}

	// 頂点バッファを作成
	CreateVertexBuffer();
}

void GridLine::CreateVertexBuffer()
{
	if (vertices_.empty()) {
		return;
	}

	// 頂点バッファを作成
	vertexBuffer_ = CreateBufferResource(directXCommon_->GetDevice(), sizeof(VertexData) * vertices_.size());

	// データを書き込み
	VertexData* vertexData = nullptr;
	vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, vertices_.data(), sizeof(VertexData) * vertices_.size());

	// 頂点バッファビューを設定
	vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * vertices_.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
}

void GridLine::CreateMaterialBuffer()
{
	// LineMaterialバッファを作成（16バイト）
	materialBuffer_ = CreateBufferResource(directXCommon_->GetDevice(), sizeof(LineMaterial));
	materialBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

	// デフォルト色を設定
	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
}