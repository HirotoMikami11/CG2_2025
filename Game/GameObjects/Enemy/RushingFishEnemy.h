#pragma once
#include <memory>

#include "BaseEnemy.h"
#include "State/RushingFishStateHoming.h"

/// <summary>
/// 突進してくる魚の敵クラス
/// </summary>
class RushingFishEnemy : public BaseEnemy {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	RushingFishEnemy();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~RushingFishEnemy();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="position">初期位置</param>
	/// <param name="pattern">敵のパターン（通常はHoming）</param>
	void Initialize(DirectXCommon* dxCommon, const Vector3& position, EnemyPattern pattern = EnemyPattern::Homing) override;

	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	void Update(const Matrix4x4& viewProjectionMatrix) override;

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="directionalLight">平行光源</param>
	void Draw(const Light& directionalLight) override;

	/// <summary>
	/// ImGui
	/// </summary>
	void ImGui() override;

	/// <summary>
	/// 衝突時に呼ばれる関数（オーバーライド）
	/// </summary>
	void OnCollision() override;

private:
	// 移動速度
	static constexpr float kMoveSpeed = 0.5f;

	// プレイヤーへのダメージ量
	static constexpr float kPlayerDamage = 10.0f;

	/// <summary>
	/// 設定されたパターンに応じた初期Stateを設定
	/// </summary>
	void SetInitializeState() override;
};