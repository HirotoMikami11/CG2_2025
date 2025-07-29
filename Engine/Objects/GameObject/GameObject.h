#pragma once
#include <string>
#include <memory>
#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "Objects/GameObject/Transform3D.h"
#include "Objects/Light/Light.h"
#include "Managers/Texture/TextureManager.h"
#include "Managers/Model/ModelManager.h"
#include "Managers/ObjectID/ObjectIDManager.h"

/// <summary>
/// ゲームオブジェクト - 共有モデルと個別Transform、個別マテリアルを使用
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

	// Transform関連のGetter/Setter
	Vector3 GetPosition() const { return transform_.GetPosition(); }
	Vector3 GetRotation() const { return transform_.GetRotation(); }
	Vector3 GetScale() const { return transform_.GetScale(); }
	Transform3D& GetTransform() { return transform_; }
	const Transform3D& GetTransform() const { return transform_; }

	void SetTransform(const Vector3Transform& newTransform) { transform_.SetTransform(newTransform); }
	void SetPosition(const Vector3& position) { transform_.SetPosition(position); }
	void SetRotation(const Vector3& rotation) { transform_.SetRotation(rotation); }
	void SetScale(const Vector3& scale) { transform_.SetScale(scale); }

	// Transform操作
	void AddPosition(const Vector3& deltaPosition) { transform_.AddPosition(deltaPosition); }
	void AddRotation(const Vector3& deltaRotation) { transform_.AddRotation(deltaRotation); }
	void AddScale(const Vector3& deltaScale) { transform_.AddScale(deltaScale); }

	// Model関連のGetter
	Model* GetModel() { return sharedModel_; }
	const Model* GetModel() const { return sharedModel_; }

	// 個別マテリアル操作（NEW: 個別マテリアルシステム）
	Material& GetMaterial(size_t index = 0) {
		if (hasIndividualMaterials_ && index < individualMaterials_.GetMaterialCount()) {
			return individualMaterials_.GetMaterial(index);
		}
		return sharedModel_ ? sharedModel_->GetMaterial(index) : dummyMaterial_;
	}

	const Material& GetMaterial(size_t index = 0) const {
		if (hasIndividualMaterials_ && index < individualMaterials_.GetMaterialCount()) {
			return individualMaterials_.GetMaterial(index);
		}
		return sharedModel_ ? sharedModel_->GetMaterial(index) : dummyMaterial_;
	}

	size_t GetMaterialCount() const {
		if (hasIndividualMaterials_) {
			return individualMaterials_.GetMaterialCount();
		}
		return sharedModel_ ? sharedModel_->GetMaterialCount() : 0;
	}

	// メインマテリアル（インデックス0）のショートカット
	Vector4 GetColor() const {
		return GetMaterial(0).GetColor();
	}
	LightingMode GetLightingMode() const {
		return GetMaterial(0).GetLightingMode();
	}

	void SetColor(const Vector4& color) {
		// 個別マテリアルを作成してから色を設定
		CreateIndividualMaterials();
		individualMaterials_.GetMaterial(0).SetColor(color);
	}

	void SetLightingMode(LightingMode mode) {
		CreateIndividualMaterials();
		individualMaterials_.GetMaterial(0).SetLightingMode(mode);
	}

	// 全マテリアルに同じ設定を適用
	void SetAllMaterialsColor(const Vector4& color, LightingMode mode = LightingMode::HalfLambert) {
		CreateIndividualMaterials();
		individualMaterials_.SetAllMaterials(color, mode);
	}

	// UV操作（メインマテリアル用）
	void SetUVTransformScale(const Vector2& scale) {
		CreateIndividualMaterials();
		individualMaterials_.GetMaterial(0).SetUVTransformScale(scale);
	}
	void SetUVTransformRotateZ(float rotate) {
		CreateIndividualMaterials();
		individualMaterials_.GetMaterial(0).SetUVTransformRotateZ(rotate);
	}
	void SetUVTransformTranslate(const Vector2& translate) {
		CreateIndividualMaterials();
		individualMaterials_.GetMaterial(0).SetUVTransformTranslate(translate);
	}

	// オブジェクト状態
	bool IsVisible() const { return isVisible_; }
	bool IsActive() const { return isActive_; }
	const std::string& GetName() const { return name_; }
	const std::string& GetModelTag() const { return modelTag_; }

	void SetVisible(bool visible) { isVisible_ = visible; }
	void SetActive(bool active) { isActive_ = active; }
	void SetName(const std::string& name) { name_ = name; }

	// テクスチャ操作（プリミティブ用）
	void SetTexture(const std::string& textureName) { textureName_ = textureName; }
	const std::string& GetTextureName() const { return textureName_; }
	bool HasCustomTexture() const { return !textureName_.empty(); }

