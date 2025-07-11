#pragma once
#include <memory>
#include <unordered_map>
#include <string>
#include "Managers/Scene/BaseScene.h"
#include "Managers/Scene/FadeManager.h"

/// <summary>
/// シーンを管理するクラス
/// </summary>
class SceneManager {
public:
	// シングルトン
	static SceneManager* GetInstance();


	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	// ImGui描画
	void ImGui();

	// シーンの管理
	void RegisterScene(const std::string& sceneName, std::unique_ptr<BaseScene> scene);
	void UnregisterScene(const std::string& sceneName);

	// シーンの切り替え
	bool ChangeScene(const std::string& sceneName);
	void SetNextScene(const std::string& sceneName);

	// フェードシーン遷移
	void FadeToScene(const std::string& sceneName, FadeManager::Status fadeOutStatus = FadeManager::Status::FadeOut, float fadeOutDuration = 1.0f, FadeManager::Status fadeInStatus = FadeManager::Status::FadeIn, float fadeInDuration = 1.0f);
	void FadeOutToScene(const std::string& sceneName, float duration = 1.0f);
	void FadeInToScene(const std::string& sceneName, float duration = 1.0f);

	// リセット機能（明示的にリセットしたい場合のみ）
	void ResetScene(const std::string& sceneName);
	void ResetCurrentScene();

	// シーンの存在確認
	bool HasScene(const std::string& sceneName) const;

	// 現在のシーン情報
	BaseScene* GetCurrentScene() const { return currentScene_; }
	const std::string& GetCurrentSceneName() const;

	/// <summary>
	/// 登録済みの全シーンのリソースを読み込み
	/// </summary>
	void LoadAllSceneResources();

private:
	// シングルトンパターン
	SceneManager() = default;
	~SceneManager() = default;
	SceneManager(const SceneManager&) = delete;
	SceneManager& operator=(const SceneManager&) = delete;

	void ProcessSceneChange();	// シーン切り替えの実処理
	void DrawScenesUI();		// シーン一覧・操作UI
	void DrawCurrentSceneUI();	// 現在のシーンのUI

	// フェード遷移用の内部処理
	void ProcessFadeTransition();

	std::unordered_map<std::string, std::unique_ptr<BaseScene>> scenes_;
	BaseScene* currentScene_ = nullptr;
	std::string currentSceneName_;
	std::string nextSceneName_;
	bool sceneChangeRequested_ = false;


	// フェード管理
	std::unique_ptr<FadeManager> fadeManager_;

	// フェード遷移用の状態管理
	enum class FadeTransitionState {
		None,
		FadeOut,
		ChangeScene,
		FadeIn
	};

	FadeTransitionState fadeTransitionState_ = FadeTransitionState::None;

	// フェード遷移用(一時的に持つデータ)
	std::string pendingSceneName_;
	FadeManager::Status pendingFadeInStatus_;
	float pendingFadeInDuration_;
};