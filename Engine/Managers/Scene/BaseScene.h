#pragma once
#include <string>

/// <summary>
/// シーンの基底クラス
/// </summary>
class BaseScene {
public:
	BaseScene(const std::string& sceneName) : sceneName_(sceneName) {}
	virtual ~BaseScene() = default;

	/// <summary>
	/// リソース読み込み（起動時に1回のみ実行）
	/// </summary>
	virtual void LoadResources() {}

	/// <summary>
	/// シーンに入った時のオフスクリーン設定
	/// </summary>
	virtual void ConfigureOffscreenEffects() {}

	/// <summary>
	/// オブジェクト初期化
	/// </summary>
	virtual void Initialize() = 0;



	/// <summary>
	/// 更新処理
	/// </summary>
	virtual void Update() = 0;

	/// <summary>
	/// 3D描画処理（オフスクリーン内で描画される）
	/// グリッド、3Dオブジェクト、ライティングなど
	/// </summary>
	virtual void DrawOffscreen() = 0;

	/// <summary>
	/// UI描画処理（オフスクリーン外で描画される）
	/// スプライト、テキスト、2D要素など
	/// </summary>
	virtual void DrawBackBuffer() {}

	/// <summary>
	/// 終了処理
	/// </summary>
	virtual void Finalize() = 0;

	/// <summary>
	/// シーンに入る時の処理
	/// </summary>
	virtual void OnEnter() {}

	/// <summary>
	/// シーンから出る時の処理
	/// </summary>
	virtual void OnExit() {}

	/// <summary>
	/// ImGui描画
	/// </summary>
	virtual void ImGui() {}

	// リソース読み込み状態の管理
	bool IsResourcesLoaded() const { return resourcesLoaded_; }
	void SetResourcesLoaded(bool loaded) { resourcesLoaded_ = loaded; }

	// オブジェクト初期化状態の管理
	bool IsInitialized() const { return initialized_; }
	void SetInitialized(bool initialized) { initialized_ = initialized; }

	// シーン名の取得
	const std::string& GetSceneName() const { return sceneName_; }

protected:
	std::string sceneName_;

private:
	bool resourcesLoaded_ = false;  // リソース読み込み済みフラグ（保持される）
	bool initialized_ = false;      // オブジェクト初期化済みフラグ（切り替え時リセット）
};