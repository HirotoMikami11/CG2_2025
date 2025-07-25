#include "SceneManager.h"
#include <cassert>
#include "Managers/ImGui/ImGuiManager.h" 

SceneManager* SceneManager::GetInstance() {
	static SceneManager instance;
	return &instance;
}

void SceneManager::Initialize() {
	fadeManager_ = std::make_unique<FadeManager>();
	fadeManager_->Initialize();
}

void SceneManager::LoadAllSceneResources() {
	// 登録されている全シーンのリソースを読み込み
	for (auto& [sceneName, scene] : scenes_) {
		if (!scene->IsResourcesLoaded()) {
			Logger::Log(Logger::GetStream(), std::format("Loading resources for scene: {}\n", sceneName));
			scene->LoadResources();
			scene->SetResourcesLoaded(true);
			Logger::Log(Logger::GetStream(), std::format("Resources loaded for scene: {}\n", sceneName));
		}
	}
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

void SceneManager::DrawOffscreen() {
	// 現在のシーンの3D描画（オフスクリーン内）
	if (currentScene_) {
		currentScene_->DrawOffscreen();
	}
}

void SceneManager::DrawBackBuffer() {
	// 現在のシーンのUI描画（オフスクリーン外）
	if (currentScene_) {
		currentScene_->DrawBackBuffer();
	}

	// フェードの描画（最前面、オフスクリーン外）
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

void SceneManager::RegisterScene(const std::string& sceneName, std::unique_ptr<BaseScene> scene) {
	assert(scene != nullptr);
	scenes_[sceneName] = std::move(scene);

	// 新しく登録されたシーンのリソースを即座に読み込み
	if (!scenes_[sceneName]->IsResourcesLoaded()) {
		Logger::Log(Logger::GetStream(), std::format("Loading resources for newly registered scene: {}\n", sceneName));
		scenes_[sceneName]->LoadResources();
		scenes_[sceneName]->SetResourcesLoaded(true);
		Logger::Log(Logger::GetStream(), std::format("Resources loaded for newly registered scene: {}\n", sceneName));
	}
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

	// 現在のシーンの処理（オブジェクトのみ解放）
	if (currentScene_) {
		currentScene_->OnExit();
		currentScene_->Finalize();
		// オブジェクト初期化フラグのみリセット（リソースフラグは保持）
		currentScene_->SetInitialized(false);
		// リソース読み込みフラグは保持: resourcesLoaded_ = true のまま
	}

	//シーンを変えるときにIDを全てリセットする
	//オフスクリーンなどで使用されるIDなどもここでリセットされる
	//シーン中に存在するものしかIDは付与されない
	ObjectIDManager::GetInstance()->ResetAllCounters();

	// 新しいシーンの設定
	currentScene_ = scenes_[sceneName].get();
	currentSceneName_ = sceneName;

	// リソースは既に読み込み済みなので、オブジェクト初期化のみ実行
	if (!currentScene_->IsInitialized()) {
		Logger::Log(Logger::GetStream(), std::format("Initializing objects for scene: {}\n", sceneName));
		currentScene_->Initialize();  // 軽量なオブジェクト作成のみ
		currentScene_->SetInitialized(true);
		Logger::Log(Logger::GetStream(), std::format("Objects initialized for scene: {}\n", sceneName));
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

	// リソース管理UI
	if (ImGui::CollapsingHeader("Resource Management")) {
		if (ImGui::Button("Reload All Scene Resources")) {
			LoadAllSceneResources();
		}

		ImGui::Separator();
		ImGui::Text("Scene Resource Status:");
		for (const auto& [sceneName, scene] : scenes_) {
			ImGui::Text("%s: Resources %s, Objects %s",
				sceneName.c_str(),
				scene->IsResourcesLoaded() ? "Loaded" : "Not Loaded",
				scene->IsInitialized() ? "Initialized" : "Not Initialized"
			);
		}
	}

	ImGui::Separator();

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
		ImGui::Text("Resources Loaded: %s", currentScene_->IsResourcesLoaded() ? "Yes" : "No");
		ImGui::Text("Objects Initialized: %s", currentScene_->IsInitialized() ? "Yes" : "No");
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
			ImGui::Text("(Res: %s, Obj: %s)",
				scene->IsResourcesLoaded() ? "Yes" : "No",
				scene->IsInitialized() ? "Yes" : "No"
			);
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
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "reset objects only");
	}

	// シーン切り替え要求があるかの表示
	if (sceneChangeRequested_) {
		ImGui::Spacing();
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "Next Scene: %s", nextSceneName_.c_str());
		ImGui::Text("(Will change next frame)");
	}

	// 描画レイヤー情報の表示
	ImGui::Separator();
	ImGui::Text("Rendering Layers:");
	ImGui::Text("3D Objects -> Offscreen (with post-processing)");
	ImGui::Text("UI Elements -> Direct to backbuffer");
	ImGui::Text("Fade Effect -> Direct to backbuffer (front-most)");
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