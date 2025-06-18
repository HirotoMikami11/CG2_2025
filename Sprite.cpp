#include "Sprite.h"
#include <cassert>
#include <cstring>
#include"ImGuiManager.h"

// 静的変数の定義
int Sprite::spriteCount_ = 0;

void Sprite::Initialize(DirectXCommon* dxCommon, const std::string& textureName, const Vector2& center, const Vector2& size)
{
	directXCommon_ = dxCommon;
	textureName_ = textureName;
	name_ = SettingName("Sprite");

	// 標準メッシュを作成（原点中心、サイズ1.0x1.0）
	CreateStandardSpriteMesh();

	// Transform2Dクラスを初期化
	transform_.Initialize(dxCommon);

	// centerとsizeをTransform2Dに設定
	Vector2Transform initialTransform{
		size,         // scale = size
		0.0f,         // rotateZ
		center        // translate = center
	};
	transform_.SetTransform(initialTransform);

	// スプライト専用のマテリアルリソースを作成
	CreateBuffers();

	// ImGui用の初期値を設定（Transform2Dから取得）
	imguiPosition_ = transform_.GetPosition();
	imguiRotation_ = transform_.GetRotation();
	imguiScale_ = transform_.GetScale();
	imguiColor_ = materialData_->color;
	imguiUvPosition_ = uvTranslate_;
	imguiUvScale_ = uvScale_;
	imguiUvRotateZ_ = uvRotateZ_;
}

void Sprite::Update(const Matrix4x4& viewProjectionMatrix)
{
	// アクティブでない場合は更新を止める
	if (!isActive_) {
		return;
	}

	// Transform2Dクラスを使用して行列更新
	transform_.UpdateMatrix(viewProjectionMatrix);
}

void Sprite::Draw()
{
	// 非表示、アクティブでない場合は描画しない
	if (!isVisible_ || !isActive_) {
		return;
	}

	// 通常のUI用スプライト描画処理（変更なし）
	ID3D12GraphicsCommandList* commandList = directXCommon_->GetCommandList();

	// スプライト専用のPSOを設定
	commandList->SetGraphicsRootSignature(directXCommon_->GetSpriteRootSignature());
	commandList->SetPipelineState(directXCommon_->GetSpritePipelineState());

	// プリミティブトポロジを設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//マテリアル
	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	//トランスフォーム（Transform2Dを使用）
	commandList->SetGraphicsRootConstantBufferView(1, transform_.GetResource()->GetGPUVirtualAddress());
	// テクスチャをバインド
	if (!textureName_.empty()) {
		commandList->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureHandle(textureName_));
	}

	// 頂点バッファをバインド
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList->IASetIndexBuffer(&indexBufferView_);

	// 描画
	commandList->DrawIndexedInstanced(static_cast<UINT>(indices_.size()), 1, 0, 0, 0);

	// 3Dの描画設定に戻す
	commandList->SetGraphicsRootSignature(directXCommon_->GetRootSignature());
	commandList->SetPipelineState(directXCommon_->GetPipelineState());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Sprite::DrawWithCustomPSO(
	ID3D12RootSignature* rootSignature,
	ID3D12PipelineState* pipelineState,
	D3D12_GPU_DESCRIPTOR_HANDLE textureHandle,
	D3D12_GPU_VIRTUAL_ADDRESS materialBufferGPUAddress)
{
	// 非表示、アクティブでない場合は描画しない
	if (!isVisible_ || !isActive_) {
		return;
	}

	ID3D12GraphicsCommandList* commandList = directXCommon_->GetCommandList();

	// 外部で指定されたPSOを設定
	commandList->SetGraphicsRootSignature(rootSignature);
	commandList->SetPipelineState(pipelineState);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// マテリアル（外部指定があればそれを使用、なければ内部のものを使用）
	D3D12_GPU_VIRTUAL_ADDRESS materialAddress = materialBufferGPUAddress != 0 ?
		materialBufferGPUAddress : materialResource_->GetGPUVirtualAddress();
	commandList->SetGraphicsRootConstantBufferView(0, materialAddress);

	// トランスフォーム（Transform2Dを使用）
	commandList->SetGraphicsRootConstantBufferView(1, transform_.GetResource()->GetGPUVirtualAddress());

	// 外部指定のテクスチャを使用
	commandList->SetGraphicsRootDescriptorTable(2, textureHandle);

	// 頂点バッファをバインド
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList->IASetIndexBuffer(&indexBufferView_);

	// 描画
	commandList->DrawIndexedInstanced(static_cast<UINT>(indices_.size()), 1, 0, 0, 0);

	// ※PSO復元は呼び出し元で行う
}

