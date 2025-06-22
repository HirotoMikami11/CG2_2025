#include "SceneManager.h"
#include <cassert>
#include "Managers/ImGuiManager.h" 


SceneManager* SceneManager::GetInstance() {
	static SceneManager instance;
	return &instance;
}

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

	// 現在のシーンの処理
	if (currentScene_) {
		currentScene_->OnExit();
		//TODO: シーンのリセット(モデルを読み込むので重い)
		currentScene_->Finalize();
		currentScene_->SetInitialized(false);
	}

	// 新しいシーンの設定
	currentScene_ = scenes_[sceneName].get();
	currentSceneName_ = sceneName;

	// 新しいシーンの初期化（初回のみ）
	if (!currentScene_->IsInitialized()) {
		currentScene_->Initialize();
		currentScene_->SetInitialized(true);
	}

	currentScene_->OnEnter();

	return true;
}

void SceneManager::FadeToScene(const std::string& sceneName, FadeManager::Status fadeOutStatus, float fadeOutDuration, FadeManager::Status fadeInStatus, float fadeInDuration) {
	if (!HasScene(sceneName) || fadeTransitionState_ != FadeTransitionState::None) {
		return;
	}

	// フェード遷移の開始
	pendingSceneName_ = sceneName;
	pendingFadeInStatus_ = fadeInStatus;
	pendingFadeInDuration_ = fadeInDuration;

	fadeTransitionState_ = FadeTransitionState::FadeOut;
	fadeManager_->Start(fadeOutStatus, fadeOutDuration);
}

void SceneManager::FadeOutToScene(const std::string& sceneName, float duration) {
	FadeToScene(sceneName, FadeManager::Status::FadeOut, duration, FadeManager::Status::FadeIn, duration);
}

void SceneManager::FadeInToScene(const std::string& sceneName, float duration) {
	FadeToScene(sceneName, FadeManager::Status::FadeOut, duration, FadeManager::Status::FadeIn, duration);
}

void SceneManager::SetNextScene(const std::string& sceneName) {
	if (HasScene(sceneName)) {
		nextSceneName_ = sceneName;
		sceneChangeRequested_ = true;
	}
}

void SceneManager::ResetScene(const std::string& sceneName) {
	auto it = scenes_.find(sceneName);
	if (it != scenes_.end()) {
		// 現在のシーンをリセットする場合
		if (currentSceneName_ == sceneName && currentScene_) {
			currentScene_->OnExit();
			currentScene_->Finalize();
			currentScene_->Initialize();
			currentScene_->SetInitialized(true);
			currentScene_->OnEnter();
		} else {
			// 非アクティブなシーンをリセット
			it->second->Finalize();
			it->second->SetInitialized(false);
		}
	}
}

void SceneManager::ResetCurrentScene() {
	if (currentScene_ && !currentSceneName_.empty()) {
		ResetScene(currentSceneName_);
	}
}

bool SceneManager::HasScene(const std::string& sceneName) const {
	return scenes_.find(sceneName) != scenes_.end();
}

const std::string& SceneManager::GetCurrentSceneName() const {
	return currentSceneName_;
}

void SceneManager::Initialize() {
	fadeManager_ = std::make_unique<FadeManager>();
	fadeManager_->Initialize();
}

void SceneManager::Update() {

	// FadeManagerの更新
	if (fadeManager_) {
		fadeManager_->Update();
	}
	// フェード遷移の処理
	ProcessFadeTransition();

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
	// フェードの描画（最前面）
	if (fadeManager_) {
		fadeManager_->Draw();
	}
}

void SceneManager::Finalize() {
	// 現在のシーンの終了処理
	if (currentScene_) {
		currentScene_->OnExit();
		currentScene_->Finalize();
		currentScene_ = nullptr;
	}

	// FadeManagerの終了処理
	if (fadeManager_) {
		fadeManager_->Finalize();
		fadeManager_.reset();
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

void SceneManager::ProcessFadeTransition() {
	switch (fadeTransitionState_) {
	case FadeTransitionState::None:
		// 何もしない
		break;

	case FadeTransitionState::FadeOut:
		// フェードアウト完了でシーン切り替え
		if (fadeManager_->IsFinished()) {
			fadeTransitionState_ = FadeTransitionState::ChangeScene;
		}
		break;

	case FadeTransitionState::ChangeScene:
		// シーン切り替え実行
		ChangeScene(pendingSceneName_);

		// フェードイン開始
		fadeTransitionState_ = FadeTransitionState::FadeIn;
		fadeManager_->Start(pendingFadeInStatus_, pendingFadeInDuration_);
		break;

	case FadeTransitionState::FadeIn:
		// フェードイン完了で遷移終了
		if (fadeManager_->IsFinished()) {
			fadeTransitionState_ = FadeTransitionState::None;
			pendingSceneName_.clear();
		}
		break;
	}
}

void SceneManager::ImGui() {
#ifdef _DEBUG

	ImGui::Begin("Scene Manager");

	// シーン管理UI
	DrawScenesUI();

	ImGui::Separator();

	// 現在のシーンのUI
	DrawCurrentSceneUI();

	ImGui::End();
#endif
}

void SceneManager::DrawScenesUI() {
#ifdef _DEBUG

	ImGui::Text("Scene Management");

	// 現在のシーン情報表示
	if (currentScene_) {
		ImGui::Text("Current Scene: %s", currentSceneName_.c_str());
		ImGui::TextColored(ImVec4(0, 1, 0, 1), "Status: Active");
		ImGui::Text("Initialized: %s", currentScene_->IsInitialized() ? "Yes" : "No");
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
		} else {
			ImGui::SameLine();
			ImGui::Text("(Init: %s)", scene->IsInitialized() ? "Yes" : "No");
		}

		// リセットボタン
		ImGui::SameLine();
		if (ImGui::SmallButton(("Reset##" + sceneName).c_str())) {
			ResetScene(sceneName);
		}

		ImGui::PopID();
	}

	// 現在のシーンのリセットボタン
	if (currentScene_) {
		ImGui::Spacing();
		if (ImGui::Button("Reset Current Scene", ImVec2(200, 0))) {
			ResetCurrentScene();
		}
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "reset all objects");
	}

	// シーン切り替え要求があるかの表示
	if (sceneChangeRequested_) {
		ImGui::Spacing();
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "Next Scene: %s", nextSceneName_.c_str());
		ImGui::Text("(Will change next frame)");
	}

#endif
}

void SceneManager::DrawCurrentSceneUI() {
#ifdef _DEBUG

	ImGui::Text("Current Scene Debug");

	if (currentScene_) {
		ImGui::Text("Scene: %s", currentSceneName_.c_str());
		ImGui::Separator();

		// 現在のシーンのImGuiを呼び出し
		currentScene_->ImGui();
	} else {
		ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1), "No active scene");
	}
#endif
}