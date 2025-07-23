#pragma once
#include <string>
#include <memory>
#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "Objects/GameObject/Material.h"
#include "Objects/GameObject/Transform.h"
#include "Objects/Light/Light.h"
#include "Managers/Texture/TextureManager.h"
#include "Managers/Model/ModelManager.h"
#include "Managers/ObjectID/ObjectIDManager.h"

/// <summary>
/// ゲームオブジェクト - 共有モデルと個別Material/Transformを使用
/// </summary>
class GameObject
{
public:
	GameObject() = default;
	virtual ~GameObject() = default;

	/// <summary>
	/// 初期化（共有モデルを使用）
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="modelTag">共有モデルのタグ名</param>
	/// <param name="textureName">テクスチャ名（プリミティブ用、空文字列の場合はテクスチャなし）</param>
	virtual void Initialize(DirectXCommon* dxCommon, const std::string& modelTag, const std::string& textureName = "");

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	virtual void Update(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// 描画処理
	/// </summary>
	/// <param name="directionalLight">平行光源</param>
	virtual void Draw(const Light& directionalLight);

	/// <summary>
	/// ImGui用のデバッグ表示
	/// </summary>
	virtual void ImGui();

	// Getter
	Vector3 GetPosition() const { return transform_.GetPosition(); }
	Vector3 GetRotation() const { return transform_.GetRotation(); }
	Vector3 GetScale() const { return transform_.GetScale(); }
	Vector4 GetColor() const { return material_.GetColor(); }
	Transform& GetTransform() { return transform_; }
	const Transform& GetTransform() const { return transform_; }
	Material& GetMaterial() { return material_; }
	const Material& GetMaterial() const { return material_; }
	bool IsVisible() const { return isVisible_; }
	bool IsActive() const { return isActive_; }
	const std::string& GetName() const { return name_; }
	const std::string& GetModelTag() const { return modelTag_; }

	// Setter
	void SetTransform(const Vector3Transform& newTransform) { transform_.SetTransform(newTransform); }
	void SetPosition(const Vector3& position) { transform_.SetPosition(position); }
	void SetRotation(const Vector3& rotation) { transform_.SetRotation(rotation); }
	void SetScale(const Vector3& scale) { transform_.SetScale(scale); }
	void SetColor(const Vector4& color) { material_.SetColor(color); }
	void SetVisible(bool visible) { isVisible_ = visible; }
	void SetActive(bool active) { isActive_ = active; }
	void SetName(const std::string& name) { name_ = name; }

	// Transform操作
	void AddPosition(const Vector3& deltaPosition) { transform_.AddPosition(deltaPosition); }
	void AddRotation(const Vector3& deltaRotation) { transform_.AddRotation(deltaRotation); }
	void AddScale(const Vector3& deltaScale) { transform_.AddScale(deltaScale); }

	// Material操作
	void SetLightingMode(LightingMode mode) { material_.SetLightingMode(mode); }
	LightingMode GetLightingMode() const { return material_.GetLightingMode(); }

	// UV操作
	void SetUVTransformScale(const Vector2& scale) { material_.SetUVTransformScale(scale); }
	void SetUVTransformRotateZ(float rotate) { material_.SetUVTransformRotateZ(rotate); }
	void SetUVTransformTranslate(const Vector2& translate) { material_.SetUVTransformTranslate(translate); }

	// テクスチャ操作（プリミティブ用）
	void SetTexture(const std::string& textureName) { textureName_ = textureName; }
	const std::string& GetTextureName() const { return textureName_; }
	bool HasCustomTexture() const { return !textureName_.empty(); }

protected:
	// 個別に持つコンポーネント
	Material material_;                    // 個別のマテリアル（色、ライティング設定等）
	Transform transform_;                  // 個別のトランスフォーム（位置、回転、スケール）

	// 共有リソースへの参照
	Model* sharedModel_ = nullptr;         // 共有モデルへのポインタ

	// オブジェクトの状態
	bool isVisible_ = true;
	bool isActive_ = true;
	std::string name_ = "GameObject";
	std::string modelTag_ = "";
	std::string textureName_ = "";         // プリミティブ用のテクスチャ名

	// システム参照
	DirectXCommon* directXCommon_ = nullptr;
	TextureManager* textureManager_ = TextureManager::GetInstance();
	ModelManager* modelManager_ = ModelManager::GetInstance();

private:
	// ImGui用の内部状態
	Vector3 imguiPosition_{ 0.0f, 0.0f, 0.0f };
	Vector3 imguiRotation_{ 0.0f, 0.0f, 0.0f };
	Vector3 imguiScale_{ 1.0f, 1.0f, 1.0f };
	Vector4 imguiColor_{ 1.0f, 1.0f, 1.0f, 1.0f };
	Vector2 imguiUvPosition_{ 0.0f, 0.0f };
	float imguiUvRotateZ_{ 0.0f };
	Vector2 imguiUvScale_{ 1.0f, 1.0f };
};

/// <summary>
/// 三角形オブジェクト
/// </summary>
class Triangle : public GameObject
{
public:
	void Initialize(DirectXCommon* dxCommon, const std::string& modelTag = "triangle", const std::string& textureName = "white") {
		GameObject::Initialize(dxCommon, modelTag, textureName);
		name_ = idManager->GenerateName("Triangle");
		SetLightingMode(LightingMode::HalfLambert);
	}

private:
	ObjectIDManager* idManager = ObjectIDManager::GetInstance();
};

/// <summary>
/// 球体オブジェクト
/// </summary>
class Sphere : public GameObject
{
public:
	void Initialize(DirectXCommon* dxCommon, const std::string& modelTag = "sphere", const std::string& textureName = "white") {
		GameObject::Initialize(dxCommon, modelTag, textureName);
		name_ = idManager->GenerateName("Sphere");
		SetLightingMode(LightingMode::HalfLambert);
	}

private:
	ObjectIDManager* idManager = ObjectIDManager::GetInstance();
};

/// <summary>
/// 平面オブジェクト
/// </summary>
class Plane : public GameObject
{
public:
	void Initialize(DirectXCommon* dxCommon, const std::string& modelTag = "plane", const std::string& textureName = "white") {
		GameObject::Initialize(dxCommon, modelTag, textureName);
		name_ = idManager->GenerateName("Plane");
		SetLightingMode(LightingMode::HalfLambert);
	}

private:
	ObjectIDManager* idManager = ObjectIDManager::GetInstance();
};

/// <summary>
/// 3Dモデルオブジェクト
/// </summary>
class Model3D : public GameObject
{
public:
	void Initialize(DirectXCommon* dxCommon, const std::string& modelTag, const std::string& textureName = "") {
		GameObject::Initialize(dxCommon, modelTag, textureName);
		name_ = idManager->GenerateName(std::format("Model ({})", modelTag), "Model3D");
		SetLightingMode(LightingMode::HalfLambert);

	}

private:
	ObjectIDManager* idManager = ObjectIDManager::GetInstance();
};