void Sprite::ImGui()
{
	if (ImGui::TreeNode(name_.c_str())) {
		// 表示・アクティブ状態
		ImGui::Checkbox("Visible", &isVisible_);
		ImGui::Checkbox("Active", &isActive_);

		// Transform（Transform2Dクラスを使用、2D用UIに最適化）
		if (ImGui::CollapsingHeader("Transform")) {
			imguiPosition_ = transform_.GetPosition();
			imguiRotation_ = transform_.GetRotation();
			imguiScale_ = transform_.GetScale();

			// 2D座標用（XYのみ）
			if (ImGui::DragFloat2("Position", &imguiPosition_.x, 1.0f)) {
				transform_.SetPosition(imguiPosition_);
			}

			// Z軸回転のみ
			if (ImGui::SliderAngle("Rotation", &imguiRotation_)) {
				transform_.SetRotation(imguiRotation_);
			}

			// 2Dサイズ用（XYのみ）- スケールとして管理
			if (ImGui::DragFloat2("Size", &imguiScale_.x, 1.0f, 0.1f, 1000.0f)) {
				transform_.SetScale(imguiScale_);
			}

			// 3D互換表示用（参考情報として表示）
			ImGui::Separator();
			ImGui::Text("3D Compatibility Info:");
			Vector3 pos3D = transform_.GetPosition3D();
			Vector3 rot3D = transform_.GetRotation3D();
			Vector3 scale3D = transform_.GetScale3D();
			ImGui::Text("Position3D: (%.2f, %.2f, %.2f)", pos3D.x, pos3D.y, pos3D.z);
			ImGui::Text("Rotation3D: (%.2f, %.2f, %.2f)", rot3D.x, rot3D.y, rot3D.z);
			ImGui::Text("Scale3D: (%.2f, %.2f, %.2f)", scale3D.x, scale3D.y, scale3D.z);
		}

		// Color & UVTransform（SpriteMaterial構造体）
		if (ImGui::CollapsingHeader("Material")) {
			imguiColor_ = materialData_->color;

			if (ImGui::ColorEdit4("Color", reinterpret_cast<float*>(&imguiColor_.x))) {
				SetColor(imguiColor_);
			}

			// UVTransform
			ImGui::Text("UVTransform");
			imguiUvPosition_ = uvTranslate_;
			imguiUvScale_ = uvScale_;
			imguiUvRotateZ_ = uvRotateZ_;

			if (ImGui::DragFloat2("UVtranslate", &imguiUvPosition_.x, 0.01f, -10.0f, 10.0f)) {
				SetUVTransformTranslate(imguiUvPosition_);
			}
			if (ImGui::DragFloat2("UVscale", &imguiUvScale_.x, 0.01f, -10.0f, 10.0f)) {
				SetUVTransformScale(imguiUvScale_);
			}
			if (ImGui::SliderAngle("UVrotate", &imguiUvRotateZ_)) {
				SetUVTransformRotateZ(imguiUvRotateZ_);
			}
		}

		// Texture
		if (ImGui::CollapsingHeader("Texture")) {
			ImGui::Text("Current Texture: %s", textureName_.c_str());

			// uvCheckerボタン
			if (textureName_ == "uvChecker") {
				ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
				if (ImGui::Button("uvChecker")) {
					SetTexture("uvChecker");
				}
				ImGui::PopStyleColor();
			} else {
				if (ImGui::Button("uvChecker")) {
					SetTexture("uvChecker");
				}
			}

			ImGui::SameLine();

			// monsterBallボタン
			if (textureName_ == "monsterBall") {
				ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
				if (ImGui::Button("monsterBall")) {
					SetTexture("monsterBall");
				}
				ImGui::PopStyleColor();
			} else {
				if (ImGui::Button("monsterBall")) {
					SetTexture("monsterBall");
				}
			}
		}

		ImGui::TreePop();
	}
}

