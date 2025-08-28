#pragma once
#include "BaseEnemyState.h"
#include "MyMath/MyFunction.h"

// 前方宣言
class BaseEnemy;

/// <summary>
/// 射撃魚のホバー状態
/// カメラの前方でうろうろする（カメラとの相対位置を維持）
/// </summary>
class FishStateHover : public BaseEnemyState {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="enemy">敵のポインタ</param>
	FishStateHover(BaseEnemy* enemy);

	/// <summary>
	/// デストラクタ
	/// </summary>
	~FishStateHover();

	/// <summary>
	/// 更新
	/// </summary>
	void Update() override;

private:
	// ホバー移動速度
	static constexpr float kHoverSpeed = 0.2f;

	// カメラからの基準距離
	static constexpr float kBaseDistance = 120.0f;

	// ランダムオフセットの範囲
	static constexpr float kOffsetRange = 15.0f;

	// 射撃間隔（フレーム数）
	static constexpr int kShootInterval = 120;

	// カメラが移動したと判定する閾値
	static constexpr float kCameraMovementThreshold = 1.0f;

	// カメラが回転したと判定する閾値
	static constexpr float kCameraRotationThreshold = 0.1f;

	// 射撃タイマー
	int shootTimer_ = 0;

	// ホバー目標位置
	Vector3 hoverTarget_;

	// 目標位置変更タイマー
	int targetChangeTimer_ = 0;

	// 目標位置変更間隔
	static constexpr int kTargetChangeInterval = 60;

	// 前回のカメラ位置（カメラの移動を検出するため）
	Vector3 lastCameraPosition_;

	// 前回のカメラ前方向（カメラの回転を検出するため）
	Vector3 lastCameraForward_;

	// 保存されたオフセット（カメラ座標系での右・上方向成分）
	float savedOffsetX_ = 0.0f;
	float savedOffsetY_ = 0.0f;

	/// <summary>
	/// 新しいホバー目標位置を生成（カメラ基準）
	/// </summary>
	void GenerateNewHoverTarget();
};