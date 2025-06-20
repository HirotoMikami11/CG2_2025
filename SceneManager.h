#pragma once
#include <memory>
#include <unordered_map>
#include <string>
#include "BaseScene.h"

class SceneManager {
public:
	SceneManager();
	~SceneManager();

	// シーンの管理
	void RegisterScene(const std::string& sceneName, std::unique_ptr<BaseScene> scene);
	void UnregisterScene(const std::string& sceneName);

	// シーンの切り替え（データ自動保持）
	bool ChangeScene(const std::string& sceneName);
	void SetNextScene(const std::string& sceneName);

	// リセット機能（明示的にリセットしたい場合のみ）
	void ResetScene(const std::string& sceneName);
	void ResetCurrentScene();

	// シーンの存在確認
	bool HasScene(const std::string& sceneName) const;

	// 現在のシーン情報
	BaseScene* GetCurrentScene() const { return currentScene_; }
	const std::string& GetCurrentSceneName() const;

	// メインループ
	void Initialize();
	void Update();
	void Draw();
	void Finalize();

	// ImGui描画
	void ImGui();

private:
	void ProcessSceneChange();	// シーン切り替えの実処理
	void DrawScenesUI();		// シーン一覧・操作UI
	void DrawCurrentSceneUI();	// 現在のシーンのUI

	std::unordered_map<std::string, std::unique_ptr<BaseScene>> scenes_;
	BaseScene* currentScene_;
	std::string currentSceneName_;
	std::string nextSceneName_;
	bool sceneChangeRequested_;
};