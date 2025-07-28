#include "GameObject.h"
#include "Managers/ImGui/ImGuiManager.h"

void GameObject::Initialize(DirectXCommon* dxCommon, const std::string& modelTag, const std::string& textureName) {
	directXCommon_ = dxCommon;
	modelTag_ = modelTag;
	textureName_ = textureName;  // プリミティブ用のテクスチャ名を設定

	// 共有モデルを取得
	sharedModel_ = modelManager_->GetModel(modelTag);
	if (sharedModel_ == nullptr) {
		Logger::Log(Logger::GetStream(), std::format("Model '{}' not found! Call ModelManager::LoadModel first.\n", modelTag));
		assert(false && "Model not preloaded! Call ModelManager::LoadModel first.");
		return;
	}

	// 個別のマテリアルを初期化（既存のMaterialクラスを使用）
	material_.Initialize(dxCommon);

	// 個別のトランスフォームを初期化（既存のTransformクラスを使用）
	transform_.Initialize(dxCommon);

	// デフォルトのトランスフォーム設定
	Vector3Transform defaultTransform{
		{1.0f, 1.0f, 1.0f},  // scale
		{0.0f, 0.0f, 0.0f},  // rotate
		{0.0f, 0.0f, 0.0f}   // translate
	};
	transform_.SetTransform(defaultTransform);

	// ImGui用の初期値を設定
	imguiPosition_ = transform_.GetPosition();
	imguiRotation_ = transform_.GetRotation();
	imguiScale_ = transform_.GetScale();
	imguiColor_ = material_.GetColor();
}

void GameObject::Update(const Matrix4x4& viewProjectionMatrix) {
	// アクティブでない場合は更新を止める
	if (!isActive_) {
		return;
	}

	// トランスフォーム行列の更新（既存のTransformクラスを使用）
	transform_.UpdateMatrix(viewProjectionMatrix);

	// UVトランスフォームの更新（既存のMaterialクラスを使用）
	material_.UpdateUVTransform();
}
void GameObject::Draw(const Light& directionalLight) {
	// 非表示、アクティブでない場合、または共有モデルがない場合は描画しない
	if (!isVisible_ || !isActive_ || !sharedModel_ || !sharedModel_->IsValid()) {
		return;
	}

	// 描画処理
	ID3D12GraphicsCommandList* commandList = directXCommon_->GetCommandList();

	// ライトを設定
	commandList->SetGraphicsRootConstantBufferView(3, directionalLight.GetResource()->GetGPUVirtualAddress());

	// 全メッシュを描画（マルチマテリアル対応）
	const auto& meshes = sharedModel_->GetMeshes();
	for (size_t i = 0; i < meshes.size(); ++i) {
		const Mesh& mesh = meshes[i];

		// このメッシュが使用するマテリアルインデックスを取得
		size_t materialIndex = sharedModel_->GetMeshMaterialIndex(i);

		// マテリアル設定の改善：常にGameObjectのマテリアル設定を優先
		if (!textureName_.empty() || sharedModel_->GetMeshCount() == 1) {
			// カスタムテクスチャが設定されている場合、または単一メッシュの場合は個別のマテリアルを使用
			commandList->SetGraphicsRootConstantBufferView(0, material_.GetResource()->GetGPUVirtualAddress());
		} else if (materialIndex < sharedModel_->GetMaterialCount()) {
			// マルチメッシュの場合：GameObjectのマテリアル設定をモデルのマテリアルに同期
			SyncMaterialSettings(materialIndex);
			commandList->SetGraphicsRootConstantBufferView(0,
				sharedModel_->GetMaterial(materialIndex).GetResource()->GetGPUVirtualAddress());
		} else {
			// フォールバック：個別のマテリアルを使用
			commandList->SetGraphicsRootConstantBufferView(0, material_.GetResource()->GetGPUVirtualAddress());
		}

		// トランスフォームを設定（個別のトランスフォームを使用）
		commandList->SetGraphicsRootConstantBufferView(1, transform_.GetResource()->GetGPUVirtualAddress());

		// テクスチャの設定（優先順位：カスタムテクスチャ > モデル付属テクスチャ）
		if (!textureName_.empty()) {
			// プリミティブ等でカスタムテクスチャが設定されている場合
			commandList->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureHandle(textureName_));
		} else if (sharedModel_->HasTexture(materialIndex)) {
			// OBJファイル等でモデル付属のテクスチャがある場合（マテリアルごと）
			commandList->SetGraphicsRootDescriptorTable(2,
				textureManager_->GetTextureHandle(sharedModel_->GetTextureTagName(materialIndex)));
		}

		// メッシュをバインドして描画
		const_cast<Mesh&>(mesh).Bind(commandList);
		const_cast<Mesh&>(mesh).Draw(commandList);
	}
}

