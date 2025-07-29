#pragma once
#include <memory>
#include <functional>

// 前方宣言
class DirectXCommon;
class Sprite;

/// <summary>
/// トランジションエフェクトのインターフェース
/// </summary>
class BaseTransitionEffect {
public:
	/// <summary>
	/// トランジションの状態
	/// </summary>
	enum class State {
		None,       // 非アクティブ
		Enter,      // 開始中（画面を覆う）
		Exit,       // 終了中（画面を見せる）
		Completed   // 完了
	};

	virtual ~BaseTransitionEffect() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	virtual void Initialize(DirectXCommon* directXCommon) = 0;

	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="deltaTime">経過時間</param>
	virtual void Update(float deltaTime) = 0;

	/// <summary>
	/// 描画
	/// </summary>
	virtual void Draw() = 0;

	/// <summary>
	/// トランジション開始
	/// </summary>
	/// <param name="state">Enter or Exit</param>
	/// <param name="duration">持続時間</param>
	virtual void Start(State state, float duration) = 0;

	/// <summary>
	/// 強制停止
	/// </summary>
	virtual void Stop() = 0;

	/// <summary>
	/// リセット
	/// </summary>
	virtual void Reset() = 0;

	/// <summary>
	/// 現在の状態を取得
	/// </summary>
	virtual State GetState() const = 0;

	/// <summary>
	/// 完了したかどうか
	/// </summary>
	virtual bool IsCompleted() const = 0;

	/// <summary>
	/// アクティブかどうか
	/// </summary>
	virtual bool IsActive() const = 0;

	/// <summary>
	/// 進行率を取得（0.0f〜1.0f）
	/// </summary>
	virtual float GetProgress() const = 0;

	/// <summary>
	/// 終了処理
	/// </summary>
	virtual void Finalize() = 0;
};