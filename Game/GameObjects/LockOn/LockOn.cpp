#include "LockOn.h"
#include "GameObjects/Player/Player.h"
#include "GameObjects/Enemy/BaseEnemy.h"
#include "Managers/ImGui/ImGuiManager.h"

LockOn::LockOn() {
}

LockOn::~LockOn() {
	ClearMultiLockOnSprites();
}

void LockOn::Initialize(DirectXCommon* dxCommon) {
	directXCommon_ = dxCommon;

	// ロックオンマーク用のスプライト初期化
	lockOnMark_ = std::make_unique<Sprite>();
	lockOnMark_->Initialize(
		dxCommon,
		"reticle",                      // テクスチャ名
		{ 640.0f, 360.0f },             // 画面中央
		{ 64.0f, 64.0f }                // サイズ
	);
	// ロックオンマークは赤色に設定
	lockOnMark_->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f });

	// ビューポート行列の計算(画面サイズが変わったら変更)
	matViewport_ = MakeViewportMatrix(0, 0, 1280, 720, 0, 1);
}

void LockOn::Update(Player* player, std::list<std::unique_ptr<BaseEnemy>>& enemies, const Matrix4x4& viewProjectionMatrix) {
	// 現在のターゲットが死んでいる場合は即座に解除
	if (target_ != nullptr && target_->IsDead()) {
		target_ = nullptr;
	}

	// ロックオン対象候補リスト
	// 距離と敵のポインタをペアとして扱う
	std::list<std::pair<float, BaseEnemy*>> targets;

	// プレイヤーのワールド座標を取得
	Vector3 playerPositionWorld = player->GetWorldPosition();

	// ロックオン判定処理
	for (auto& enemy : enemies) {
		// unique_ptrから生ポインタを取得
		BaseEnemy* enemyPtr = enemy.get();

		// 死んだ敵は最初からスキップ
		if (enemyPtr->IsDead()) {
			continue;
		}

		// 敵のワールド座標を取得
		Vector3 enemyPositionWorld = enemyPtr->GetWorldPosition();

		// 簡易的な判定：Z座標で比較（カメラがZ軸負方向を向いている前提）
		if (enemyPositionWorld.z <= playerPositionWorld.z) {
			continue; // プレイヤーより手前（またはカメラの後ろ）にいる敵は除外
		}

		// ワールドからスクリーン座標に変換
		Vector3 positionScreen = ConvertWorldToScreenPosition(enemyPositionWorld, viewProjectionMatrix);
		Vector2 positionScreenV2(positionScreen.x, positionScreen.y);

		// プレイヤーの2Dレティクル位置を取得
		Vector2 player2DReticlePos = player->GetPosition2DReticle();

		// スプライトの中心からの距離
		float distance = Distance(player2DReticlePos, positionScreenV2);

		// 2Dレティクルからのスクリーン距離が規定範囲内のとき
		if (distance <= kDistanceLockOn) {
			// ロックオン対象リストに追加
			targets.emplace_back(std::make_pair(distance, enemyPtr));
		}
	}

	// 一時的にロックオンを解除
	target_ = nullptr;

	// 対象を絞り込んで座標設定する
	if (!targets.empty()) {
		// 距離で昇順にソート（近い順）
		targets.sort([](const std::pair<float, BaseEnemy*>& a, const std::pair<float, BaseEnemy*>& b) {
			return a.first < b.first;
			});

		// 一番近い敵をロックオン対象に設定
		target_ = targets.front().second;

		// ロックオンマークの位置を設定
		Vector3 targetPosition = target_->GetWorldPosition();
		Vector3 positionScreen = ConvertWorldToScreenPosition(targetPosition, viewProjectionMatrix);
		lockOnMark_->SetPosition(Vector2(positionScreen.x, positionScreen.y));
	}
}

void LockOn::DrawUI(const Matrix4x4& viewProjectionMatrixSprite) {
	// ロックオン対象がいたら描画
	if (target_ == nullptr) {
		return; // ロックオン対象がいない場合は描画しない
	}

	// スプライトの更新を行ってから描画
	lockOnMark_->Update(viewProjectionMatrixSprite);
	lockOnMark_->Draw();
}

