#include "SceneManager.h"
#include "ImGuiManager.h"
#include <cassert>

SceneManager::SceneManager()
	: currentScene_(nullptr)
	, currentSceneName_("")
	, nextSceneName_("")
	, sceneChangeRequested_(false) {
}

SceneManager::~SceneManager() = default;

void SceneManager::RegisterScene(const std::string& sceneName, std::unique_ptr<BaseScene> scene) {
	assert(scene != nullptr);
	scenes_[sceneName] = std::move(scene);
}

void SceneManager::UnregisterScene(const std::string& sceneName) {
	auto it = scenes_.find(sceneName);
	if (it != scenes_.end()) {
		// 現在のシーンを削除する場合は現在のシーンをクリア
		if (currentSceneName_ == sceneName) {
			if (currentScene_) {
				currentScene_->OnExit();
				currentScene_->Finalize();
			}
			currentScene_ = nullptr;
			currentSceneName_.clear();
		}
		scenes_.erase(it);
	}
}

bool SceneManager::ChangeScene(const std::string& sceneName) {
	if (!HasScene(sceneName)) {
		return false;
	}

	// 現在のシーンがある場合の終了処理
	if (currentScene_) {
		currentScene_->OnExit();
		currentScene_->Finalize();
	}

	// 新しいシーンの設定
	currentScene_ = scenes_[sceneName].get();
	currentSceneName_ = sceneName;

	// 新しいシーンの初期化
	currentScene_->Initialize();
	currentScene_->OnEnter();

	return true;
}

void SceneManager::SetNextScene(const std::string& sceneName) {
	if (HasScene(sceneName)) {
		nextSceneName_ = sceneName;
		sceneChangeRequested_ = true;
	}
}

bool SceneManager::HasScene(const std::string& sceneName) const {
	return scenes_.find(sceneName) != scenes_.end();
}

const std::string& SceneManager::GetCurrentSceneName() const {
	return currentSceneName_;
}

void SceneManager::Initialize() {
	// 初期化時は何もしない（シーンは個別に初期化される）
}

void SceneManager::Update() {
	// シーン切り替えの処理
	ProcessSceneChange();

	// 現在のシーンの更新
	if (currentScene_) {
		currentScene_->Update();
	}
}

void SceneManager::Draw() {
	// 現在のシーンの描画
	if (currentScene_) {
		currentScene_->Draw();
	}
}

void SceneManager::Finalize() {
	// 現在のシーンの終了処理
	if (currentScene_) {
		currentScene_->OnExit();
		currentScene_->Finalize();
		currentScene_ = nullptr;
	}

	// 全シーンのクリア
	scenes_.clear();
	currentSceneName_.clear();
}

void SceneManager::ProcessSceneChange() {
	if (sceneChangeRequested_) {
		ChangeScene(nextSceneName_);
		sceneChangeRequested_ = false;
		nextSceneName_.clear();
	}
}

void SceneManager::ImGui() {
	ImGui::Begin("Scene Manager");

	// シーン管理UI
	DrawScenesUI();

	ImGui::Separator();

	// 現在のシーンのUI
	DrawCurrentSceneUI();

	ImGui::End();
}

void SceneManager::DrawScenesUI() {
	ImGui::Text("Scene Management");

	// 現在のシーン情報表示
	if (currentScene_) {
		ImGui::Text("Current Scene: %s", currentSceneName_.c_str());
		ImGui::TextColored(ImVec4(0, 1, 0, 1), "Status: Active");
	} else {
		ImGui::Text("Current Scene: None");
		ImGui::TextColored(ImVec4(1, 0, 0, 1), "Status: No Scene");
	}

	ImGui::Spacing();

	// 登録されているシーン一覧とボタン
	ImGui::Text("Available Scenes (%zu):", scenes_.size());

	for (const auto& [sceneName, scene] : scenes_) {
		ImGui::PushID(sceneName.c_str());

		// 現在のシーンかどうかで色を変える
		bool isCurrent = (currentSceneName_ == sceneName);
		if (isCurrent) {
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 0.6f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.8f, 0.2f, 0.8f));
		}

		// シーン切り替えボタン
		if (ImGui::Button(sceneName.c_str(), ImVec2(120, 0))) {
			if (!isCurrent) {
				SetNextScene(sceneName);
			}
		}

		if (isCurrent) {
			ImGui::PopStyleColor(2);
			ImGui::SameLine();
			ImGui::Text("(Current)");
		}

		ImGui::PopID();
	}

	// シーン切り替え要求があるかの表示
	if (sceneChangeRequested_) {
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "Next Scene: %s", nextSceneName_.c_str());
		ImGui::Text("(Will change next frame)");
	}
}

void SceneManager::DrawCurrentSceneUI() {
	ImGui::Text("Current Scene Debug");

	if (currentScene_) {
		ImGui::Text("Scene: %s", currentSceneName_.c_str());
		ImGui::Separator();

		// 現在のシーンのImGui
		currentScene_->ImGui();

	} else {
		ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1), "No active scene");
	}
}