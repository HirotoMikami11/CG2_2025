#include "FieldLoader.h"

FieldLoader::FieldLoader() {
}

FieldLoader::~FieldLoader() {
}

void FieldLoader::Initialize(DirectXCommon* dxCommon) {
	directXCommon_ = dxCommon;
	isFieldLoaded_ = false;
}

bool FieldLoader::LoadField(const std::string& fileName) {
	// 既存のフィールドをクリア
	ClearField();

	std::ifstream file(fileName);
	if (!file.is_open()) {
		return false;
	}

	std::string line;
	// ヘッダー行をスキップ
	if (!std::getline(file, line)) {
		file.close();
		return false;
	}

	// データ行を読み込み
	while (std::getline(file, line)) {
		std::stringstream ss(line);
		std::string cell;
		std::vector<std::string> values;

		while (std::getline(ss, cell, ',')) {
			values.push_back(cell);
		}

		if (values.size() >= 11) {
			try {
				// データを解析
				int id = std::stoi(values[0]);
				RockType rockType = Rock::StringToRockType(values[1]);
				Vector3 position = { std::stof(values[2]), std::stof(values[3]), std::stof(values[4]) };
				Vector3 rotation = { std::stof(values[5]), std::stof(values[6]), std::stof(values[7]) };
				Vector3 scale = { std::stof(values[8]), std::stof(values[9]), std::stof(values[10]) };

				// Rockオブジェクトを生成
				auto rock = std::make_unique<Rock>();
				rock->Initialize(directXCommon_, rockType, position, rotation, scale);
				rocks_.push_back(std::move(rock));

			} catch (const std::exception& e) {
				// データ変換エラーの場合はスキップ
				continue;
			}
		}
	}

	file.close();
	isFieldLoaded_ = true;
	return true;
}

void FieldLoader::Update(const Matrix4x4& viewProjectionMatrix) {
	if (!isFieldLoaded_) {
		return;
	}

	// 全ての岩を更新
	for (auto& rock : rocks_) {
		rock->Update(viewProjectionMatrix);
	}
}

void FieldLoader::Draw(const Light& directionalLight) {
	if (!isFieldLoaded_) {
		return;
	}

	// 全ての岩を描画
	for (auto& rock : rocks_) {
		rock->Draw(directionalLight);
	}
}

void FieldLoader::ClearField() {
	rocks_.clear();
	isFieldLoaded_ = false;
}