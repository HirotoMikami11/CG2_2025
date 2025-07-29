#include "GameObject.h"
#include "Managers/ImGui/ImGuiManager.h"

// 静的メンバの定義
Material GameObject::dummyMaterial_;

void GameObject::Initialize(DirectXCommon* dxCommon, const std::string& modelTag, const std::string& textureName) {
	directXCommon_ = dxCommon;
	modelTag_ = modelTag;
	textureName_ = textureName;

	// 共有モデルを取得
	sharedModel_ = modelManager_->GetModel(modelTag);
	if (sharedModel_ == nullptr) {
		Logger::Log(Logger::GetStream(), std::format("Model '{}' not found! Call ModelManager::LoadModel first.\n", modelTag));
		assert(false && "Model not preloaded! Call ModelManager::LoadModel first.");
		return;
	}

	// 個別のトランスフォームを初期化
	transform_.Initialize(dxCommon);

	// デフォルトのトランスフォーム設定
	Vector3Transform defaultTransform{
		{1.0f, 1.0f, 1.0f},  // scale
		{0.0f, 0.0f, 0.0f},  // rotate
		{0.0f, 0.0f, 0.0f}   // translate
	};
	transform_.SetTransform(defaultTransform);

	// ImGui用の状態を初期化
	imguiPosition_ = transform_.GetPosition();
	imguiRotation_ = transform_.GetRotation();
	imguiScale_ = transform_.GetScale();

	Logger::Log(Logger::GetStream(), std::format("GameObject initialized with model '{}' ({} materials)\n",
		modelTag, sharedModel_->GetMaterialCount()));
}

void GameObject::Update(const Matrix4x4& viewProjectionMatrix) {
	// アクティブでない場合は更新を止める
	if (!isActive_) {
		return;
	}

	// トランスフォーム行列の更新
	transform_.UpdateMatrix(viewProjectionMatrix);

	// モデルのマテリアル更新
	if (sharedModel_) {
		sharedModel_->UpdateMaterials();
	}
}

