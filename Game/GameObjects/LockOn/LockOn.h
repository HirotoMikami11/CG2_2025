#pragma once
#include <memory>
#include <list>
#include <utility>
#include "Engine.h"
#include "Objects/Sprite/Sprite.h"

// 前方宣言
class Player;
class Enemy;

/// <summary>
/// ロックオンシステム
/// </summary>
class LockOn {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	LockOn();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~LockOn();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 更新（通常モード）
	/// </summary>
	/// <param name="player">プレイヤー</param>
	/// <param name="enemies">敵のリスト</param>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	void Update(Player* player, std::list<std::unique_ptr<Enemy>>& enemies, const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// マルチロックオン用の更新処理
	/// </summary>
	/// <param name="player">プレイヤー</param>
	/// <param name="enemies">敵のリスト</param>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	/// <param name="multiLockOnTargets">マルチロックオン対象リスト</param>
	void UpdateMultiLockOn(Player* player, std::list<std::unique_ptr<Enemy>>& enemies, const Matrix4x4& viewProjectionMatrix, std::list<Enemy*>& multiLockOnTargets);

	/// <summary>
	/// UI描画（通常モード）
	/// </summary>
	void DrawUI(const Matrix4x4& viewProjectionMatrixSprite);

	/// <summary>
	/// マルチロックオン用のUI描画
	/// </summary>
	/// <param name="multiLockOnTargets">マルチロックオン対象リスト</param>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	/// <param name="viewProjectionMatrixSprite">スプライト用ビュープロジェクション行列</param>
	void DrawMultiLockOnUI(const std::list<Enemy*>& multiLockOnTargets, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewProjectionMatrixSprite);

	/// <summary>
	/// ターゲットを取得
	/// </summary>
	/// <returns>ロックオン対象の敵（nullptrの場合はロックオンなし）</returns>
	Enemy* GetTarget() const { return target_; }

private:
	// ロックオンマークのスプライト
	std::unique_ptr<Sprite> lockOnMark_;

	// マルチロックオン用のスプライトリスト
	std::list<std::unique_ptr<Sprite>> multiLockOnMarks_;

	// ロック対象
	Enemy* target_ = nullptr;

	// システム参照
	DirectXCommon* directXCommon_ = nullptr;

	// ビューポート行列（座標変換用）
	Matrix4x4 matViewport_;

	// ロックオン距離の限界値（スクリーン座標ベース）
	static constexpr float kDistanceLockOn = 32.0f;
	// マルチロックオン距離の限界値（スクリーン座標ベース）
	static constexpr float kMultiLockOnDistance = 32.0f; // 通常のロックオンより近い距離

	/// <summary>
	/// マルチロックオン用のスプライトを作成
	/// </summary>
	/// <param name="position">位置</param>
	/// <returns>作成されたスプライト</returns>
	std::unique_ptr<Sprite> CreateMultiLockOnSprite(const Vector2& position);

	/// <summary>
	/// マルチロックオン用のスプライトを削除
	/// </summary>
	void ClearMultiLockOnSprites();
};