void LockOn::UpdateMultiLockOn(Player* player, std::list<std::unique_ptr<BaseEnemy>>& enemies, const Matrix4x4& viewProjectionMatrix, std::list<BaseEnemy*>& multiLockOnTargets) {
	// 自機のワールド座標を取得する
	Vector3 playerPositionWorld = player->GetWorldPosition();

	// 死んだ敵、または自機より後ろに行った敵をマルチロックオンリストから除去
	multiLockOnTargets.remove_if([&viewProjectionMatrix, &playerPositionWorld](BaseEnemy* enemy) {
		// 死んだ敵は除去
		if (enemy == nullptr || enemy->IsDead()) {
			return true;
		}

		// 敵のワールド座標を取得
		Vector3 enemyPositionWorld = enemy->GetWorldPosition();

		// 簡易的な奥行き判定：Z座標で比較（カメラがZ軸負方向を向いている前提）
		if (enemyPositionWorld.z <= playerPositionWorld.z) {
			return true; // 自機より後ろに行った敵は除去
		}

		return false; // 除去しない
		});

	// 2Dレティクルの位置を取得
	Vector2 reticlePosition = player->GetPosition2DReticle();

	// マルチロックオン判定処理
	for (auto& enemy : enemies) {
		BaseEnemy* enemyPtr = enemy.get();

		// 死んだ敵は最初からスキップ
		if (enemyPtr->IsDead()) {
			continue;
		}

		// 敵のワールド座標を取得
		Vector3 enemyPositionWorld = enemyPtr->GetWorldPosition();

		// 自機と敵の奥行きを比較して、自機より手前にいる場合は除外
		if (enemyPositionWorld.z <= playerPositionWorld.z) {
			continue;
		}

		// ワールドからスクリーン座標に変換
		Vector3 positionScreen = ConvertWorldToScreenPosition(enemyPositionWorld, viewProjectionMatrix);
		Vector2 enemyScreenPos(positionScreen.x, positionScreen.y);

		// 2Dレティクルと敵の距離を計算
		float distance = Distance(reticlePosition, enemyScreenPos);

		// 既にロックオンされているかチェック
		bool alreadyLocked = false;
		for (BaseEnemy* lockedEnemy : multiLockOnTargets) {
			if (lockedEnemy == enemyPtr) {
				alreadyLocked = true;
				break;
			}
		}

		// 2Dレティクルからの距離が規定範囲内で、まだロックオンされていない場合
		if (distance <= kMultiLockOnDistance && !alreadyLocked) {
			// マルチロックオンリストに追加
			multiLockOnTargets.push_back(enemyPtr);
		}
	}
}

void LockOn::DrawMultiLockOnUI(const std::list<BaseEnemy*>& multiLockOnTargets, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewProjectionMatrixSprite) {
	// 既存のマルチロックオンスプライトをクリア
	ClearMultiLockOnSprites();

	// マルチロックオン対象がいる場合のみ描画
	if (multiLockOnTargets.empty()) {
		return;
	}

	// 各ロックオン対象にマークを描画
	for (BaseEnemy* target : multiLockOnTargets) {
		if (target != nullptr && !target->IsDead()) {
			// 敵のワールド座標を取得
			Vector3 targetPosition = target->GetWorldPosition();
			// ワールドからスクリーン座標に変換
			Vector3 positionScreen = ConvertWorldToScreenPosition(targetPosition, viewProjectionMatrix);

			// マルチロックオン用のスプライトを作成
			auto multiLockMark = CreateMultiLockOnSprite(Vector2(positionScreen.x, positionScreen.y));
			multiLockOnMarks_.push_back(std::move(multiLockMark));
		}
	}

	// 全てのマルチロックオンマークを描画
	for (auto& mark : multiLockOnMarks_) {
		mark->Update(viewProjectionMatrixSprite);
		mark->Draw();
	}
}

std::unique_ptr<Sprite> LockOn::CreateMultiLockOnSprite(const Vector2& position) {
	// マルチロックオン用のスプライトを作成（色を変えて区別）
	auto multiLockMark = std::make_unique<Sprite>();
	multiLockMark->Initialize(
		directXCommon_,
		"reticle",                      // テクスチャ名
		position,                       // 座標
		{ 64.0f, 64.0f }               // サイズ
	);
	// マルチロックオンマークは緑色に設定
	multiLockMark->SetColor({ 0.0f, 1.0f, 0.0f, 1.0f });
	return multiLockMark;
}

void LockOn::ClearMultiLockOnSprites() {
	// 全てのマルチロックオンスプライトを削除
	multiLockOnMarks_.clear();
}