// サイズ管理用の新しいメソッド
Vector2 Sprite::GetSize() const
{
	return transform_.GetScale();
}

void Sprite::SetSize(const Vector2& size)
{
	transform_.SetScale(size);
}

void Sprite::AddSize(const Vector2& deltaSize)
{
	transform_.AddScale(deltaSize);
}

void Sprite::SetColor(const Vector4& color)
{
	if (materialData_) {
		materialData_->color = color;
	}
}

void Sprite::SetUVTransformScale(const Vector2& uvScale)
{
	uvScale_ = uvScale;
	UpdateUVTransform();
}

void Sprite::SetUVTransformRotateZ(float uvRotateZ)
{
	uvRotateZ_ = uvRotateZ;
	UpdateUVTransform();
}

void Sprite::SetUVTransformTranslate(const Vector2& uvTranslate)
{
	uvTranslate_ = uvTranslate;
	UpdateUVTransform();
}

void Sprite::CreateStandardSpriteMesh()
{
	// 標準メッシュ：原点中心、サイズ1.0x1.0の正方形
	vertices_.resize(4);

	// 左下
	vertices_[0].position = { -0.5f, 0.5f, 0.0f, 1.0f };
	vertices_[0].texcoord = { 0.0f, 1.0f };
	vertices_[0].normal = { 0.0f, 0.0f, -1.0f };

	// 左上
	vertices_[1].position = { -0.5f, -0.5f, 0.0f, 1.0f };
	vertices_[1].texcoord = { 0.0f, 0.0f };
	vertices_[1].normal = { 0.0f, 0.0f, -1.0f };

	// 右下
	vertices_[2].position = { 0.5f, 0.5f, 0.0f, 1.0f };
	vertices_[2].texcoord = { 1.0f, 1.0f };
	vertices_[2].normal = { 0.0f, 0.0f, -1.0f };

	// 右上
	vertices_[3].position = { 0.5f, -0.5f, 0.0f, 1.0f };
	vertices_[3].texcoord = { 1.0f, 0.0f };
	vertices_[3].normal = { 0.0f, 0.0f, -1.0f };

	// インデックスデータ（2つの三角形）
	indices_ = { 0, 1, 2, 1, 3, 2 };
}

void Sprite::CreateBuffers()
{
	// 頂点バッファを作成
	vertexBuffer_ = CreateBufferResource(directXCommon_->GetDevice(), sizeof(VertexData) * vertices_.size());
	VertexData* vertexData = nullptr;
	vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, vertices_.data(), sizeof(VertexData) * vertices_.size());

	// 頂点バッファビューを設定
	vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * vertices_.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// インデックスバッファを作成
	indexBuffer_ = CreateBufferResource(directXCommon_->GetDevice(), sizeof(uint32_t) * indices_.size());
	uint32_t* indexData = nullptr;
	indexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	std::memcpy(indexData, indices_.data(), sizeof(uint32_t) * indices_.size());

	// インデックスバッファビューを設定
	indexBufferView_.BufferLocation = indexBuffer_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(uint32_t) * indices_.size());
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	// SpriteMaterialリソースを作成
	materialResource_ = CreateBufferResource(directXCommon_->GetDevice(), sizeof(SpriteMaterial));
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

	// SpriteMaterial初期化
	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };  // 白色
	UpdateUVTransform();  // UVTransformを初期化
}

void Sprite::UpdateUVTransform()
{
	if (!materialData_) return;

	Matrix4x4 uvTransformMatrix = MakeScaleMatrix({ uvScale_.x, uvScale_.y, 1.0f });
	uvTransformMatrix = Matrix4x4Multiply(uvTransformMatrix, MakeRotateZMatrix(uvRotateZ_));
	uvTransformMatrix = Matrix4x4Multiply(uvTransformMatrix, MakeTranslateMatrix({ uvTranslate_.x, uvTranslate_.y, 0.0f }));

	materialData_->uvTransform = uvTransformMatrix;
}