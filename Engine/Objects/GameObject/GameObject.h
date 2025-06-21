#pragma once
#include <string>
#include <memory>
#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "Objects/GameObject/Model.h"
#include "Objects/GameObject/Transform.h"
#include "Objects/Light/Light.h"


#include "Managers/Texture/TextureManager.h"


/// <summary>
/// ゲームオブジェクト
/// </summary>
class GameObject
{
public:
	GameObject() = default;
	virtual ~GameObject() = default;


	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="meshType">メッシュの種類</param>
	/// <param name="textureName">テクスチャ名</param>
	/// <param name="directoryPath">ディレクトリパス（OBJファイル用）</param>
	/// <param name="filename">ファイル名（OBJファイル用）</param>
	virtual void Initialize(
		DirectXCommon* dxCommon,
		MeshType meshType,
		const std::string& textureName = "",
		const std::string& directoryPath = "",
		const std::string& filename = ""
	);

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	virtual void Update(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// UVトランスフォームの更新
	/// </summary>
	void UpdateUVTransform() {
		model_.GetMaterial().UpdateUVTransform();
	}

	/// <summary>
	/// 描画処理
	/// </summary>
	/// <param name="directionalLight">平行光源</param>
	virtual void Draw(const Light& directionalLight);

	virtual void DrawWithCustomPSO(
		ID3D12RootSignature* rootSignature,
		ID3D12PipelineState* pipelineState,
		const Light& directionalLight);


	/// <summary>
	/// ImGui用のデバッグ表示
	/// </summary>
	virtual void ImGui();

	//Getter
	Vector3 GetPosition() const { return transform_.GetPosition(); }
	Vector3 GetRotation() const { return transform_.GetRotate(); }
	Vector3 GetScale() const { return transform_.GetScale(); }
	Vector4 GetColor() const { return model_.GetMaterial().GetColor(); }
	Transform& GetTransform() { return transform_; }
	const Transform& GetTransform() const { return transform_; }
	Model& GetModel() { return model_; }
	const Model& GetModel() const { return model_; }
	bool IsVisible() const { return isVisible_; }
	bool IsActive() const { return isActive_; }
	const std::string& GetName() const { return name_; }

	// Setter
	void SetTransform(const Vector3Transform& newTransform) { transform_.SetTransform(newTransform); }
	void SetPosition(const Vector3& position) { transform_.SetPosition(position); }
	void SetRotation(const Vector3& rotation) { transform_.SetRotate(rotation); }
	void SetScale(const Vector3& scale) { transform_.SetScale(scale); }

	void SetColor(const Vector4& color) { model_.GetMaterial().SetColor(color); }
	void SetVisible(bool visible) { isVisible_ = visible; }
	void SetActive(bool active) { isActive_ = active; }

	void SetName(const std::string& name) { name_ = name; }
	void SetTexture(const std::string& textureName) { textureName_ = textureName; }

	//　Transform操作
	void AddPosition(const Vector3& deltaPosition) { transform_.AddPosition(deltaPosition); }
	void AddRotation(const Vector3& deltaRotation) { transform_.AddRotation(deltaRotation); }
	void AddScale(const Vector3& deltaScale) { transform_.AddScale(deltaScale); }

	// Material操作 
	void SetLightingEnable(bool enable) { model_.GetMaterial().SetLightingEnable(enable); }
	void SetLambertianReflectance(bool enable) { model_.GetMaterial().SetLambertianReflectance(enable); }
	bool IsLightingEnabled() const { return model_.GetMaterial().IsLightingEnabled(); }
	bool IsLambertianReflectanceEnabled() const { return model_.GetMaterial().IsLambertianReflectanceEnabled(); }

protected:

	Model model_;
	Transform transform_;

	//それぞれの状態
	bool isVisible_ = true;				//
	bool isActive_ = true;				//
	std::string name_ = "GameObject";	//
	std::string textureName_ = "";		//

	//DirectX
	DirectXCommon* directXCommon_ = nullptr;
	TextureManager* textureManager_ = TextureManager::GetInstance();
private:

	// ImGui用の内部状態
	Vector3 imguiPosition_{ 0.0f, 0.0f, 0.0f };
	Vector3 imguiRotation_{ 0.0f, 0.0f, 0.0f };
	Vector3 imguiScale_{ 1.0f, 1.0f, 1.0f };
	Vector4 imguiColor_{ 1.0f, 1.0f, 1.0f, 1.0f };

	Vector2 imguiUvPosition_{ 0.0f,0.0f };
	float imguiUvRotateZ_{ 0.0f };
	Vector2 imguiUvScale_{ 1.0f,1.0f };


	bool imguiLighting_ = false;
	bool imguiLambertian_ = false;

};


/// <summary>
/// 三角形オブジェクト
/// </summary>
class Triangle : public GameObject
{
public:
	void Initialize(DirectXCommon* dxCommon, const std::string& textureName = "white") {
		GameObject::Initialize(dxCommon, MeshType::TRIANGLE, textureName);
		name_ = SettingName("Triangle");
	}

private:
	static int triangleCount_;

	/// 名前の後ろに番号をつけて識別しやすくする関数
	static std::string SettingName(const std::string& baseName) {
		return baseName + "_" + std::to_string(++triangleCount_);
	}
};

/// <summary>
/// 球体オブジェクト
/// </summary>
class Sphere : public GameObject
{
public:
	void Initialize(DirectXCommon* dxCommon, const std::string& textureName = "white") {
		GameObject::Initialize(dxCommon, MeshType::SPHERE, textureName);
		name_ = SettingName("Sphere");
		SetLightingEnable(true);
	}
private:
	static int sphereCount_;
	/// 名前の後ろに番号をつけて識別しやすくする関数
	static std::string SettingName(const std::string& baseName) {
		return baseName + "_" + std::to_string(++sphereCount_);
	}
};

/// <summary>
/// 3Dモデルオブジェクト
/// </summary>
class Model3D : public GameObject
{
public:
	void Initialize(DirectXCommon* dxCommon, const std::string& directoryPath = "resources/Model/Plane", const std::string& filename="plane.obj") {
		GameObject::Initialize(dxCommon, MeshType::MODEL_OBJ, "", directoryPath, filename);
		name_ = SettingName("Model (" + filename + ")");
		SetLightingEnable(true);
	}
	void Draw(const Light& directionalLight) {

		// 描画処理
		ID3D12GraphicsCommandList* commandList = directXCommon_->GetCommandList();
		commandList->SetGraphicsRootConstantBufferView(0, model_.GetMaterial().GetResource()->GetGPUVirtualAddress());			// マテリアルを設定
		commandList->SetGraphicsRootConstantBufferView(1, transform_.GetResource()->GetGPUVirtualAddress());					// トランスフォームを設定
		// テクスチャ名が設定されている場合のみ
		// テクスチャのSRVをバインド（OBJの場合は自動で読み込まれたテクスチャを使用）
		if (model_.HasTexture()) {
			commandList->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureHandle(model_.GetTextureTagName()));
		}

		commandList->SetGraphicsRootConstantBufferView(3, directionalLight.GetResource()->GetGPUVirtualAddress());				// ライトを設定

		// メッシュをバインドして描画
		model_.GetMesh().Bind(commandList);
		model_.GetMesh().Draw(commandList);
	}

private:
	static int modelCount_;
	/// 名前の後ろに番号をつけて識別しやすくする関数
	static std::string SettingName(const std::string& baseName) {
		return baseName + "_" + std::to_string(++modelCount_);
	}
};