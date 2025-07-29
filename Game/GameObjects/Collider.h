#pragma once
#include "Engine.h"
#include "CollisionManager/CollisionConfig.h"	//衝突属性のフラグを定義する

/// <summary>
/// 衝突判定オブジェクト
/// </summary>
class Collider {

private:
	// 衝突半径
	float radius_ = 1.0f; // 当たり判定用に半径

	//衝突属性(自分)
	uint32_t collisonAttribute_ = 0xffffffff;
	//衝突マスク(相手)
	uint32_t collisionMask_ = 0xffffffff;

public:
	/// <summary>
	/// 仮想デストラクタ
	/// </summary>
	virtual ~Collider() = default;

	/// <summary>
	/// 衝突時に呼ばれる関数
	/// </summary>
	virtual void OnCollision() = 0;

	/// <summary>
	/// ワールド座標を取得する関数
	/// </summary>
	virtual Vector3 GetWorldPosition() = 0;

	// 半径を取得
	virtual float GetRadius() const { return radius_; }

	// 半径を設定
	virtual void SetRadius(float radius) { radius_ = radius; }

	///衝突属性の取得設定

	//衝突属性(自分)を取得
	uint32_t GetCollisionAttribute() const { return collisonAttribute_; }
	// 衝突属性(自分)を設定
	void SetCollisionAttribute(uint32_t attribute) { collisonAttribute_ = attribute; }

	// 衝突マスク(相手)を取得
	uint32_t GetCollisionMask() const { return collisionMask_; }
	// 衝突マスク(相手)を設定
	void SetCollisionMask(uint32_t mask) { collisionMask_ = mask; }
};