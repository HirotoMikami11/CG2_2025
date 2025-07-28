#pragma once

#include <vector>
#include <string>
#include "Objects/GameObject/Material.h"
#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "BaseSystem/Logger/Logger.h"

/// <summary>
/// 複数のマテリアルをグループとして管理するクラス（Model用）
/// </summary>
class MaterialGroup {
public:
	MaterialGroup() = default;
	~MaterialGroup() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="materialCount">マテリアル数</param>
	void Initialize(DirectXCommon* dxCommon, size_t materialCount = 1);

	/// <summary>
	/// マテリアル数を設定（動的にリサイズ?）
	/// </summary>
	/// <param name="count">マテリアル数</param>
	void ResizeMaterials(size_t count);

	/// <summary>
	/// 指定したインデックスのマテリアルを取得
	/// </summary>
	/// <param name="index">マテリアルインデックス</param>
	/// <returns>マテリアルの参照</returns>
	Material& GetMaterial(size_t index = 0);
	const Material& GetMaterial(size_t index = 0) const;

	/// <summary>
	/// マテリアル数を取得
	/// </summary>
	/// <returns>マテリアル数</returns>
	size_t GetMaterialCount() const { return materials_.size(); }

	/// <summary>
	/// 全マテリアルのUVTransformを更新
	/// </summary>
	void UpdateAllUVTransforms();

	/// <summary>
	/// 全マテリアルに同じ設定を適用
	/// </summary>
	/// <param name="color">色</param>
	/// <param name="lightingMode">ライティングモード</param>
	void SetAllMaterials(const Vector4& color, LightingMode lightingMode);

	/// <summary>
	/// マテリアルテンプレートから初期化
	/// </summary>
	/// <param name="templateMaterials">テンプレートマテリアルリスト</param>
	void InitializeFromTemplates(const std::vector<Material>& templateMaterials);

	/// <summary>
	/// 空のマテリアルかチェック
	/// </summary>
	/// <returns>空かどうか</returns>
	bool IsEmpty() const { return materials_.empty(); }

private:
	std::vector<Material> materials_;
	DirectXCommon* directXCommon_ = nullptr;
};