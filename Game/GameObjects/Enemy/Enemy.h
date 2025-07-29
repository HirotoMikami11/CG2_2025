#pragma once
#include <memory>
#include <list>

#include "Objects/GameObject/GameObject.h"
#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "Objects/Light/Light.h"
#include "Engine.h"

#include "State/BaseEnemyState.h"
#include "State/EnemyStateApproach.h"
#include "State/EnemyStateLeave.h"

#include "GameObjects/EnemyBullet/EnemyBullet.h"
#include "MyMath/TimedCall.h"
#include "GameObjects/Collider.h"	//衝突判定


// プレイヤークラスの前方宣言
class Player;

/// <summary>
/// 敵クラス
/// </summary>
class Enemy : public Collider {
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
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="position">初期位置</param>
	void Initialize(DirectXCommon* dxCommon, const Vector3& position);

	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	void Update(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="directionalLight">平行光源</param>
	void Draw(const Light& directionalLight);

	/// <summary>
	/// ImGui
	/// </summary>
	void ImGui();

	/// <summary>
	/// 状態変更
	/// </summary>
	/// <param name="state">ステート</param>
	void ChangeState(std::unique_ptr<BaseEnemyState> state);

	/// <summary>
	/// 弾丸発射
	/// </summary>
	void Fire();

	/// <summary>
	/// 敵の弾丸を削除
	/// </summary>
	void DeleteBullets();

	/// <summary>
	/// タイマーを使って、一定間隔で弾丸を発射する関数
	/// </summary>
	void AutoFire();

	/// <summary>
	/// 自動発射を開始するフラグ
	/// </summary>
	void StartAutoFire();

	/// <summary>
	/// 時限発動の更新、終了した者の削除
	/// </summary>
	void UpdateTimedCalls();

	/// <summary>
	/// 時限発動をクリアする
	/// </summary>
	void ClearTimeCalls();

	// Getter
	Vector3 GetPosition() const { return gameObject_->GetPosition(); }
	Vector3 GetVelocity() const { return velocity_; }
	Vector3 GetWorldPosition() override;

	/// <summary>
	/// 衝突時に呼ばれる関数（オーバーライド）
	/// </summary>
	void OnCollision() override;

	// Setter
	void SetPosition(const Vector3& position) { gameObject_->SetPosition(position); }
	void SetPlayer(Player* player) { player_ = player; }

	/// <summary>
	/// 弾リストを取得
	/// </summary>
	const std::list<std::unique_ptr<EnemyBullet>>& GetBullets() const { return bullets_; }

	// 発射間隔
	static const int kFireInterval_ = 60; // 発射間隔(フレーム数)

private:
	// ゲームオブジェクト
	std::unique_ptr<Sphere> gameObject_;

	// 速度
	Vector3 velocity_ = { 0.0f, 0.0f, 0.0f };

	// 状態
	std::unique_ptr<BaseEnemyState> state_;

	// 敵の弾丸
	std::list<std::unique_ptr<EnemyBullet>> bullets_;

	// 自動発射をするかどうか
	bool isAutoFire_ = false;

	// 遅延クリア用フラグ
	bool shouldClearTimedCalls_ = false;

	// 時限発動クラス
	std::list<std::unique_ptr<TimedCall>> timedCalls_;

	// プレイヤーの情報
	Player* player_ = nullptr;

	// システム参照
	DirectXCommon* directXCommon_ = nullptr;

	// 弾の速度
	static constexpr float kBulletSpeed = 2.0f;
};