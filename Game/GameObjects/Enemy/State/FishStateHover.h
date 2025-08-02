#pragma once
#include "BaseEnemyState.h"
#include "MyMath/MyFunction.h"

// 前方宣言
class BaseEnemy;

/// <summary>
/// 射撃魚のホバー状態
/// プレイヤーの前方でうろうろする
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
	static constexpr float kHoverSpeed = 0.1f;

	// プレイヤーからの基準距離
	static constexpr float kBaseDistance = 100.0f;

	// ランダムオフセットの範囲
	static constexpr float kOffsetRange = 10.0f;

	// 射撃間隔（フレーム数）
	static constexpr int kShootInterval = 240;

	// 射撃タイマー
	int shootTimer_ = 0;

	// ホバー目標位置
	Vector3 hoverTarget_;

	// 目標位置変更タイマー
	int targetChangeTimer_ = 0;

	// 目標位置変更間隔
	static constexpr int kTargetChangeInterval = 60;

	/// <summary>
	/// 新しいホバー目標位置を生成
	/// </summary>
	void GenerateNewHoverTarget();
};