#pragma once
#include <memory>
#include <functional>
#include <unordered_map>
#include <string>
#include "TransitionEffect/BaseTransitionEffect.h"

/// <summary>
/// トランジション管理クラス
/// シーン遷移のタイミングとエフェクトを管理
/// </summary>
class TransitionManager {
public:
	/// <summary>
	/// トランジション状態
	/// </summary>
	enum class TransitionState {
		None,           // 非アクティブ
		EnterEffect,    // エフェクト開始中
		Transition,     // 遷移処理
		ExitEffect,     // エフェクト終了中
		Completed       // 完了
	};

	// トランジション完了時のコールバック
	using TransitionCallback = std::function<void()>;

	// シングルトン
	static TransitionManager* GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// エフェクトを登録
	/// </summary>
	void RegisterEffect(const std::string& name, std::unique_ptr<BaseTransitionEffect> effect);

	/// <summary>
	/// エフェクトを取得
	/// </summary>
	BaseTransitionEffect* GetEffect(const std::string& name);

	/// <summary>
	/// トランジション開始
	/// </summary>
	/// <param name="effectName">使用するエフェクト名</param>
	/// <param name="enterDuration">開始エフェクトの時間</param>
	/// <param name="exitDuration">終了エフェクトの時間</param>
	/// <param name="onTransition">遷移時のコールバック</param>
	/// <param name="onComplete">完了時のコールバック</param>
	void StartTransition(
		const std::string& effectName,
		float enterDuration,
		float exitDuration,
		TransitionCallback onTransition = nullptr,
		TransitionCallback onComplete = nullptr
	);

	/// <summary>
	/// 簡易版：フェードアウト・インでトランジション
	/// </summary>
	void FadeTransition(
		float duration,
		TransitionCallback onTransition = nullptr,
		TransitionCallback onComplete = nullptr
	);

	/// <summary>
	/// カスタムトランジション（エフェクトを直接指定）
	/// </summary>
	void CustomTransition(
		BaseTransitionEffect* effect,
		float enterDuration,
		float exitDuration,
		TransitionCallback onTransition = nullptr,
		TransitionCallback onComplete = nullptr
	);

	/// <summary>
	/// 更新
	/// </summary>
	void Update(float deltaTime);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// 強制停止
	/// </summary>
	void Stop();

	/// <summary>
	/// 現在の状態を取得
	/// </summary>
	TransitionState GetState() const { return currentState_; }

	/// <summary>
	/// トランジション中かどうか
	/// </summary>
	bool IsTransitioning() const;

	/// <summary>
	/// アクティブなエフェクトを取得
	/// </summary>
	BaseTransitionEffect* GetActiveEffect() const { return activeEffect_; }

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

private:
	TransitionManager() = default;
	~TransitionManager() = default;
	TransitionManager(const TransitionManager&) = delete;
	TransitionManager& operator=(const TransitionManager&) = delete;

	void ProcessTransition();

	// エフェクトの管理
	std::unordered_map<std::string, std::unique_ptr<BaseTransitionEffect>> effects_;
	BaseTransitionEffect* activeEffect_ = nullptr;

	// トランジション状態
	TransitionState currentState_ = TransitionState::None;
	float enterDuration_ = 0.0f;
	float exitDuration_ = 0.0f;

	// コールバック
	TransitionCallback onTransitionCallback_;
	TransitionCallback onCompleteCallback_;

	// DirectX参照
	DirectXCommon* directXCommon_ = nullptr;
};