// 新しく追加するメソッド
void GameObject::SyncMaterialSettings(size_t materialIndex) {
	if (materialIndex < sharedModel_->GetMaterialCount()) {
		Material& modelMaterial = sharedModel_->GetMaterial(materialIndex);

		// GameObjectのマテリアル設定をモデルのマテリアルに同期
		modelMaterial.SetColor(material_.GetColor());
		modelMaterial.SetLightingMode(material_.GetLightingMode());

		// UV変換も同期
		modelMaterial.SetUVTransformScale(material_.GetUVTransformScale());
		modelMaterial.SetUVTransformRotateZ(material_.GetUVTransformRotateZ());
		modelMaterial.SetUVTransformTranslate(material_.GetUVTransformTranslate());
	}
}
void GameObject::ImGui() {
#ifdef _DEBUG
	// 現在の名前を表示
	if (ImGui::TreeNode(name_.c_str())) {
		// 表示・アクティブ状態
		ImGui::Checkbox("Visible", &isVisible_);
		ImGui::Checkbox("Active", &isActive_);

		// モデル情報
		ImGui::Text("Model Tag: %s", modelTag_.c_str());
		if (sharedModel_) {
			ImGui::Text("Shared Model Loaded: Yes");
			ImGui::Text("Model Path: %s", sharedModel_->GetFilePath().c_str());
			ImGui::Text("Mesh Count: %zu", sharedModel_->GetMeshCount());
			ImGui::Text("Material Count: %zu", sharedModel_->GetMaterialCount());
			
			// マルチテクスチャ情報表示
			const auto& textureTagNames = sharedModel_->GetTextureTagNames();
			for (size_t i = 0; i < textureTagNames.size(); ++i) {
				ImGui::Text("Texture %zu: %s", i, textureTagNames[i].empty() ? "none" : textureTagNames[i].c_str());
			}
		} else {
			ImGui::Text("Shared Model Loaded: No");
		}

		// Transform（既存のTransformクラスのデータを表示・操作）
		if (ImGui::CollapsingHeader("Transform")) {
			// ImGui用の値を現在の値で更新
			imguiPosition_ = transform_.GetPosition();
			imguiRotation_ = transform_.GetRotation();
			imguiScale_ = transform_.GetScale();

			if (ImGui::DragFloat3("Position", &imguiPosition_.x, 0.01f)) {
				transform_.SetPosition(imguiPosition_);
			}
			if (ImGui::DragFloat3("Rotation", &imguiRotation_.x, 0.01f)) {
				transform_.SetRotation(imguiRotation_);
			}
			if (ImGui::DragFloat3("Scale", &imguiScale_.x, 0.01f)) {
				transform_.SetScale(imguiScale_);
			}
		}

		// Material（既存のMaterialクラスのデータを表示・操作）
		if (ImGui::CollapsingHeader("Material")) {

			// UVトランスフォーム
			imguiUvPosition_ = material_.GetUVTransformTranslate();
			imguiUvScale_ = material_.GetUVTransformScale();
			imguiUvRotateZ_ = material_.GetUVTransformRotateZ();

			if (ImGui::DragFloat2("UVtranslate", &imguiUvPosition_.x, 0.01f, -10.0f, 10.0f)) {
				material_.SetUVTransformTranslate(imguiUvPosition_);
			}
			if (ImGui::DragFloat2("UVscale", &imguiUvScale_.x, 0.01f, -10.0f, 10.0f)) {
				material_.SetUVTransformScale(imguiUvScale_);
			}
			if (ImGui::SliderAngle("UVrotate", &imguiUvRotateZ_)) {
				material_.SetUVTransformRotateZ(imguiUvRotateZ_);
			}

			// ImGui用の値を現在の値で更新
			imguiColor_ = material_.GetColor();
			if (ImGui::ColorEdit4("Color", reinterpret_cast<float*>(&imguiColor_.x))) {
				material_.SetColor(imguiColor_);
			}

			// ライティング選択（ComboBox形式）
			// ComboBox用の選択肢配列
			const char* lightingModeNames[] = { "None", "Lambert", "Half-Lambert" };

			// 現在のモードをインデックスに変換
			LightingMode currentMode = material_.GetLightingMode();
			int currentModeIndex = static_cast<int>(currentMode);

			// ComboBoxの表示と選択処理
			if (ImGui::Combo("Lighting", &currentModeIndex, lightingModeNames, IM_ARRAYSIZE(lightingModeNames))) {
				// 選択が変わった場合、新しいモードを設定
				material_.SetLightingMode(static_cast<LightingMode>(currentModeIndex));
			}
		}

		// メッシュ情報（複数メッシュ対応）
		if (ImGui::CollapsingHeader("Mesh Info") && sharedModel_) {
			ImGui::Text("Total Meshes: %zu", sharedModel_->GetMeshCount());
			ImGui::Text("Total Materials: %zu", sharedModel_->GetMaterialCount());
			ImGui::Text("Shared: Yes (Memory Optimized)");

			// 各メッシュの詳細情報
			const auto& meshes = sharedModel_->GetMeshes();
			const auto& objectNames = sharedModel_->GetObjectNames();

			for (size_t i = 0; i < meshes.size(); ++i) {
				const Mesh& mesh = meshes[i];
				std::string meshName = std::format("Mesh {} ({})", i,
					i < objectNames.size() ? objectNames[i] : "Unknown");

				if (ImGui::TreeNode(meshName.c_str())) {
					ImGui::Text("Mesh Type: %s", Mesh::MeshTypeToString(mesh.GetMeshType()).c_str());
					ImGui::Text("Vertex Count: %d", mesh.GetVertexCount());
					ImGui::Text("Index Count: %d", mesh.GetIndexCount());
					
					// マテリアル情報
					size_t materialIndex = sharedModel_->GetMeshMaterialIndex(i);
					ImGui::Text("Material Index: %zu", materialIndex);
					if (sharedModel_->HasTexture(materialIndex)) {
						ImGui::Text("Texture: %s", sharedModel_->GetTextureTagName(materialIndex).c_str());
					} else {
						ImGui::Text("Texture: none");
					}
					
					ImGui::TreePop();
				}
			}
		}

		// テクスチャ設定（シンプル版）
		if (ImGui::CollapsingHeader("Texture")) {
			// モデルのテクスチャ状態表示
			if (sharedModel_ && sharedModel_->GetMaterialCount() > 0) {
				ImGui::Text("Model Textures:");
				const auto& textureTagNames = sharedModel_->GetTextureTagNames();
				for (size_t i = 0; i < textureTagNames.size(); ++i) {
					ImGui::Text("  Material %zu: %s", i, textureTagNames[i].empty() ? "none" : textureTagNames[i].c_str());
				}
			}

			// カスタムテクスチャ選択
			std::vector<std::string> textureList = textureManager_->GetTextureTagList();
			if (!textureList.empty()) {
				// const char*配列を作成（正しい方法）
				std::vector<const char*> textureNames;
				textureNames.push_back("Default");

				for (const auto& texture : textureList) {
					textureNames.push_back(texture.c_str());
				}

				// 現在の選択インデックス
				int currentIndex = 0;
				if (!textureName_.empty()) {
					for (size_t i = 0; i < textureList.size(); ++i) {
						if (textureList[i] == textureName_) {
							currentIndex = static_cast<int>(i + 1);
							break;
						}
					}
				}

				// ComboBox表示
				if (ImGui::Combo("Custom Texture", &currentIndex, textureNames.data(), static_cast<int>(textureNames.size()))) {
					if (currentIndex == 0) {
						SetTexture(""); // Default選択時
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