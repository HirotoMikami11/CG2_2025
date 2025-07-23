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

	// 個別のマテリアルとトランスフォームを設定
	commandList->SetGraphicsRootConstantBufferView(0, material_.GetResource()->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(1, transform_.GetResource()->GetGPUVirtualAddress());

	// テクスチャの設定（優先順位：カスタムテクスチャ > モデル付属テクスチャ）
	if (!textureName_.empty()) {
		// プリミティブ等でカスタムテクスチャが設定されている場合
		commandList->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureHandle(textureName_));
	} else if (sharedModel_->HasTexture()) {
		// OBJファイル等でモデル付属のテクスチャがある場合
		commandList->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureHandle(sharedModel_->GetTextureTagName()));
	}

	// ライトを設定
	commandList->SetGraphicsRootConstantBufferView(3, directionalLight.GetResource()->GetGPUVirtualAddress());

	// 共有メッシュをバインドして描画
	Mesh& mesh = sharedModel_->GetMesh();
	mesh.Bind(commandList);
	mesh.Draw(commandList);
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
			ImGui::Text("Has Texture: %s", sharedModel_->HasTexture() ? "Yes" : "No");
			if (sharedModel_->HasTexture()) {
				ImGui::Text("Texture Tag: %s", sharedModel_->GetTextureTagName().c_str());
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

		// メッシュ情報（共有メッシュの情報を表示）
		if (ImGui::CollapsingHeader("Mesh Info") && sharedModel_) {
			Mesh& mesh = sharedModel_->GetMesh();
			ImGui::Text("Mesh Type: %s", Mesh::MeshTypeToString(mesh.GetMeshType()).c_str());
			ImGui::Text("Vertex Count: %d", mesh.GetVertexCount());
			ImGui::Text("Index Count: %d", mesh.GetIndexCount());
			ImGui::Text("Shared: Yes (Memory Optimized)");
		}

		// テクスチャ設定（プリミティブ用）
		if (ImGui::CollapsingHeader("Texture")) {
			if (sharedModel_ && sharedModel_->HasTexture()) {
				ImGui::Text("Model Texture: %s", sharedModel_->GetTextureTagName().c_str());
			} else {
				ImGui::Text("Model Texture: None");
			}

			if (!textureName_.empty()) {
				ImGui::Text("Current Custom Texture: %s", textureName_.c_str());
			} else {
				ImGui::Text("Current Custom Texture: None");
			}

			// テクスチャの動的切り替え（プリミティブ用）
			ImGui::Separator();
			ImGui::Text("Set Custom Texture:");

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

			ImGui::SameLine();

			// whiteボタン
			if (textureName_ == "white") {
				ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
				if (ImGui::Button("white")) {
					SetTexture("white");
				}
				ImGui::PopStyleColor();
			} else {
				if (ImGui::Button("white")) {
					SetTexture("white");
				}
			}
		}

		ImGui::TreePop();
	}
#endif
}