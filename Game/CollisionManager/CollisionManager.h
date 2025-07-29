#pragma once

#include "GameObjects/Collider.h"
#include <list>
#include <memory>
#include "Engine.h"

class CollisionManager {
public:

	/// <summary>
	/// コンストラクタ
	/// </summary>
	CollisionManager();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~CollisionManager();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新処理
	/// リストのクリア、コライダー登録、衝突判定処理を行う
	/// </summary>
	void Update();

	/// <summary>
	/// コライダーを引数で受け取り、リストに登録する関数
	/// </summary>
	/// <param name="collider">登録するコライダー</param>
	void AddCollider(Collider* collider);

	/// <summary>
	/// コライダーリストをクリアする関数
	/// </summary>
	void ClearColliderList();

	/// <summary>
	/// 衝突判定と応答
	/// </summary>
	void CheckAllCollision();

private:
	/// <summary>
	/// コライダー２つの衝突判定と応答
	/// </summary>
	/// <param name="colliderA">コライダーA</param>
	/// <param name="colliderB">コライダーB</param>
	void CheckCollisionPair(Collider* colliderA, Collider* colliderB);

	// コライダーのリスト
	std::list<Collider*> colliders_;
};