protected:
	// 個別に持つコンポーネント
	Transform3D transform_;					// 個別のトランスフォーム（位置、回転、スケール）

	// 共有リソースへの参照
	Model* sharedModel_ = nullptr;			// 共有モデルへのポインタ

	// NEW: 個別マテリアルシステム
	MaterialGroup individualMaterials_;		// 個別のマテリアルグループ
	bool hasIndividualMaterials_ = false;	// 個別マテリアルを持っているかのフラグ

	// オブジェクトの状態
	bool isVisible_ = true;
	bool isActive_ = true;
	std::string name_ = "GameObject";
	std::string modelTag_ = "";
	std::string textureName_ = "";			// プリミティブ用のテクスチャ名

	// システム参照
	DirectXCommon* directXCommon_ = nullptr;
	TextureManager* textureManager_ = TextureManager::GetInstance();
	ModelManager* modelManager_ = ModelManager::GetInstance();

	/// <summary>
	/// 個別マテリアルを作成する（共有モデルからコピー）
	/// </summary>
	void CreateIndividualMaterials() {
		if (hasIndividualMaterials_ || !sharedModel_) {
			return; // 既に作成済みまたは共有モデルがない
		}

		// 共有モデルと同じ数のマテリアルを作成
		size_t materialCount = sharedModel_->GetMaterialCount();
		individualMaterials_.Initialize(directXCommon_, materialCount);

		// 共有モデルのマテリアル設定をコピー
		for (size_t i = 0; i < materialCount; ++i) {
			individualMaterials_.GetMaterial(i).CopyFrom(sharedModel_->GetMaterial(i));
		}

		hasIndividualMaterials_ = true;
	}

private:
	// ダミーマテリアル（モデルがない場合の安全対策）
	static Material dummyMaterial_;

	// ImGui用の内部状態
	Vector3 imguiPosition_{ 0.0f, 0.0f, 0.0f };
	Vector3 imguiRotation_{ 0.0f, 0.0f, 0.0f };
	Vector3 imguiScale_{ 1.0f, 1.0f, 1.0f };
};

// 以下、特化クラスは変更なし
class Triangle : public GameObject
{
public:
	void Initialize(DirectXCommon* dxCommon, const std::string& modelTag = "triangle", const std::string& textureName = "white") {
		GameObject::Initialize(dxCommon, modelTag, textureName);
		name_ = ObjectIDManager::GetInstance()->GenerateName("Triangle");
		SetLightingMode(LightingMode::HalfLambert);
	}
};

class Sphere : public GameObject
{
public:
	void Initialize(DirectXCommon* dxCommon, const std::string& modelTag = "sphere", const std::string& textureName = "white") {
		GameObject::Initialize(dxCommon, modelTag, textureName);
		name_ = ObjectIDManager::GetInstance()->GenerateName("Sphere");
		SetLightingMode(LightingMode::HalfLambert);
	}
};

class Plane : public GameObject
{
public:
	void Initialize(DirectXCommon* dxCommon, const std::string& modelTag = "plane", const std::string& textureName = "white") {
		GameObject::Initialize(dxCommon, modelTag, textureName);
		name_ = ObjectIDManager::GetInstance()->GenerateName("Plane");
		SetLightingMode(LightingMode::HalfLambert);
	}
};

class Model3D : public GameObject
{
public:
	void Initialize(DirectXCommon* dxCommon, const std::string& modelTag, const std::string& textureName = "") {
		GameObject::Initialize(dxCommon, modelTag, textureName);
		name_ = ObjectIDManager::GetInstance()->GenerateName(std::format("Model ({})", modelTag), "Model3D");
		SetLightingMode(LightingMode::HalfLambert);
	}
};