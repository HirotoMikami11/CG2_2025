#pragma once
#include <string>

// シーンの基底クラス
class BaseScene {
public:
	BaseScene(const std::string& sceneName) : sceneName_(sceneName), isInitialized_(false) {}
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

	// ImGui描画（各シーンで実装）
	virtual void ImGui() {}

	// 初期化状態の管理
	bool IsInitialized() const { return isInitialized_; }
	void SetInitialized(bool initialized) { isInitialized_ = initialized; }

protected:
	std::string sceneName_;		//シーンの名前
	bool isInitialized_;		//初期化されているか否か
};