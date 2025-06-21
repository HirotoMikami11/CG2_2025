#include "GameObject.h"
#include "Managers/ImGuiManager.h" 

// 静的変数の定義
int Triangle::triangleCount_ = 0;
int Sphere::sphereCount_ = 0;
//int Sprite::spriteCount_ = 0;
int Model3D::modelCount_ = 0;

void GameObject::Initialize(
	DirectXCommon* dxCommon,
	MeshType meshType,
	const std::string& textureName,			//貼る画像の名前
	const std::string& directoryPath,		//使うモデルのパス
	const std::string& filename)			//使うモデルの名前
{


	directXCommon_ = dxCommon;
	textureName_ = textureName;

	// モデルの初期化
	model_.Initialize(dxCommon, meshType, directoryPath, filename);

	// トランスフォームの初期化
	transform_.Initialize(dxCommon);

	// デフォルトのトランスフォーム設定
	Vector3Transform defaultTransform{
		{1.0f, 1.0f, 1.0f},  // scale
		{0.0f, 0.0f, 0.0f},  // rotate
		{0.0f, 0.0f, 0.0f}   // translate
	};
	// トランスフォームをデフォルトに設定
	transform_.SetTransform(defaultTransform);

	// ImGui用の初期値を設定
	imguiPosition_ = transform_.GetPosition();
	imguiRotation_ = transform_.GetRotate();
	imguiScale_ = transform_.GetScale();
	imguiColor_ = model_.GetMaterial().GetColor();
	imguiLighting_ = model_.GetMaterial().IsLightingEnabled();
	imguiLambertian_ = model_.GetMaterial().IsLambertianReflectanceEnabled();
}

void GameObject::Update(const Matrix4x4& viewProjectionMatrix) {
	// アクティブでない場合は更新を止める
	if (!isActive_) {
		return;
	}

	// トランスフォーム行列の更新
	transform_.UpdateMatrix(viewProjectionMatrix);

}


void GameObject::Draw(const Light& directionalLight)
{
	// 非表示、アクティブでない場合は描画しない
	if (!isVisible_ || !isActive_) {
		return;
	}

	// 描画処理
	ID3D12GraphicsCommandList* commandList = directXCommon_->GetCommandList();
	commandList->SetGraphicsRootConstantBufferView(0, model_.GetMaterial().GetResource()->GetGPUVirtualAddress());			// マテリアルを設定
	commandList->SetGraphicsRootConstantBufferView(1, transform_.GetResource()->GetGPUVirtualAddress());					// トランスフォームを設定
	// テクスチャ名が設定されている場合のみ
	commandList->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureHandle(textureName_));		// テクスチャを設定
	commandList->SetGraphicsRootConstantBufferView(3, directionalLight.GetResource()->GetGPUVirtualAddress());				// ライトを設定

	// メッシュをバインドして描画
	model_.GetMesh().Bind(commandList);
	model_.GetMesh().Draw(commandList);

}

void GameObject::DrawWithCustomPSO(ID3D12RootSignature* rootSignature, ID3D12PipelineState* pipelineState, const Light& directionalLight)
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

	// 描画処理
	commandList->SetGraphicsRootConstantBufferView(0, model_.GetMaterial().GetResource()->GetGPUVirtualAddress());			// マテリアルを設定
	commandList->SetGraphicsRootConstantBufferView(1, transform_.GetResource()->GetGPUVirtualAddress());					// トランスフォームを設定
	// テクスチャ名が設定されている場合のみ
	commandList->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureHandle(textureName_));		// テクスチャを設定
	commandList->SetGraphicsRootConstantBufferView(3, directionalLight.GetResource()->GetGPUVirtualAddress());				// ライトを設定

	// メッシュをバインドして描画
	model_.GetMesh().Bind(commandList);
	model_.GetMesh().Draw(commandList);


	// 3Dの描画設定に戻す
	commandList->SetGraphicsRootSignature(directXCommon_->GetRootSignature());
	commandList->SetPipelineState(directXCommon_->GetPipelineState());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

}



