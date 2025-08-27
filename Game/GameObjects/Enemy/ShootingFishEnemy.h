#pragma once
#include <memory>

#include "BaseEnemy.h"
#include "GameObjects/Enemy/State/FishStateApproach.h"
#include "GameObjects/Enemy/State/FishStateHover.h"
#include "GameObjects/Enemy/State/FishStateShoot.h"
#include "GameObjects/Enemy/State/FishStateEscape.h"

/// <summary>
/// 射撃する魚の敵クラス
/// プレイヤーの前方に移動してうろうろし、4回射撃した後に逃げる
/// </summary>
class ShootingFishEnemy : public BaseEnemy {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	ShootingFishEnemy();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~ShootingFishEnemy();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="position">初期位置</param>
	/// <param name="pattern">敵のパターン（通常はShoting）</param>
	void Initialize(DirectXCommon* dxCommon, const Vector3& position, EnemyPattern pattern = EnemyPattern::Shooting) override;

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
	/// 弾丸発射
	/// </summary>
	void Fire();

	int GetShotCount() { return shotCount_; };
	void AddShotCount() { shotCount_++; };
private:
	// 移動速度
	static constexpr float kMoveSpeed = 1.0f;

	// 敵の弾丸
	std::list<std::unique_ptr<EnemyBullet>> bullets_;

	// 弾の速度
	static constexpr float kBulletSpeed = 2.0f;

	// 射撃回数
	int shotCount_ = 0;

	/// <summary>
	/// 設定されたパターンに応じた初期Stateを設定
	/// </summary>
	void SetInitializeState() override;
};