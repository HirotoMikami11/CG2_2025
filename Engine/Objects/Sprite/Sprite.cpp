#include "Objects/Sprite/Sprite.h"
#include <cassert>
#include <cstring>
#include "Managers/ImGui/ImGuiManager.h" 

void Sprite::Initialize(DirectXCommon* dxCommon, const std::string& textureName, const Vector2& center, const Vector2& size, const Vector2& anchor)
{
	directXCommon_ = dxCommon;
	textureName_ = textureName;
	anchor_ = anchor;

	//ID生成
	ObjectIDManager* idManager = ObjectIDManager::GetInstance();
	name_ = idManager->GenerateName("Sprite");

	// アンカーポイントを考慮したメッシュを作成
	CreateSpriteMesh();

	// Transform2Dクラスを初期化
	transform_.Initialize(dxCommon);

	// centerとsizeをTransform2Dに設定
	Vector2Transform initialTransform{
		size,			// scale = size
		0.0f,			// rotateZ
		center			// translate = center
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
	imguiAnchor_ = anchor_;
}

void Sprite::Initialize(DirectXCommon* dxCommon, const Vector2& center, const Vector2& size, const Vector2& anchor)
{
	directXCommon_ = dxCommon;
	textureName_ = "white";
	anchor_ = anchor;

	//ID生成
	ObjectIDManager* idManager = ObjectIDManager::GetInstance();
	name_ = idManager->GenerateName("Sprite");

	// アンカーポイントを考慮したメッシュを作成
	CreateSpriteMesh();

	// Transform2Dクラスを初期化
	transform_.Initialize(dxCommon);

	// centerとsizeをTransform2Dに設定
	Vector2Transform initialTransform{
		size,			// scale = size
		0.0f,			// rotateZ
		center			// translate = center
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
	imguiAnchor_ = anchor_;
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

	// 通常のUI用スプライト描画処理
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

void Sprite::ImGui()
{
#ifdef _DEBUG
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

			// アンカーポイント設定
			imguiAnchor_ = anchor_;
			if (ImGui::DragFloat2("Anchor", &imguiAnchor_.x, 0.01f, 0.0f, 1.0f)) {
				SetAnchor(imguiAnchor_);
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

		// テクスチャ設定
		if (ImGui::CollapsingHeader("Texture")) {
			ImGui::Text("Current Texture: %s", textureName_.c_str());

			// カスタムテクスチャ選択
			std::vector<std::string> textureList = textureManager_->GetTextureTagList();
			if (!textureList.empty()) {
				std::vector<const char*> textureNames;
				textureNames.push_back("Default");
				for (const auto& texture : textureList) {
					textureNames.push_back(texture.c_str());
				}
				int currentIndex = 0;
				if (!textureName_.empty()) {
					for (size_t i = 0; i < textureList.size(); ++i) {
						if (textureList[i] == textureName_) {
							currentIndex = static_cast<int>(i + 1);
							break;
						}
					}
				}
				if (ImGui::Combo("Custom Texture", &currentIndex, textureNames.data(), static_cast<int>(textureNames.size()))) {
					if (currentIndex == 0) {
						SetTexture("");
					} else {
						SetTexture(textureList[currentIndex - 1]);
					}
				}
			} else {
				ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No textures loaded");
			}
		}

		ImGui::TreePop();
	}
#endif
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

void Sprite::SetAnchor(const Vector2& anchor)
{
	anchor_ = anchor;
	// アンカーが変更されたらメッシュを再生成
	CreateSpriteMesh();

	// 頂点バッファを更新
	VertexData* vertexData = nullptr;
	vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, vertices_.data(), sizeof(VertexData) * vertices_.size());
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

void Sprite::CreateSpriteMesh()
{
	//アンカーを基準とした正方形
	vertices_.resize(4);

	// アンカーに基づいて頂点座標を計算
	float left = -anchor_.x;
	float right = 1.0f - anchor_.x;
	float top = anchor_.y - 1.0f;  // 画面座標系に合わせて上下反転
	float bottom = anchor_.y;

	// 左下
	vertices_[0].position = { left, bottom, 0.0f, 1.0f };
	vertices_[0].texcoord = { 0.0f, 1.0f };
	vertices_[0].normal = { 0.0f, 0.0f, -1.0f };

	// 左上
	vertices_[1].position = { left, top, 0.0f, 1.0f };
	vertices_[1].texcoord = { 0.0f, 0.0f };
	vertices_[1].normal = { 0.0f, 0.0f, -1.0f };

	// 右下
	vertices_[2].position = { right, bottom, 0.0f, 1.0f };
	vertices_[2].texcoord = { 1.0f, 1.0f };
	vertices_[2].normal = { 0.0f, 0.0f, -1.0f };

	// 右上
	vertices_[3].position = { right, top, 0.0f, 1.0f };
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
	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };	// 白色
	UpdateUVTransform();								// UVTransformを初期化
}

void Sprite::UpdateUVTransform()
{
	if (!materialData_) return;

	Matrix4x4 uvTransformMatrix = MakeScaleMatrix({ uvScale_.x, uvScale_.y, 1.0f });
	uvTransformMatrix = Matrix4x4Multiply(uvTransformMatrix, MakeRotateZMatrix(uvRotateZ_));
	uvTransformMatrix = Matrix4x4Multiply(uvTransformMatrix, MakeTranslateMatrix({ uvTranslate_.x, uvTranslate_.y, 0.0f }));

	materialData_->uvTransform = uvTransformMatrix;
}