#pragma once

#include "Engine.h"
#include "Objects/GameObject/GameObject.h"
#include <numbers>

// 前方宣言
class Player;

/// <summary>
/// 敵キャラクター
/// </summary>
class Enemy {
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
	/// <param name="position">初期位置</param>
	void Initialize(const Vector3& position);

	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	void Update(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="directionalLight">ライト</param>
	void Draw(const Light& directionalLight);

	/// <summary>
	/// ImGui描画
	/// </summary>
	void ImGui();

	/// <summary>
	/// 移動処理
	/// </summary>
	void IsMove();

	/// <summary>
	/// アニメーション処理
	/// </summary>
	void IsAnimation();

	/// <summary>
	/// プレイヤーとの衝突処理
	/// </summary>
	/// <param name="player">プレイヤー</param>
	void OnCollision(const Player* player);

	// Getter
	/// <summary>
	/// AABB取得
	/// </summary>
	/// <returns>AABB</returns>
	AABB GetAABB() const;

	/// <summary>
	/// ワールド座標取得
	/// </summary>
	/// <returns>ワールド座標</returns>
	Vector3 GetWorldPosition() const;

	/// <summary>
	/// 位置取得
	/// </summary>
	/// <returns>位置</returns>
	Vector3 GetPosition() const { return Object_->GetPosition(); }

	// Setter
	/// <summary>
	/// 位置設定
	/// </summary>
	/// <param name="position">位置</param>
	void SetPosition(const Vector3& position) { Object_->SetPosition(position); }

private:
	// 敵オブジェクト
	std::unique_ptr<Model3D> Object_ = nullptr;

	// 速度
	Vector3 velocity_ = {};
	// 歩行の速さ
	static inline const float kWalkSpeed = 0.03f;

	// 当たり判定サイズ
	static inline const float kWidth = 1.0f;
	static inline const float kHeight = 1.0f;

	/// アニメーション設定
	// 最初の角度[度]
	static inline const float kWalkMotionAngleStart = -20.0f;
	// 最後の角度[度]
	static inline const float kWalkMotionAngleEnd = 40.0f;
	// アニメーションの周期となる時間
	static inline const float kWalkMotionTime = 1.0f;
	// 経過時間
	float walkTimer_ = 0.0f;

	// システム参照
	DirectXCommon* directXCommon_;
};