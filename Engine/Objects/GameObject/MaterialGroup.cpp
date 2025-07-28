#include "MaterialGroup.h"

void MaterialGroup::Initialize(DirectXCommon* dxCommon, size_t materialCount) {
	directXCommon_ = dxCommon;
	ResizeMaterials(materialCount);
}

void MaterialGroup::ResizeMaterials(size_t count) {
	if (count == 0) { count = 1; } // 最低1つは必要

	// 既存のマテリアルを全て破棄
	materials_.clear();
	// 新しいサイズでベクターをリサイズ
	materials_.resize(count);

	// 各マテリアルを初期化
	for (auto& material : materials_) {
		material.Initialize(directXCommon_);
		material.SetLitObjectSettings(); // デフォルトでライティング有効
	}

	Logger::Log(Logger::GetStream(), std::format("MaterialGroup: Resized to {} materials\n", count));
}

Material& MaterialGroup::GetMaterial(size_t index) {
	// インデックスが配列の範囲外かチェック
	if (index >= materials_.size()) {
		Logger::Log(Logger::GetStream(), std::format("MaterialGroup: Index {} out of range (max: {}), using material 0\n",
			index, materials_.size() - 1));
		// 範囲外アクセスを防ぐため、安全にマテリアル0を返す
	   // （エラーを避けつつ、何らかの描画は継続される）
		return materials_[0];
	}
	// 正常な場合は指定されたインデックスのマテリアルを返す
	return materials_[index];
}

const Material& MaterialGroup::GetMaterial(size_t index) const {
	// 非const版と同様の範囲チェック
	if (index >= materials_.size()) {
		Logger::Log(Logger::GetStream(), std::format("MaterialGroup: Index {} out of range (max: {}), using material 0\n",
			index, materials_.size() - 1));

		// 安全にマテリアル0を返す
		return materials_[0];
	}

	// 正常な場合は指定されたインデックスのマテリアルを返す
	return materials_[index];
}

void MaterialGroup::UpdateAllUVTransforms() {
	// 全てのマテリアルに対し更新
	for (auto& material : materials_) {
		material.UpdateUVTransform();
	}
}

void MaterialGroup::SetAllMaterials(const Vector4& color, LightingMode lightingMode) {
	// 全てのマテリアルに対して同じ設定を適用
	for (auto& material : materials_) {
		material.SetColor(color);
		material.SetLightingMode(lightingMode);
	}
	Logger::Log(Logger::GetStream(), "MaterialGroup: Applied settings to all materials\n");
}

void MaterialGroup::InitializeFromTemplates(const std::vector<Material>& templateMaterials) {
	// テンプレートが空の場合はデフォルトマテリアル1つを作成
	if (templateMaterials.empty()) {
		// デフォルト設定で1つのマテリアルを作成
		ResizeMaterials(1);
		return;
	}
	// テンプレートの数に合わせてマテリアル配列をリサイズ
	ResizeMaterials(templateMaterials.size());

	// 各テンプレートマテリアルの設定を対応するマテリアルにコピー
	for (size_t i = 0; i < templateMaterials.size() && i < materials_.size(); ++i) {
		materials_[i].CopyFrom(templateMaterials[i]);
	}

	Logger::Log(Logger::GetStream(), std::format("MaterialGroup: Initialized from {} template materials\n", templateMaterials.size()));
}