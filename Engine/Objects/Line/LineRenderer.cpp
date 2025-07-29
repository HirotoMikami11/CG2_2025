#include "LineRenderer.h"
#include "Managers/ImGui/ImGuiManager.h"
#include "BaseSystem/Logger/Logger.h"
#include <algorithm>

void LineRenderer::Initialize(DirectXCommon* dxCommon) {
	directXCommon_ = dxCommon;

	// 最大線分数分の頂点バッファを作成（線分1本につき2頂点）
	const size_t totalVertexCount = kMaxLineCount * kVertexCountPerLine;
	const size_t vertexBufferSize = sizeof(LineVertex) * totalVertexCount;

	// 頂点バッファ作成
	vertexBuffer_ = CreateBufferResource(directXCommon_->GetDevice(), vertexBufferSize);

	// 頂点バッファをマップ
	vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

	// 頂点バッファビューを設定
	vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = static_cast<UINT>(vertexBufferSize);
	vertexBufferView_.StrideInBytes = sizeof(LineVertex);

	// トランスフォームバッファ作成
	transformBuffer_ = CreateBufferResource(directXCommon_->GetDevice(), sizeof(TransformationMatrix));
	transformBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&transformData_));

	// 線分データの初期化
	lineData_.reserve(kMaxLineCount);

	isInitialized_ = true;
	Logger::Log(Logger::GetStream(), "LineRenderer: Initialized with max lines !!\n");
}

void LineRenderer::AddLine(const Vector3& start, const Vector3& end, const Vector4& color) {
	if (!isInitialized_) {
		Logger::Log(Logger::GetStream(), "LineRenderer: Not initialized!\n");
		return;
	}

	// 最大数チェック
	if (IsFull()) {
		Logger::Log(Logger::GetStream(), "LineRenderer: Maximum line reached!\n");
		return;
	}

	// 線分データを追加
	LineData lineData;
	lineData.start = start;
	lineData.end = end;
	lineData.color = color;
	lineData_.push_back(lineData);

	// 頂点バッファの更新が必要
	needsVertexUpdate_ = true;
}

void LineRenderer::Reset() {
	lineData_.clear();
	needsVertexUpdate_ = true;
}

void LineRenderer::Draw(const Matrix4x4& viewProjectionMatrix) {
	if (!isInitialized_ || !directXCommon_ || !isVisible_ || IsEmpty()) {
		return;
	}

	// 頂点バッファ更新
	if (needsVertexUpdate_) {
		UpdateVertexBuffer();
		needsVertexUpdate_ = false;
	}

	// トランスフォーム更新
	if (transformData_) {
		transformData_->WVP = viewProjectionMatrix;
		transformData_->World = MakeIdentity4x4();
	}

	ID3D12GraphicsCommandList* commandList = directXCommon_->GetCommandList();

	// 線分用のPSOを設定
	commandList->SetGraphicsRootSignature(directXCommon_->GetLineRootSignature());
	commandList->SetPipelineState(directXCommon_->GetLinePipelineState());

	// プリミティブトポロジを線分に設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

	// 頂点バッファをバインド
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);

	// トランスフォーム設定（RootParameter[0]: VertexShader用）
	commandList->SetGraphicsRootConstantBufferView(0, transformBuffer_->GetGPUVirtualAddress());

	// 一括描画（線分数 * 2頂点）
	const uint32_t vertexCount = GetLineCount() * kVertexCountPerLine;
	commandList->DrawInstanced(vertexCount, 1, 0, 0);

	// 3D用のPSOに戻す
	commandList->SetGraphicsRootSignature(directXCommon_->GetRootSignature());
	commandList->SetPipelineState(directXCommon_->GetPipelineState());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void LineRenderer::UpdateVertexBuffer() {
	if (!vertexData_ || lineData_.empty()) {
		return;
	}

	// 線分データを頂点データに変換
	for (size_t i = 0; i < lineData_.size(); ++i) {
		const LineData& line = lineData_[i];

		// 開始点の頂点
		size_t vertexIndex = i * kVertexCountPerLine;
		vertexData_[vertexIndex].position = { line.start.x, line.start.y, line.start.z, 1.0f };
		vertexData_[vertexIndex].color = line.color;
		vertexData_[vertexIndex].texcoord = { 0.0f, 0.0f };  // 未使用
		vertexData_[vertexIndex].normal = { 0.0f, 1.0f, 0.0f };  // 未使用

		// 終了点の頂点
		vertexData_[vertexIndex + 1].position = { line.end.x, line.end.y, line.end.z, 1.0f };
		vertexData_[vertexIndex + 1].color = line.color;
		vertexData_[vertexIndex + 1].texcoord = { 1.0f, 1.0f };  // 未使用
		vertexData_[vertexIndex + 1].normal = { 0.0f, 1.0f, 0.0f };  // 未使用
	}
}

void LineRenderer::ImGui() {
#ifdef _DEBUG
	if (ImGui::TreeNode("Line Renderer")) {
		// 基本情報
		ImGui::Checkbox("Visible", &isVisible_);
		ImGui::Text("Line Count: %d / %d", GetLineCount(), kMaxLineCount);

		// 使用率表示
		float usage = static_cast<float>(GetLineCount()) / static_cast<float>(kMaxLineCount);
		ImGui::ProgressBar(usage, ImVec2(0.0f, 0.0f),
			std::format("{:.1f}%", usage * 100.0f).c_str());

		// 状態表示
		if (IsEmpty()) {
			ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Status: Empty");
		} else if (IsFull()) {
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Status: Full");
		} else {
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Status: Normal");
		}

		// 操作ボタン
		if (ImGui::Button("Clear All Lines")) {
			Reset();
		}

		ImGui::TreePop();
	}
#endif
}