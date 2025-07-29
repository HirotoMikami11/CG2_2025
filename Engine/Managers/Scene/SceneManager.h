// SceneManager.h
#pragma once
#include <memory>
#include <unordered_map>
#include <string>
#include <functional>
#include "Managers/Scene/BaseScene.h"

/// <summary>
/// 純粋なシーン管理クラス
/// トランジション処理は含まない
/// </summary>
class SceneManager {
public:
	// シーン変更時のコールバック
	using SceneChangeCallback = std::function<void(const std::string& prevScene, const std::string& nextScene)>;

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
	/// 3D描画（オフスクリーン内）
	/// </summary>
	void DrawOffscreen();

	/// <summary>
	/// UI描画（オフスクリーン外）
	/// </summary>
	void DrawBackBuffer();

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// ImGui描画
	/// </summary>
	void ImGui();

	// シーンの管理
	void RegisterScene(const std::string& sceneName, std::unique_ptr<BaseScene> scene);
	void UnregisterScene(const std::string& sceneName);

	/// <summary>
	/// シーンの即時切り替え
	/// </summary>
	bool ChangeScene(const std::string& sceneName);

	/// <summary>
	/// 次フレームでのシーン切り替え予約
	/// </summary>
	void SetNextScene(const std::string& sceneName);

	/// <summary>
	/// シーンをリセット
	/// </summary>
	void ResetScene(const std::string& sceneName);
	void ResetCurrentScene();

	/// <summary>
	/// シーンの存在確認
	/// </summary>
	bool HasScene(const std::string& sceneName) const;

	/// <summary>
	/// 現在のシーン情報
	/// </summary>
	BaseScene* GetCurrentScene() const { return currentScene_; }
	const std::string& GetCurrentSceneName() const { return currentSceneName_; }

	/// <summary>
	/// 登録済みの全シーンのリソースを読み込み
	/// </summary>
	void LoadAllSceneResources();

	/// <summary>
	/// シーン変更時のコールバックを設定
	/// </summary>
	void SetSceneChangeCallback(SceneChangeCallback callback) { sceneChangeCallback_ = callback; }

private:
	// シングルトンパターン
	SceneManager() = default;
	~SceneManager() = default;
	SceneManager(const SceneManager&) = delete;
	SceneManager& operator=(const SceneManager&) = delete;

	void ProcessSceneChange();
	void DrawScenesUI();
	void DrawCurrentSceneUI();

	// シーン管理
	std::unordered_map<std::string, std::unique_ptr<BaseScene>> scenes_;
	BaseScene* currentScene_ = nullptr;
	std::string currentSceneName_;
	std::string nextSceneName_;
	bool sceneChangeRequested_ = false;

	// コールバック
	SceneChangeCallback sceneChangeCallback_;
};