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
	/// 更新
	/// </summary>
	/// <param name="player">プレイヤー</param>
	/// <param name="enemies">敵のリスト</param>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	void Update(Player* player, std::list<std::unique_ptr<Enemy>>& enemies, const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// UI描画
	/// </summary>
	void DrawUI(const Matrix4x4& viewProjectionMatrixSprite);

	/// <summary>
	/// ターゲットを取得
	/// </summary>
	/// <returns>ロックオン対象の敵（nullptrの場合はロックオンなし）</returns>
	Enemy* GetTarget() const { return target_; }

private:
	// ロックオンマークのスプライト
	std::unique_ptr<Sprite> lockOnMark_;

	// ロック対象
	Enemy* target_ = nullptr;

	// システム参照
	DirectXCommon* directXCommon_ = nullptr;

	// ビューポート行列（座標変換用）
	Matrix4x4 matViewport_;

	// ロックオン距離の限界値（スクリーン座標ベース）
	static constexpr float kDistanceLockOn = 400.0f;

	/// <summary>
	/// ワールド座標をスクリーン座標に変換
	/// </summary>
	/// <param name="worldPosition">ワールド座標</param>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	/// <returns>スクリーン座標</returns>
	Vector3 ConvertWorldToScreenPosition(const Vector3& worldPosition, const Matrix4x4& viewProjectionMatrix);
};