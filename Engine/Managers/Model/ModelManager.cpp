#include "ModelManager.h"
#include <cassert>

ModelManager* ModelManager::GetInstance() {
	static ModelManager instance;
	return &instance;
}

ModelManager::~ModelManager() {
	Finalize();
}

void ModelManager::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;
	textureManager_ = TextureManager::GetInstance();

	Logger::Log(Logger::GetStream(), "ModelManager initialized !!\n");
}

void ModelManager::Finalize() {
	// 全てのモデルを解放
	UnloadAll();
	dxCommon_ = nullptr;
	textureManager_ = nullptr;
}

bool ModelManager::LoadModel(const std::string& directoryPath, const std::string& filename, const std::string& tagName) {
	// 既に同じタグ名で登録されている場合はスキップ（成功として扱う）
	if (HasModel(tagName)) {
		Logger::Log(Logger::GetStream(), std::format("Model with tag '{}' already exists. Skipping load.\n", tagName));
		return true;
	}

	// 新しいモデルを作成
	auto model = std::make_unique<Model>();

	// モデルをロード
	if (!model->LoadFromOBJ(directoryPath, filename, dxCommon_)) {
		Logger::Log(Logger::GetStream(), std::format("Failed to load model: {} from {}\n", filename, directoryPath));
		return false;
	}

	// マップに登録
	models_[tagName] = std::move(model);

	Logger::Log(Logger::GetStream(), std::format("Model '{}' loaded successfully with tag '{}'\n", filename, tagName));
	return true;
}

bool ModelManager::LoadPrimitive(MeshType meshType, const std::string& tagName) {
	// 既に同じタグ名で登録されている場合はスキップ（成功として扱う）
	if (HasModel(tagName)) {
		Logger::Log(Logger::GetStream(), std::format("Primitive model with tag '{}' already exists. Skipping load.\n", tagName));
		return true;
	}

	// 新しいプリミティブモデルを作成
	auto model = std::make_unique<Model>();

	// プリミティブモデルをロード
	if (!model->LoadFromPrimitive(meshType, dxCommon_)) {
		Logger::Log(Logger::GetStream(), std::format("Failed to load primitive model: {}\n", Mesh::MeshTypeToString(meshType)));
		return false;
	}

	// マップに登録
	models_[tagName] = std::move(model);

	Logger::Log(Logger::GetStream(), std::format("Primitive model '{}' loaded successfully with tag '{}'\n",
		Mesh::MeshTypeToString(meshType), tagName));
	return true;
}

Model* ModelManager::GetModel(const std::string& tagName) {
	auto it = models_.find(tagName);
	if (it != models_.end()) {
		return it->second.get();
	}

	// モデルが見つからない場合はnullptr
	Logger::Log(Logger::GetStream(), std::format("Model with tag '{}' not found.\n", tagName));
	return nullptr;
}

void ModelManager::UnloadModel(const std::string& tagName) {
	auto modelIt = models_.find(tagName);
	if (modelIt != models_.end()) {
		// モデルをアンロード
		modelIt->second->Unload();

		// マップから削除
		models_.erase(modelIt);

		Logger::Log(Logger::GetStream(), std::format("Model with tag '{}' unloaded.\n", tagName));
	}
}

void ModelManager::UnloadAll() {
	// 全てのモデルを解放
	for (const auto& pair : models_) {
		Logger::Log(Logger::GetStream(), std::format("Unloading model: {}\n", pair.first));
		pair.second->Unload();
	}

	models_.clear();

	Logger::Log(Logger::GetStream(), "All models unloaded.\n");
}

bool ModelManager::HasModel(const std::string& tagName) const {
	return models_.find(tagName) != models_.end();
}