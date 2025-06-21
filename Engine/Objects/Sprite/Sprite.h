#pragma once

#include <string>
#include <memory>
#include <d3d12.h>
#include <wrl.h>

#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "MyMath/MyFunction.h"
#include "Objects/Sprite/Transform2D.h"  // Transform2Dを使用

#include "Managers/Texture/TextureManager.h"
/// <summary>
/// スプライト専用のマテリアル構造体
/// </summary>
struct SpriteMaterial {
	Vector4 color;			// 色
	Matrix4x4 uvTransform;	// UV変換行列
};

/// <summary>
/// 2Dスプライト専用クラス（GameObjectから独立）
/// </summary>
class Sprite
{
public:
	Sprite() = default;
	~Sprite() = default;

	/// <summary>
	/// 初期化（centerとsizeはTransform2Dで管理）
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="textureName">テクスチャ名</param>
	/// <param name="center">中心座標（Transform2Dの初期位置として設定）</param>
	/// <param name="size">サイズ（Transform2Dの初期スケールとして設定）</param>
	void Initialize(
		DirectXCommon* dxCommon,
		const std::string& textureName,
		const Vector2& center,
		const Vector2& size
	);

	/// <summary>
	/// テクスチャなしで初期化（サイズのみ指定、位置は原点）
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="center">中心座標</param>
	/// <param name="size">サイズ/param>
	void Initialize(
		DirectXCommon* dxCommon,
		const Vector2& center = { 50.0f, 50.0f },
		const Vector2& size = { 100.0f, 100.0f }
	);

