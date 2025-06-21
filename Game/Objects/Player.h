#pragma once

#include "Engine.h"
#include "Objects/GameObject/GameObject.h"
#include "Objects/MapChipField.h"
#include <algorithm>
#include <numbers>
#include <array>

class MapChipField;

class Player {
private:
	// マップとの当たり判定情報
	struct CollisionMapInfo {
		bool isCeilingHit = false;
		bool isLanding = false;
		bool isWallHit = false;
		Vector3 moveVelocity;
	};

	// 角
	enum Corner {
		kRightBottom, // 右下
		kLeftBottom,  // 左下
		kRightTop,    // 右上
		kLeftTop,     // 左上

		kNumCorner // 要素数
	};

	// 向いている向き
	enum class LRDirection { kRight, kLeft };

public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// 描画
	/// </summary>
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
	/// 旋回処理
	/// </summary>
	void TurningControl();

	/// <summary>
	/// 落下する処理
	/// </summary>
	void IsFall();

	/// <summary>
	/// マップ衝突をチェックする
	/// </summary>
	void CollisonMap(CollisionMapInfo& info);
	void CollisonMapTop(CollisionMapInfo& info);
	void CollisonMapBottom(CollisionMapInfo& info);
	void CollisonMapLeft(CollisionMapInfo& info);
	void CollisonMapRight(CollisionMapInfo& info);

	/// <summary>
	/// 角の座標を求める
	/// </summary>
	Vector3& CornerPosition(const Vector3& center, Corner corner);

	/// <summary>
	/// 判定結果を反映して移動させる
	/// </summary>
	void MoveAfterCollisionCheck(const CollisionMapInfo& info);

	/// <summary>
	/// 天井に接触している場合の処理
	/// </summary>
	void IsCeilingHit(const CollisionMapInfo& info);

	/// <summary>
	/// 壁に接触している場合の処理
	/// </summary>
	void IsWallHit(const CollisionMapInfo& info);

	/// <summary>
	/// 接地状態の切り替え処理
	/// </summary>
	void SwitchGrounding(const CollisionMapInfo& info);

	// Getter
	Vector3 GetPosition() const { return Object_->GetPosition(); }
	const Vector3& GetVelocity() const { return velocity_; }

	// Setter
	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }
	void SetPosition(const Vector3& position) { Object_->SetPosition(position); }

private:
	//プレイヤー
	std::unique_ptr<Model3D> Object_ = nullptr;

	// マップチップによるフィールド
	MapChipField* mapChipField_ = nullptr;

	// 物理パラメータ
	Vector3 velocity_ = {};

	// 加速度
	static inline const float kAcceleration = 0.01f;
	// 最大速度
	static inline const float kLimitRunSpeed = 0.2f;

	// 方向
	LRDirection lrDirection_ = LRDirection::kRight;
	// 旋回開始時の角度
	float turnFirstRotationY_ = 0.0f;
	// 旋回タイマー
	float turnTimer_ = 0.0f;
	// 旋回時間<秒>
	static inline const float kTimeTurn = 0.3f;

	// 接地状態フラグ
	bool onGround_ = true;
	// 重力加速度（下方向）
	static inline const float kGravityAcceleration = 0.05f;
	// 最大落下速度（下方向）
	static inline const float kLimitFallSpeed = 0.3f;
	// ジャンプ初速（上方向）
	static inline const float kJumpAcceleration = 0.7f;
	// 着地していないとき摩擦で横方向速度を減衰させる値
	static inline const float kAttenuation = 0.1f;
	// 着地時速度減衰率
	static inline const float kAttenuationLanding = 0.1f;
	// 下方向の接地判定を取る際に、マージンを埋めるための定数
	static inline const float kGroundCheckOffset = 0.2f;
	// 当たり判定サイズ
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;
	// 余白
	static inline const float kBlank = 0.2f;
	// 壁衝突時の減衰率
	static inline const float kAttenuationWall = 0.2f;

	// システム参照
	DirectXCommon* directXCommon_;
	InputManager* input_;
};