#pragma once
#include "MyFunction.h"
#include <functional>

/// <summary>
/// 時限発動
/// </summary>
class TimedCall {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	TimedCall(std::function<void()> callback,uint32_t time);
	/// <summary>
	/// デストラクタ
	/// </summary>
	~TimedCall() = default;
	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	bool IsFinished() const { return isFinished_; }

private:
	// コールバック
	std::function<void()> callback_;
	// 残り時間
	uint32_t time_ = 0; // 残り時間
	// 終了フラグ
	bool isFinished_ = false; // 終了フラグ
};