	/// <summary>
	/// 更新処理（スプライト専用のビュープロジェクション行列を使用）
	/// </summary>
	/// <param name="viewProjectionMatrix">スプライト用ビュープロジェクション行列</param>
	void Update(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// 通常の描画処理（UI用スプライト専用）
	/// </summary>
	void Draw();

	/// <summary>
	/// カスタムPSOを使用した描画（オフスクリーン用）
	/// </summary>
	/// <param name="rootSignature">使用するルートシグネチャ</param>
	/// <param name="pipelineState">使用するパイプラインステート</param>
	/// <param name="textureHandle">使用するテクスチャハンドル</param>
	/// <param name="materialBufferGPUAddress">使用するマテリアルバッファのGPUアドレス（0の場合は内部マテリアルを使用）</param>
	void DrawWithCustomPSO(
		ID3D12RootSignature* rootSignature,
		ID3D12PipelineState* pipelineState,
		D3D12_GPU_DESCRIPTOR_HANDLE textureHandle,
		D3D12_GPU_VIRTUAL_ADDRESS materialBufferGPUAddress = 0
	);

	/// <summary>
	/// ImGui用のデバッグ表示
	/// </summary>
	void ImGui();

	// Transform関連のGetter（2D用）
	Vector2 GetPosition() const { return transform_.GetPosition(); }
	float GetRotation() const { return transform_.GetRotation(); }
	Vector2 GetScale() const { return transform_.GetScale(); }
	Transform2D& GetTransform() { return transform_; }
	const Transform2D& GetTransform() const { return transform_; }

	// サイズ管理（スケールと同じ）
	Vector2 GetSize() const;
	void SetSize(const Vector2& size);
	void AddSize(const Vector2& deltaSize);

	// 3D互換性のためのGetter
	Vector3 GetPosition3D() const { return transform_.GetPosition3D(); }
	Vector3 GetRotation3D() const { return transform_.GetRotation3D(); }
	Vector3 GetScale3D() const { return transform_.GetScale3D(); }

	// Sprite固有のGetter
	Vector4 GetColor() const { return materialData_->color; }
	bool IsVisible() const { return isVisible_; }
	bool IsActive() const { return isActive_; }
	const std::string& GetName() const { return name_; }
	const std::string& GetTextureName() const { return textureName_; }

	// Transform関連のSetter（2D用）
	void SetTransform(const Vector2Transform& newTransform) { transform_.SetTransform(newTransform); }
	void SetPosition(const Vector2& position) { transform_.SetPosition(position); }
	void SetRotation(float rotation) { transform_.SetRotation(rotation); }
	void SetScale(const Vector2& scale) { transform_.SetScale(scale); }

	// 3D互換性のためのSetter
	void SetTransform3D(const Vector3Transform& newTransform) {
		Vector2Transform transform2D{
			{newTransform.scale.x, newTransform.scale.y},
			newTransform.rotate.z,
			{newTransform.translate.x, newTransform.translate.y}
		};
		transform_.SetTransform(transform2D);
	}
	void SetPosition3D(const Vector3& position) { transform_.SetPosition3D(position); }
	void SetRotation3D(const Vector3& rotation) { transform_.SetRotation3D(rotation); }
	void SetScale3D(const Vector3& scale) { transform_.SetScale3D(scale); }

	// Sprite固有のSetter
	void SetColor(const Vector4& color);
	void SetVisible(bool visible) { isVisible_ = visible; }
	void SetActive(bool active) { isActive_ = active; }
	void SetName(const std::string& name) { name_ = name; }
	void SetTexture(const std::string& textureName) { textureName_ = textureName; }

	// Transform操作（2D用）
	void AddPosition(const Vector2& deltaPosition) { transform_.AddPosition(deltaPosition); }
	void AddRotation(float deltaRotation) { transform_.AddRotation(deltaRotation); }
	void AddScale(const Vector2& deltaScale) { transform_.AddScale(deltaScale); }

	// 3D互換性のためのTransform操作
	void AddPosition3D(const Vector3& deltaPosition) {
		transform_.AddPosition({ deltaPosition.x, deltaPosition.y });
	}
	void AddRotation3D(const Vector3& deltaRotation) {
		transform_.AddRotation(deltaRotation.z);
	}
	void AddScale3D(const Vector3& deltaScale) {
		transform_.AddScale({ deltaScale.x, deltaScale.y });
	}

	// UVTransform関連
	void SetUVTransformScale(const Vector2& uvScale);
	void SetUVTransformRotateZ(float uvRotateZ);
	void SetUVTransformTranslate(const Vector2& uvTranslate);
	Vector2 GetUVTransformScale() const { return uvScale_; }
	float GetUVTransformRotateZ() const { return uvRotateZ_; }
	Vector2 GetUVTransformTranslate() const { return uvTranslate_; }

private:
	static int spriteCount_;

	/// <summary>
	/// 標準スプライトメッシュを作成（原点中心、サイズ1.0x1.0）
	/// </summary>
	void CreateStandardSpriteMesh();

	/// <summary>
	/// バッファを作成
	/// </summary>
	void CreateBuffers();

	/// <summary>
	/// UVトランスフォーム行列を更新
	/// </summary>
	void UpdateUVTransform();

	/// <summary>
	/// 名前の後ろに番号をつけて識別しやすくする関数
	/// </summary>
	static std::string SettingName(const std::string& baseName) {
		return baseName + "_" + std::to_string(++spriteCount_);
	}

private:
	// 基本情報
	DirectXCommon* directXCommon_ = nullptr;
	TextureManager* textureManager_ = TextureManager::GetInstance();

	// 状態
	bool isVisible_ = true;
	bool isActive_ = true;
	std::string name_ = "Sprite";
	std::string textureName_ = "";

	// Transform2Dクラスを使用
	Transform2D transform_;

	// SpriteMaterial構造体に対応したマテリアルデータ
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	SpriteMaterial* materialData_ = nullptr;

	// UV変換用のローカル変数（ImGuiとの連携用）
	Vector2 uvTranslate_{ 0.0f, 0.0f };
	Vector2 uvScale_{ 1.0f, 1.0f };
	float uvRotateZ_ = 0.0f;

	// メッシュデータ（標準メッシュ：原点中心、サイズ1.0x1.0）
	std::vector<VertexData> vertices_;
	std::vector<uint32_t> indices_;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

	// ImGui用の内部状態（2D用に最適化）
	Vector2 imguiPosition_{ 0.0f, 0.0f };
	float imguiRotation_{ 0.0f };
	Vector2 imguiScale_{ 1.0f, 1.0f };
	Vector4 imguiColor_{ 1.0f, 1.0f, 1.0f, 1.0f };
	Vector2 imguiUvPosition_{ 0.0f, 0.0f };
	Vector2 imguiUvScale_{ 1.0f, 1.0f };
	float imguiUvRotateZ_{ 0.0f };
};