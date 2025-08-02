#pragma once
#include <algorithm>
#include <list>
#include <memory>
#include "Engine.h"

#include "Objects/GameObject/GameObject.h"

#include "GameObjects/PlayerBullet/PlayerBullet.h"
#include "GameObjects/PlayerBullet/PlayerHomingBullet.h"
#include "GameObjects/Enemy/BaseEnemy.h"

#include "GameObjects/Collider.h"	//衝突判定
#include "CollisionManager/CollisionConfig.h"	//衝突属性のフラグを定義する
#include "CameraController/CameraController.h" // カメラコントローラー

// 分離したクラス
#include "PlayerHealth.h"
#include "PlayerUI.h"
#include "PlayerReticle.h"

// 前方宣言
class LockOn;

/// <summary>
/// 攻撃モード
/// </summary>
enum class AttackMode {
	Normal,      // 通常モード
	MultiLockOn  // マルチロックオンモード
};

/// <summary>
/// プレイヤークラス
/// </summary>
class Player : public Collider {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	Player();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~Player();

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
	/// UI描画（レティクル描画）
	/// </summary>
	void DrawUI();

	/// <summary>
	/// ImGui
	/// </summary>
	void ImGui();

	/// <summary>
	/// ワールド座標を取得（オーバーライド）
	/// </summary>
	/// <returns>ワールド座標</returns>
	Vector3 GetWorldPosition() override;

	/// <summary>
	/// 3Dレティクルのワールド座標を取得
	/// </summary>
	/// <returns>3Dレティクルのワールド座標</returns>
	Vector3 GetWorldPosition3DReticle();

	/// <summary>
	/// プレイヤーの前方向ベクトルを取得
	/// </summary>
	/// <returns>前方向ベクトル</returns>
	Vector3 GetForward() const;

	/// <summary>
	/// 衝突時に呼ばれる関数（オーバーライド）
	/// </summary>
	void OnCollision() override;

	//Getter
	Vector3 GetPosition() const { return gameObject_->GetPosition(); }
	Vector2 GetPosition2DReticle() const { return reticle_->GetPosition2DReticle(); }	// 2Dレティクルの位置を取得

	const std::list<std::unique_ptr<PlayerBullet>>& GetBullets() const { return bullets_; }	// 弾リストを取得
	const std::list<std::unique_ptr<PlayerHomingBullet>>& GetHomingBullets() const { return homingBullets_; }	// ホーミング弾リストを取得

	void SetPosition(const Vector3& position) { gameObject_->SetPosition(position); }
	// 親オブジェクトを設定（Transform3Dの親子関係）
	void SetParent(const Transform3D* parent) { if (gameObject_) { gameObject_->GetTransform().SetParent(parent); } }
	// スプライト用ビュープロジェクション行列を設定
	void SetViewProjectionMatrixSprite(const Matrix4x4& viewProjectionMatrixSprite) {
		viewProjectionMatrixSprite_ = viewProjectionMatrixSprite;
	}
	void SetLockOn(LockOn* lockOn) { lockOn_ = lockOn; }	// ロックオンシステムを設定
	bool IsMultiLockOnMode() const { return attackMode_ == AttackMode::MultiLockOn; }	// 現在の攻撃モードがマルチロックオンかどうか
	AttackMode GetAttackMode() const { return attackMode_; }	// 現在の攻撃モードを取得
	std::list<BaseEnemy*>& GetMultiLockOnTargets() { return multiLockOnTargets_; }	// マルチロックオンターゲットリストを取得

	// UIシステムへのアクセス
	PlayerUI& GetUI() { return *ui_; }
	const PlayerUI& GetUI() const { return *ui_; }

	// レティクルシステムへのアクセス
	PlayerReticle& GetReticle() { return *reticle_; }
	const PlayerReticle& GetReticle() const { return *reticle_; }

	// 体力システムへのアクセス
	PlayerHealth& GetHealth() { return *health_; }
	const PlayerHealth& GetHealth() const { return *health_; }
	/// Health用
	void TakeDamage(float damage) { health_->TakeDamage(damage); }	// ダメージを受ける
	float GetCurrentHP() const { return health_->GetCurrentHP(); }// 現在のHPを取得
	float GetMaxHP() const { return health_->GetMaxHP(); }	// 最大HPを取得
	float GetCurrentEN() const { return health_->GetCurrentEN(); }// 現在のENを取得
	float GetMaxEN() const { return health_->GetMaxEN(); }	// 最大ENを取得

private:
	// ゲームオブジェクト
	std::unique_ptr<Model3D> gameObject_;

	// プレイヤーの弾リスト
	std::list<std::unique_ptr<PlayerBullet>> bullets_;
	// プレイヤーのホーミング弾リスト
	std::list<std::unique_ptr<PlayerHomingBullet>> homingBullets_;

	// 分離したシステム
	std::unique_ptr<PlayerHealth> health_;		 // 体力・エネルギー管理
	std::unique_ptr<PlayerUI> ui_;				 // UI管理
	std::unique_ptr<PlayerReticle> reticle_;	 // レティクル管理

	// 入力
	InputManager* input_ = nullptr;

	// システム参照
	DirectXCommon* directXCommon_ = nullptr;

	// ビュープロジェクション行列（スプライト用）
	Matrix4x4 viewProjectionMatrixSprite_;

	// ロックオンシステム
	LockOn* lockOn_ = nullptr;

	/// 攻撃モード関連
	AttackMode attackMode_ = AttackMode::Normal;	// 攻撃モード（デフォルトは通常モード）
	std::list<BaseEnemy*> multiLockOnTargets_;		// マルチロックオン対象リスト

	// 発射間隔
	static constexpr float kFireInterval = 10.0f;	// 発射間隔（フレーム数）
	float fireTimer_ = kFireInterval;				// 発射タイマー

	// 移動制限
	static constexpr float kMoveLimitX = 18.0f; // X軸の移動制限
	static constexpr float kMoveLimitY = 10.0f; // Y軸の移動制限
	static constexpr float kCharacterSpeed = 0.2f; // 移動速度
	static constexpr float kBulletSpeed = 1.0f; // 弾の速度

	/// <summary>
	/// 移動処理
	/// </summary>
	void Move();

	/// <summary>
	/// カメラに背を向ける処理
	/// </summary>
	void FaceAwayFromCamera();

	/// <summary>
	/// 攻撃モード切り替え処理
	/// </summary>
	void SwitchAttackMode();

	/// <summary>
	/// 攻撃処理
	/// </summary>
	void Attack();

	/// <summary>
	/// 通常攻撃
	/// </summary>
	void Fire();

	/// <summary>
	/// マルチロックオン時の発射処理
	/// </summary>
	void FireMultiLockOn();

	/// <summary>
	/// 寿命の尽きた弾を削除する
	/// </summary>
	void DeleteBullets();

	/// <summary>
	/// 寿命の尽きたホーミング弾を削除する
	/// </summary>
	void DeleteHomingBullets();

	/// <summary>
	/// 偏差射撃の計算
	/// </summary>
	/// <param name="enemyPos">敵の位置</param>
	/// <param name="enemyVelocity">敵の速度</param>
	/// <param name="bulletSpeed">弾の速度</param>
	/// <returns>予測位置</returns>
	Vector3 CalculateLeadingShot(const Vector3& enemyPos, const Vector3& enemyVelocity, float bulletSpeed);
};