#pragma once
#include <string>

// シーンの基底クラス
class BaseScene {
public:
	BaseScene(const std::string& sceneName) : sceneName_(sceneName) {}
	virtual ~BaseScene() = default;

	// 純粋仮想関数
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
	virtual void Finalize() = 0;

	// シーン名の取得
	const std::string& GetSceneName() const { return sceneName_; }

	// シーンの状態管理
	virtual void OnEnter() {}	// シーンに入る時の処理
	virtual void OnExit() {}	// シーンから出る時の処理

	// ImGui描画
	virtual void ImGui() {}		// 各シーンで実装

protected:
	std::string sceneName_;
};