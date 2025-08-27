#pragma once
#include "Objects/GameObject/GameObject.h"
#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "GameObjects/Collider.h"	//衝突判定
#include "CollisionManager/CollisionConfig.h"	//衝突属性のフラグを定義する

/// <summary>
/// プレイヤーの弾クラス
/// </summary>
class PlayerBullet : public Collider {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	PlayerBullet();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~PlayerBullet();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="position">初期位置</param>
	/// <param name="velocity">速度</param>
	void Initialize(DirectXCommon* dxCommon, const Vector3& position, const Vector3& velocity);

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
	bool IsDead() const {  return Collider::IsDead() || deathTimer_ <= 0; ; }

	/// <summary>
	/// ワールド座標を取得（オーバーライド）
	/// </summary>
	/// <returns>ワールド座標</returns>
	Vector3 GetWorldPosition() override;

	/// <summary>
	/// 衝突時に呼ばれる関数（オーバーライド）
	/// </summary>
	/// <param name="other">衝突相手のコライダー</param>
	void OnCollision(Collider* other) override;



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

	// 寿命
	static const int32_t kLifeTime_ = 60 * 5; // 弾の寿命（フレーム数）
	int32_t deathTimer_ = kLifeTime_; // 弾の寿命タイマー

	// HP
	int hp_ = 1; // 弾のHP（デフォルト1）

	// システム参照
	DirectXCommon* directXCommon_ = nullptr;

	/// <summary>
	/// 速度の方向を向く
	/// </summary>
	void SetToVelocityDirection();
};