void GameObject::ImGui()
{
#ifdef _DEBUG

	// 現在の名前を表示
	if (ImGui::TreeNode(name_.c_str())) {
		// 表示・アクティブ状態
		ImGui::Checkbox("Visible", &isVisible_);		//表示するか否か
		ImGui::Checkbox("Active", &isActive_);			//アクティブかどうか


		///*-----------------------------------------------------------------------*///
		///								Transform									///
		///*-----------------------------------------------------------------------*///

		if (ImGui::CollapsingHeader("Transform")) {
			// ImGui用の値を現在の値で更新
			imguiPosition_ = transform_.GetPosition();
			imguiRotation_ = transform_.GetRotate();
			imguiScale_ = transform_.GetScale();

			if (ImGui::DragFloat3("Position", &imguiPosition_.x, 0.01f)) {
				transform_.SetPosition(imguiPosition_);
			}
			if (ImGui::DragFloat3("Rotation", &imguiRotation_.x, 0.01f)) {
				transform_.SetRotate(imguiRotation_);
			}
			if (ImGui::DragFloat3("Scale", &imguiScale_.x, 0.01f)) {
				transform_.SetScale(imguiScale_);
			}
		}

		///*-----------------------------------------------------------------------*///
		///								Material									///
		///*-----------------------------------------------------------------------*///

		if (ImGui::CollapsingHeader("Material")) {
			// ImGui用の値を現在の値で更新
			imguiColor_ = model_.GetMaterial().GetColor();
			imguiLighting_ = model_.GetMaterial().IsLightingEnabled();
			imguiLambertian_ = model_.GetMaterial().IsLambertianReflectanceEnabled();

			if (ImGui::ColorEdit4("Color", reinterpret_cast<float*>(&imguiColor_.x))) {
				model_.GetMaterial().SetColor(imguiColor_);
			}
			if (ImGui::Checkbox("Enable Lighting", &imguiLighting_)) {
				model_.GetMaterial().SetLightingEnable(imguiLighting_);
			}
			if (ImGui::Checkbox("Lambertian Reflectance", &imguiLambertian_)) {
				model_.GetMaterial().SetLambertianReflectance(imguiLambertian_);
			}

			// ImGui用の値を現在の値で更新
			imguiUvPosition_ = model_.GetMaterial().GetUVTransformTranslate();
			imguiUvScale_ = model_.GetMaterial().GetUVTransformScale();
			imguiUvRotateZ_ = model_.GetMaterial().GetUVTransformRotateZ();

			ImGui::Text("UVTransform");
			if (ImGui::DragFloat2("UVtranslate", &imguiUvPosition_.x, 0.01f, -10.0f, 10.0f)) {
				model_.GetMaterial().SetUVTransformTranslate(imguiUvPosition_);
			}
			if (ImGui::DragFloat2("UVscale", &imguiUvScale_.x, 0.01f, -10.0f, 10.0f)) {
				model_.GetMaterial().SetUVTransformScale(imguiUvScale_);
			}
			if (ImGui::SliderAngle("UVrotate", &imguiUvRotateZ_)) {
				model_.GetMaterial().SetUVTransformRotateZ(imguiUvRotateZ_);
			}
		}


		///*-----------------------------------------------------------------------*///
		///									Texture									///
		///*-----------------------------------------------------------------------*///
		/// 
		if (ImGui::CollapsingHeader("Texture")) {
			ImGui::Text("Current Texture: %s", textureName_.c_str());
			///テクスチャの動的切り替え

			// uvCheckerボタン
			if (textureName_ == "uvChecker") {
				//押されている場合は色を押された色に変更する
				ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
				if (ImGui::Button("uvChecker")) {
					SetTexture("uvChecker");//押されたらテクスチャのタグを変更
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
				//押されている場合は色を押された色に変更する
				ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
				if (ImGui::Button("monsterBall")) {
					SetTexture("monsterBall");//押されたらテクスチャのタグを変更
				}
				ImGui::PopStyleColor();
			} else {
				if (ImGui::Button("monsterBall")) {
					SetTexture("monsterBall");
				}
			}
		}

		///*-----------------------------------------------------------------------*///
		///									メッシュ									///
		///*-----------------------------------------------------------------------*///
		if (ImGui::CollapsingHeader("Mesh Info")) {
			ImGui::Text("Mesh Type: %s", Mesh::MeshTypeToString(model_.GetMesh().GetMeshType()).c_str());
			ImGui::Text("Vertex Count: %d", model_.GetMesh().GetVertexCount());
			ImGui::Text("Index Count: %d", model_.GetMesh().GetIndexCount());
		}

		ImGui::TreePop();
	}
#endif
}