void GameObject::Draw(const Light& directionalLight) {
	// 非表示、アクティブでない場合、または共有モデルがない場合は描画しない
	if (!isVisible_ || !isActive_ || !sharedModel_ || !sharedModel_->IsValid()) {
		return;
	}

	ID3D12GraphicsCommandList* commandList = directXCommon_->GetCommandList();

	// ライトを設定
	commandList->SetGraphicsRootConstantBufferView(3, directionalLight.GetResource()->GetGPUVirtualAddress());

	// トランスフォームを設定（全メッシュ共通）
	commandList->SetGraphicsRootConstantBufferView(1, transform_.GetResource()->GetGPUVirtualAddress());

	// 全メッシュを描画（マルチマテリアル対応）
	const auto& meshes = sharedModel_->GetMeshes();
	for (size_t i = 0; i < meshes.size(); ++i) {
		const Mesh& mesh = meshes[i];

		// このメッシュが使用するマテリアルインデックスを取得
		size_t materialIndex = sharedModel_->GetMeshMaterialIndex(i);

		// 範囲チェック（安全のため）
		if (materialIndex >= sharedModel_->GetMaterialCount()) {
			materialIndex = 0; // フォールバック
		}

		// マテリアルを設定（直接Modelのマテリアルから取得）
		commandList->SetGraphicsRootConstantBufferView(0,
			sharedModel_->GetMaterial(materialIndex).GetResource()->GetGPUVirtualAddress());

		// テクスチャの設定
		if (!textureName_.empty()) {
			// カスタムテクスチャが設定されている場合
			commandList->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureHandle(textureName_));
		} else if (sharedModel_->HasTexture(materialIndex)) {
			// モデル付属のテクスチャがある場合
			commandList->SetGraphicsRootDescriptorTable(2,
				textureManager_->GetTextureHandle(sharedModel_->GetTextureTagName(materialIndex)));
		}

		// メッシュをバインドして描画
		const_cast<Mesh&>(mesh).Bind(commandList);
		const_cast<Mesh&>(mesh).Draw(commandList);
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
		} else {
			ImGui::Text("Shared Model Loaded: No");
		}

		// Transform
		if (ImGui::CollapsingHeader("Transform")) {
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

		// マルチマテリアル対応のMaterial設定（直接モデルのマテリアルを操作）
		if (ImGui::CollapsingHeader("Materials") && sharedModel_) {
			size_t materialCount = sharedModel_->GetMaterialCount();

			// マテリアル数表示
			ImGui::Text("Material Count: %zu", materialCount);
			ImGui::Separator();

			// 複数マテリアルがある場合のみ全マテリアル設定を表示
			if (materialCount > 1) {
				if (ImGui::TreeNode("All Materials")) {
					// 最初のマテリアルの値をベースとして使用
					Material& baseMaterial = sharedModel_->GetMaterial(0);
					Vector4 color = baseMaterial.GetColor();
					LightingMode lightingMode = baseMaterial.GetLightingMode();
					Vector2 uvPosition = baseMaterial.GetUVTransformTranslate();
					Vector2 uvScale = baseMaterial.GetUVTransformScale();
					float uvRotateZ = baseMaterial.GetUVTransformRotateZ();

					if (ImGui::ColorEdit4("Color", reinterpret_cast<float*>(&color.x))) {
						for (size_t i = 0; i < materialCount; ++i) {
							sharedModel_->GetMaterial(i).SetColor(color);
						}
					}

					// ライティング選択
					const char* lightingModeNames[] = { "None", "Lambert", "Half-Lambert" };
					int currentModeIndex = static_cast<int>(lightingMode);
					if (ImGui::Combo("Lighting", &currentModeIndex, lightingModeNames, IM_ARRAYSIZE(lightingModeNames))) {
						LightingMode newMode = static_cast<LightingMode>(currentModeIndex);
						for (size_t i = 0; i < materialCount; ++i) {
							sharedModel_->GetMaterial(i).SetLightingMode(newMode);
						}
					}

					// UV設定（全マテリアルに適用）
					if (ImGui::DragFloat2("UV Translate", &uvPosition.x, 0.01f, -10.0f, 10.0f)) {
						for (size_t i = 0; i < materialCount; ++i) {
							sharedModel_->GetMaterial(i).SetUVTransformTranslate(uvPosition);
						}
					}
					if (ImGui::DragFloat2("UV Scale", &uvScale.x, 0.01f, -10.0f, 10.0f)) {
						for (size_t i = 0; i < materialCount; ++i) {
							sharedModel_->GetMaterial(i).SetUVTransformScale(uvScale);
						}
					}
					if (ImGui::SliderAngle("UV Rotate", &uvRotateZ)) {
						for (size_t i = 0; i < materialCount; ++i) {
							sharedModel_->GetMaterial(i).SetUVTransformRotateZ(uvRotateZ);
						}
					}

					ImGui::TreePop();
				}
				ImGui::Separator();
			}

			// 個別マテリアル設定
			for (size_t i = 0; i < materialCount; ++i) {
				std::string materialName = std::format("Material {}", i);

				if (ImGui::TreeNode(materialName.c_str())) {
					Material& material = sharedModel_->GetMaterial(i);

					// 色設定（直接Modelのマテリアルを操作）
					Vector4 color = material.GetColor();
					if (ImGui::ColorEdit4("Color", reinterpret_cast<float*>(&color.x))) {
						material.SetColor(color);
					}

					// ライティング設定
					const char* lightingModeNames[] = { "None", "Lambert", "Half-Lambert" };
					LightingMode currentMode = material.GetLightingMode();
					int currentModeIndex = static_cast<int>(currentMode);
					if (ImGui::Combo("Lighting", &currentModeIndex, lightingModeNames, IM_ARRAYSIZE(lightingModeNames))) {
						material.SetLightingMode(static_cast<LightingMode>(currentModeIndex));
					}

					// UV設定（直接Modelのマテリアルを操作）
					Vector2 uvPosition = material.GetUVTransformTranslate();
					Vector2 uvScale = material.GetUVTransformScale();
					float uvRotateZ = material.GetUVTransformRotateZ();

					if (ImGui::DragFloat2("UV Translate", &uvPosition.x, 0.01f, -10.0f, 10.0f)) {
						material.SetUVTransformTranslate(uvPosition);
					}
					if (ImGui::DragFloat2("UV Scale", &uvScale.x, 0.01f, -10.0f, 10.0f)) {
						material.SetUVTransformScale(uvScale);
					}
					if (ImGui::SliderAngle("UV Rotate", &uvRotateZ)) {
						material.SetUVTransformRotateZ(uvRotateZ);
					}

					ImGui::TreePop();
				}
			}
		}

		// メッシュ情報
		if (ImGui::CollapsingHeader("Mesh Info") && sharedModel_) {
			ImGui::Text("Total Meshes: %zu", sharedModel_->GetMeshCount());
			ImGui::Text("Shared: Yes (Memory Optimized)");

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

		// テクスチャ設定
		if (ImGui::CollapsingHeader("Texture")) {
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