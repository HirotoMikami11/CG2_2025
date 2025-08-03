#pragma once
#include <memory>
#include "Objects/GameObject/GameObject.h"
#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "Objects/Light/Light.h"
#include "Engine.h"

#include "GameObjects/Collider.h"
#include "CollisionManager/CollisionConfig.h"

// 前方宣言
class BaseEnemy;

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
	/// <param name="target">ターゲットの敵</param>
	void Initialize(DirectXCommon* dxCommon, const Vector3& position, const Vector3& velocity, BaseEnemy* target);

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
	/// ImGui
	/// </summary>
	void ImGui();

	/// <summary>
	/// 衝突時に呼ばれる関数（オーバーライド）
	/// </summary>
	void OnCollision() override;

	/// <summary>
	/// ワールド座標を取得（オーバーライド）
	/// </summary>
	/// <returns>ワールド座標</returns>
	Vector3 GetWorldPosition() override;

	// Getter
	Vector3 GetPosition() const { return gameObject_->GetPosition(); }
	Vector3 GetVelocity() const { return velocity_; }
	bool IsDead() const { return isDead_; }

	// Setter
	void SetPosition(const Vector3& position) { gameObject_->SetPosition(position); }
	void SetVelocity(const Vector3& velocity) { velocity_ = velocity; }

private:
	// ゲームオブジェクト
	std::unique_ptr<Model3D> gameObject_;

	// 速度
	Vector3 velocity_;

	// ターゲット敵
	BaseEnemy* target_ = nullptr;

	// 生存時間
	static const int32_t kLifeTime = 60 * 10; // 60fps
	int32_t deathTimer_ = kLifeTime;

	// 死亡フラグ
	bool isDead_ = false;

	// システム参照
	DirectXCommon* directXCommon_ = nullptr;

	// ホーミング性能
	static constexpr float kHomingStrength = 0.2f; // ホーミングの強さ（0.0f〜1.0f）
	static constexpr float kMaxSpeed = 3.0f;       // 最大速度

	/// <summary>
	/// ホーミング処理
	/// </summary>
	void UpdateHoming();
};