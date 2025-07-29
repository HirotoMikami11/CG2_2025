#include "TimedCall.h"

/// メンバ初期化子を使用?
TimedCall::TimedCall(std::function<void()> callback, uint32_t time) : callback_(callback), time_(time) {}

void TimedCall::Update() {

	// 処理が終わっているなら早期リターン
	if (isFinished_) {
		return;
	}

	// 残り時間を減らす
	time_--;

	// 残り時間が0以下になったら処理を終了
	if (time_ <= 0) {
		isFinished_ = true; // 処理が終わったフラグを立てる

		// コールバック関数を呼び出す
		callback_();
	}
}
