#pragma once
#include <memory>
#include <list>

#include "BaseEnemy.h"
#include "State/EnemyStateApproach.h"
#include "State/EnemyStateLeave.h"
#include "State/EnemyStateStraight.h"

/// <summary>
/// 通常の敵クラス（射撃する敵）
/// </summary>
class Enemy : public BaseEnemy {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	Enemy();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~Enemy();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="position">初期位置</param>
	/// <param name="pattern">敵のパターン</param>
	void Initialize(DirectXCommon* dxCommon, const Vector3& position, EnemyPattern pattern = EnemyPattern::Straight) override;

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

	/// <summary>
	/// 敵の弾丸を削除
	/// </summary>
	void DeleteBullets();

	/// <summary>
	/// タイマーを使って、一定間隔で弾丸を発射する関数
	/// </summary>
	void AutoFire();

	/// <summary>
	/// 自動発射を開始するフラグ
	/// </summary>
	void StartAutoFire();

	/// <summary>
	/// 弾リストを取得
	/// </summary>
	const std::list<std::unique_ptr<EnemyBullet>>& GetBullets() const { return bullets_; }

	// 発射間隔
	static const int kFireInterval_ = 60; // 発射間隔(フレーム数)

private:
	// 敵の弾丸
	std::list<std::unique_ptr<EnemyBullet>> bullets_;

	// 自動発射をするかどうか
	bool isAutoFire_ = false;

	// 弾の速度
	static constexpr float kBulletSpeed = 2.0f;

	void OnDeath() override {
		//実装を後回しにする
	}

	/// <summary>
	/// 設定されたパターンに応じた初期Stateを設定
	/// </summary>
	void SetInitializeState() override;
};