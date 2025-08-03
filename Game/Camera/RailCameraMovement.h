#pragma once
#include "RailTrack.h"

/// <summary>
/// レールカメラの移動制御クラス
/// </summary>
class RailCameraMovement {
public:
	RailCameraMovement() = default;
	~RailCameraMovement() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(RailTrack* track);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	// 移動制御
	void StartMovement() { isMoving_ = true; }
	void StopMovement() { isMoving_ = false; }
	void ResetPosition();
	bool IsMoving() const { return isMoving_; }

	// 進行度
	float GetProgress() const { return t_; }
	void SetProgress(float progress);

	// 速度
	float GetSpeed() const { return speed_; }
	void SetSpeed(float speed) { speed_ = speed; }

	// ループ
	bool IsLoopEnabled() const { return loopEnabled_; }
	void SetLoopEnabled(bool enabled) { loopEnabled_ = enabled; }

	// 等間隔移動
	bool IsUniformSpeedEnabled() const { return uniformSpeedEnabled_; }
	void SetUniformSpeedEnabled(bool enabled) { uniformSpeedEnabled_ = enabled; }

	// 先読み距離
	float GetLookAheadDistance() const { return lookAheadDistance_; }
	void SetLookAheadDistance(float distance) { lookAheadDistance_ = distance; }

	// フレーム計算
	int GetCurrentFrameFromStart() const;
	float GetProgressFromFrame(int frame) const;
	void SetProgressFromFrame(int frame);
	int GetMaxFrames() const;

	// 現在位置と注視点
	Vector3 GetCurrentPosition() const;
	Vector3 GetLookAtTarget() const;

	// 終端状態
	bool IsAtEnd() const { return isAtEnd_; }

private:
	RailTrack* track_ = nullptr;

	// 移動パラメータ
	float t_ = 0.0f;
	float speed_ = 0.00025f;
	bool isMoving_ = true;
	bool loopEnabled_ = true;
	float lookAheadDistance_ = 0.01f;

	// 等間隔移動
	bool uniformSpeedEnabled_ = true;

	// 終端処理
	bool isAtEnd_ = false;
};