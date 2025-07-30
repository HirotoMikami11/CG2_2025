#include "LockOn.h"
#include "GameObjects/Player/Player.h"
#include "GameObjects/Enemy/Enemy.h"
#include "Managers/ImGui/ImGuiManager.h"

LockOn::LockOn() {
}

LockOn::~LockOn() = default;

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

void LockOn::Update(Player* player, std::list<std::unique_ptr<Enemy>>& enemies, const Matrix4x4& viewProjectionMatrix) {
	// 現在のターゲットが死んでいる場合は即座に解除
	if (target_ != nullptr && target_->IsDead()) {
		target_ = nullptr;
	}

	// ロックオン対象候補リスト
	// 距離と敵のポインタをペアとして扱う
	std::list<std::pair<float, Enemy*>> targets;

	// プレイヤーのワールド座標を取得
	Vector3 playerPositionWorld = player->GetWorldPosition();

	// ロックオン判定処理
	for (auto& enemy : enemies) {
		// unique_ptrから生ポインタを取得
		Enemy* enemyPtr = enemy.get();

		// 死んだ敵は最初からスキップ
		if (enemyPtr->IsDead()) {
			continue;
		}

		// 敵のワールド座標を取得
		Vector3 enemyPositionWorld = enemyPtr->GetWorldPosition();

		// プレイヤーから敵への方向ベクトルを計算
		Vector3 playerToEnemy = enemyPositionWorld - playerPositionWorld;

		// カメラの前方向（通常はZ+方向）との内積で奥行きを判定
		// プレイヤーよりも前（カメラから遠い）にいる敵のみを対象とする
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
		float dx = player2DReticlePos.x - positionScreenV2.x;
		float dy = player2DReticlePos.y - positionScreenV2.y;
		float distance = std::sqrt(dx * dx + dy * dy);

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
		targets.sort([](const std::pair<float, Enemy*>& a, const std::pair<float, Enemy*>& b) {
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

Vector3 LockOn::ConvertWorldToScreenPosition(const Vector3& worldPosition, const Matrix4x4& viewProjectionMatrix) {
	// ビュープロジェクション行列とビューポート行列を合成
	Matrix4x4 matViewProjectionViewport = Matrix4x4Multiply(viewProjectionMatrix, matViewport_);

	// ワールドからスクリーンに座標変換（3Dから2Dになる）
	Vector3 screenPosition = Matrix4x4Transform(worldPosition, matViewProjectionViewport);

	return screenPosition;
}