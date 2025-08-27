#pragma once
#define NOMINMAX
#include <algorithm>
#include "Engine.h"
#include "CollisionManager/CollisionConfig.h"  // 衝突属性のフラグを定義


/// <summary>
/// 衝突判定オブジェクト
/// </summary>
class Collider {

private:
	// 衝突半径
	float radius_ = 1.0f; // 当たり判定用に半径

	// 衝突属性(自分)
	uint32_t collisonAttribute_ = 0xffffffff;
	// 衝突マスク(相手)
	uint32_t collisionMask_ = 0xffffffff;

	// 攻撃力とHP
	float attackPower_ = 1.0f;  // デフォルト攻撃力
	float maxHP_ = 1.0f;        // 最大HP
	float currentHP_ = 1.0f;    // 現在のHP

public:
	/// <summary>
	/// 仮想デストラクタ
	/// </summary>
	virtual ~Collider() = default;

	/// <summary>
	/// 衝突時に呼ばれる関数（相手のコライダー情報を受け取る）
	/// </summary>
	virtual void OnCollision(Collider* other) = 0;

	/// <summary>
	/// ワールド座標を取得する関数
	/// </summary>
	virtual Vector3 GetWorldPosition() = 0;

	// 半径を取得
	virtual float GetRadius() const { return radius_; }

	// 半径を設定
	virtual void SetRadius(float radius) { radius_ = radius; }

	/// 衝突属性の取得設定

	// 衝突属性(自分)を取得
	uint32_t GetCollisionAttribute() const { return collisonAttribute_; }
	// 衝突属性(自分)を設定
	void SetCollisionAttribute(uint32_t attribute) { collisonAttribute_ = attribute; }

	// 衝突マスク(相手)を取得
	uint32_t GetCollisionMask() const { return collisionMask_; }
	// 衝突マスク(相手)を設定
	void SetCollisionMask(uint32_t mask) { collisionMask_ = mask; }

	// 攻撃力の取得・設定
	float GetAttackPower() const { return attackPower_; }
	void SetAttackPower(float attackPower) { attackPower_ = attackPower; }

	// HP関連
	float GetMaxHP() const { return maxHP_; }
	float GetCurrentHP() const { return currentHP_; }
	void SetMaxHP(float maxHP) {
		maxHP_ = maxHP;
		currentHP_ = maxHP; // 初期化時は最大HPに設定
	}
	void SetCurrentHP(float currentHP) { currentHP_ = currentHP; }

	/// <summary>
	/// ダメージを受ける（基本実装）
	/// </summary>
	/// <param name="damage">ダメージ量</param>
	/// <returns>実際に受けたダメージ量</returns>
	virtual float TakeDamage(float damage) {
		float actualDamage = (std::min)(damage, currentHP_);//NOINMAXで解決できなかったので、()で囲む
		currentHP_ -= actualDamage;
		currentHP_ = (std::max)(0.0f, currentHP_);
		return actualDamage;
	}
	/// <summary>
	/// 死亡しているかどうか
	/// </summary>
	virtual bool IsDead() const { return currentHP_ <= 0.0f; }
};
