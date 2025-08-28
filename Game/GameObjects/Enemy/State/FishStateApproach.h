#pragma once
#include "BaseEnemyState.h"
#include "MyMath/MyFunction.h"

// 前方宣言
class BaseEnemy;

/// <summary>
/// 射撃魚の接近状態
/// カメラの前方50付近の平面上のランダムな位置まで移動する
/// </summary>
class FishStateApproach : public BaseEnemyState {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="enemy">敵のポインタ</param>
	FishStateApproach(BaseEnemy* enemy);

	/// <summary>
	/// デストラクタ
	/// </summary>
	~FishStateApproach();

	/// <summary>
	/// 更新
	/// </summary>
	void Update() override;

private:
	// 移動速度
	static constexpr float kMoveSpeed = 1.2f;

	// カメラの前方に停止する距離
	static constexpr float kTargetDistance = 120.0f;

	// 距離の許容誤差
	static constexpr float kDistanceTolerance = 5.0f;

	// カメラ座標系でのオフセット範囲（x,y方向に-+10）
	static constexpr float kOffsetRange = 10.0f;

	// カメラが移動したと判定する閾値
	static constexpr float kCameraMovementThreshold = 1.0f;

	// カメラが回転したと判定する閾値
	static constexpr float kCameraRotationThreshold = 0.1f;

	// 目標位置
	Vector3 targetPosition_;

	// 前回のカメラ位置（カメラの移動を検出するため）
	Vector3 lastCameraPosition_;

	// 前回のカメラ前方向（カメラの回転を検出するため）
	Vector3 lastCameraForward_;

	// 保存されたオフセット（カメラ座標系での右・上方向成分）
	float savedOffsetX_ = 0.0f;
	float savedOffsetY_ = 0.0f;

	/// <summary>
	/// カメラ基準で目標位置を計算する
	/// </summary>
	void CalculateTargetPosition();
};