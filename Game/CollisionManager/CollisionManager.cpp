#include "CollisionManager.h"

CollisionManager::CollisionManager() {}

CollisionManager::~CollisionManager() {}

void CollisionManager::Initialize() {
	// 初期化処理
	colliders_.clear();	//一応リストのクリア
}

void CollisionManager::Update() {
	// ここでは登録されたコライダーの衝突判定のみを実行
	CheckAllCollision();
}

void CollisionManager::AddCollider(Collider* collider) {
	if (collider != nullptr) {
		colliders_.push_back(collider);
	}
}

void CollisionManager::ClearColliderList() {
	colliders_.clear();
}

void CollisionManager::CheckAllCollision() {
	// リスト内のペアを総当たり
	std::list<Collider*>::iterator itrA = colliders_.begin();
	for (; itrA != colliders_.end(); ++itrA) {
		// イテレータAからコライダーAを取得する
		Collider* colliderA = *itrA;
		// イテレータBはイテレータAの次の要素から回して重複判定を回避
		std::list<Collider*>::iterator itrB = itrA;
		itrB++;

		for (; itrB != colliders_.end(); ++itrB) {
			// イテレータBからコライダーBを取得する
			Collider* colliderB = *itrB;
			// コライダーA,Bの衝突判定と応答
			CheckCollisionPair(colliderA, colliderB);
		}
	}
}

void CollisionManager::CheckCollisionPair(Collider* colliderA, Collider* colliderB) {
	/// 判定対象A,Bの座標
	Vector3 posA, posB;

	posA = colliderA->GetWorldPosition();
	posB = colliderB->GetWorldPosition();

	float length = Length(posA - posB);

	// 衝突フィルタリング
	// コライダーAとコライダーBで衝突属性とマスクを確認
	if ((colliderA->GetCollisionAttribute() & colliderB->GetCollisionMask()) == 0 ||
		(colliderB->GetCollisionAttribute() & colliderA->GetCollisionMask()) == 0) {
		// 衝突しない場合は何もしない
		return;
	}

	// 球と球の交差判定
	if (length < colliderA->GetRadius() + colliderB->GetRadius()) {
		// コールバック関数を呼び出す
		colliderA->OnCollision();
		colliderB->OnCollision();
	}
}