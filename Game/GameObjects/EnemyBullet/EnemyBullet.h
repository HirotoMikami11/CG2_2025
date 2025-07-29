#pragma once
#include "Objects/GameObject/GameObject.h"
#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "Objects/Light/Light.h"
#include "GameObjects/Collider.h"	//衝突判定

// プレイヤークラスの前方宣言
class Player;

/// <summary>
/// 敵の弾クラス
/// </summary>
class EnemyBullet : public Collider {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	EnemyBullet();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~EnemyBullet();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="position">初期位置</param>
	/// <param name="velocity">速度</param>
	/// <param name="speed">弾の速度</param>
	void Initialize(DirectXCommon* dxCommon, const Vector3& position, const Vector3& velocity, const float& speed);

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
	/// プレイヤーを設定
	/// </summary>
	/// <param name="player">プレイヤーのポインタ</param>
	void SetPlayer(Player* player) { player_ = player; }

private:
	// ゲームオブジェクト
	std::unique_ptr<Sphere> gameObject_;

	// 速度
	Vector3 velocity_;

	// 寿命
	static const int32_t kLifeTime_ = 60 * 5; // 弾の寿命（フレーム数）
	int32_t deathTimer_ = kLifeTime_; // 弾の寿命タイマー
	bool isDead_ = false; // 弾が消滅したかどうかのフラグ

	// プレイヤーの情報
	Player* player_ = nullptr;

	// ホーミング用の変数
	float bulletSpeed_; // 速度
	float t_; // ホーミングの補間割合

	// システム参照
	DirectXCommon* directXCommon_ = nullptr;

	/// <summary>
	/// ホーミング
	/// </summary>
	/// <returns>新しい速度ベクトル</returns>
	Vector3 IsHoming();

	/// <summary>
	/// プレイヤーの方向を向く
	/// </summary>
	void SetToPlayerDirection();
};