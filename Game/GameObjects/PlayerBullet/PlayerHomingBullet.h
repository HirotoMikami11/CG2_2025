#pragma once
#include "Objects/GameObject/GameObject.h"
#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "GameObjects/Collider.h"
#include "CollisionManager/CollisionConfig.h"

// 前方宣言
class Enemy;

/// <summary>
/// プレイヤーのホーミング弾クラス
/// </summary>
class PlayerHomingBullet : public Collider {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	PlayerHomingBullet();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~PlayerHomingBullet();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="position">初期位置</param>
	/// <param name="velocity">初期速度</param>
	/// <param name="target">ホーミング対象</param>
	void Initialize(DirectXCommon* dxCommon, const Vector3& position, const Vector3& velocity, Enemy* target);

	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	void Update(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="directionalLight">平行光源</param>
	void Draw(const Light& directionalLight);

	/// <summary>
	/// 死亡フラグを取得
	/// </summary>
	/// <returns>死亡フラグ</returns>
	bool IsDead() const { return isDead_; }

	/// <summary>
	/// ワールド座標を取得（オーバーライド）
	/// </summary>
	/// <returns>ワールド座標</returns>
	Vector3 GetWorldPosition() override;

	/// <summary>
	/// 衝突時に呼ばれる関数（オーバーライド）
	/// </summary>
	void OnCollision() override;

	/// <summary>
	/// ダメージを受ける
	/// </summary>
	/// <param name="damage">ダメージ量</param>
	void TakeDamage(int damage);

	/// <summary>
	/// HPを取得
	/// </summary>
	/// <returns>現在のHP</returns>
	int GetHP() const { return hp_; }

private:
	// ゲームオブジェクト
	std::unique_ptr<Model3D> gameObject_;

	// 速度
	Vector3 velocity_;

	// ホーミング対象
	Enemy* target_ = nullptr;

	// ホーミング用の変数
	float bulletSpeed_;  // 速度の大きさ
	float t_;            // ホーミングの補間割合

	// 寿命
	static const int32_t kLifeTime_ = 60 * 5; // 弾の寿命（フレーム数）
	int32_t deathTimer_ = kLifeTime_; // 弾の寿命タイマー
	bool isDead_ = false; // 弾が消滅したかどうかのフラグ
	// HP
	int hp_ = 1; // 弾のHP（デフォルト1）
	// システム参照
	DirectXCommon* directXCommon_ = nullptr;

	/// <summary>
	/// ホーミング処理
	/// </summary>
	/// <returns>新しい速度ベクトル</returns>
	Vector3 IsHoming();

	/// <summary>
	/// ターゲットの方向を向く
	/// </summary>
	void SetToTargetDirection();
};