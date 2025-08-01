#pragma once
#include <algorithm>
#include <list>
#include <memory>
#include "Engine.h"

#include "Objects/GameObject/GameObject.h"
#include "Objects/Sprite/Sprite.h" // レティクル用スプライト
#include "GameObjects/PlayerBullet/PlayerBullet.h"
#include "GameObjects/PlayerBullet/PlayerHomingBullet.h"
#include "GameObjects/Enemy/Enemy.h"
#include "GameObjects/Collider.h"	//衝突判定
#include "CollisionManager/CollisionConfig.h"	//衝突属性のフラグを定義する
#include "CameraController/CameraController.h" // カメラコントローラー

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
	/// 衝突時に呼ばれる関数（オーバーライド）
	/// </summary>
	void OnCollision() override;

	/// <summary>
	/// 位置を取得
	/// </summary>
	/// <returns>位置</returns>
	Vector3 GetPosition() const { return gameObject_->GetPosition(); }

	/// <summary>
	/// 位置を設定
	/// </summary>
	/// <param name="position">位置</param>
	void SetPosition(const Vector3& position) { gameObject_->SetPosition(position); }

	/// <summary>
	/// 弾リストを取得
	/// </summary>
	const std::list<std::unique_ptr<PlayerBullet>>& GetBullets() const { return bullets_; }
	/// <summary>
	/// ホーミング弾リストを取得
	/// </summary>
	const std::list<std::unique_ptr<PlayerHomingBullet>>& GetHomingBullets() const { return homingBullets_; }

	/// <summary>
	/// 2Dレティクルの位置を取得
	/// </summary>
	/// <returns>2Dレティクルのスクリーン座標</returns>
	Vector2 GetPosition2DReticle() const {
		if (sprite2DReticle_) {
			return sprite2DReticle_->GetPosition();
		}
		return Vector2{ 640.0f, 360.0f }; // デフォルト位置
	}

	/// <summary>
	/// 親オブジェクトを設定（Transform3Dの親子関係）
	/// KamataEngineと同様の親子関係実装
	/// </summary>
	/// <param name="parent">親のTransform3D</param>
	void SetParent(const Transform3D* parent) {
		if (gameObject_) {
			gameObject_->GetTransform().SetParent(parent);
		}
	}

	/// <summary>
	/// スプライト用ビュープロジェクション行列を設定
	/// </summary>
	/// <param name="viewProjectionMatrixSprite">スプライト用ビュープロジェクション行列</param>
	void SetViewProjectionMatrixSprite(const Matrix4x4& viewProjectionMatrixSprite) {
		viewProjectionMatrixSprite_ = viewProjectionMatrixSprite;
	}
	/// <summary>
	/// ロックオンシステムを設定
	/// </summary>
	/// <param name="lockOn">ロックオンシステム</param>
	void SetLockOn(LockOn* lockOn) { lockOn_ = lockOn; }

	/// <summary>
	/// 現在の攻撃モードがマルチロックオンかどうか
	/// </summary>
	/// <returns>true: マルチロックオンモード, false: 通常モード</returns>
	bool IsMultiLockOnMode() const { return attackMode_ == AttackMode::MultiLockOn; }

	/// <summary>
	/// 現在の攻撃モードを取得
	/// </summary>
	/// <returns>現在の攻撃モード</returns>
	AttackMode GetAttackMode() const { return attackMode_; }

	/// <summary>
	/// マルチロックオンターゲットリストを取得
	/// </summary>
	/// <returns>マルチロックオンターゲットリスト</returns>
	std::list<Enemy*>& GetMultiLockOnTargets() { return multiLockOnTargets_; }

	/// <summary>
	/// ダメージを受ける
	/// </summary>
	/// <param name="damage">ダメージ量</param>
	void TakeDamage(float damage);

	/// <summary>
	/// 現在のHPを取得
	/// </summary>
	/// <returns>現在のHP</returns>
	float GetCurrentHP() const { return currentHP_; }

	/// <summary>
	/// 最大HPを取得
	/// </summary>
	/// <returns>最大HP</returns>
	float GetMaxHP() const { return maxHP_; }

	/// <summary>
	/// 現在のENを取得
	/// </summary>
	/// <returns>現在のEN</returns>
	float GetCurrentEN() const { return currentEN_; }

	/// <summary>
	/// 最大ENを取得
	/// </summary>
	/// <returns>最大EN</returns>
	float GetMaxEN() const { return maxEN_; }

private:
	// ゲームオブジェクト
	std::unique_ptr<Model3D> gameObject_;

	// 3Dレティクル用のゲームオブジェクト
	std::unique_ptr<Model3D> reticle3D_;

	// 2Dレティクル用のスプライト
	std::unique_ptr<Sprite> sprite2DReticle_;

	// プレイヤーの弾リスト
	std::list<std::unique_ptr<PlayerBullet>> bullets_;
	// プレイヤーのホーミング弾リスト
	std::list<std::unique_ptr<PlayerHomingBullet>> homingBullets_;

	// 入力
	InputManager* input_ = nullptr;

	// システム参照
	DirectXCommon* directXCommon_ = nullptr;

	// ビュープロジェクション行列（スプライト用）
	Matrix4x4 viewProjectionMatrixSprite_;

	// ビューポート行列（座標変換用）
	Matrix4x4 matViewport_;

	///ゲームパッド用のNearFar座標
	Vector3 posNear_;
	Vector3 posFar_;
	Vector3 spritePosition_;

	// ロックオンシステム
	LockOn* lockOn_ = nullptr;

	/// 攻撃モード関連
	AttackMode attackMode_ = AttackMode::Normal; // 攻撃モード（デフォルトは通常モード）
	std::list<Enemy*> multiLockOnTargets_;       // マルチロックオン対象リスト

	// 発射間隔
	static constexpr float kFireInterval = 10.0f;   // 発射間隔（フレーム数）
	float fireTimer_ = kFireInterval;              // 発射タイマー

	// 移動制限
	static constexpr float kMoveLimitX = 18.0f; // X軸の移動制限
	static constexpr float kMoveLimitY = 10.0f; // Y軸の移動制限
	static constexpr float kCharacterSpeed = 0.4f; // 移動速度
	static constexpr float kRotSpeed = 0.02f; // 回転速度
	static constexpr float kBulletSpeed = 1.0f; // 弾の速度
	//キーボードの時は自キャラからの距離
	static constexpr float kDistancePlayerTo3DReticleKeyborad = 20.0f; // プレイヤーから3Dレティクルまでの距離(キーボードの時)
	// ゲームパッドの時の距離は、カメラからの距離なので、カメラと自キャラの距離を加算した値
	static constexpr float kDistancePlayerTo3DReticleGamepad = 50.0f; // プレイヤーから3Dレティクルまでの距離(ゲームパッドの時)

	// HP/ENシステム
	float maxHP_ = 100.0f;           // 最大HP（酸素）
	float currentHP_ = 100.0f;       // 現在のHP
	float maxEN_ = 100.0f;           // 最大EN（エネルギー）
	float currentEN_ = 100.0f;       // 現在のEN
	float energyCostPerShot_ = 0.5f; // 1発撃つごとのエネルギー消費量
	float energyRegenRate_ = 1.0f;   // エネルギー回復速度（毎フレーム）
	int energyRegenDelay_ = 60;      // エネルギー回復開始までの待機時間（フレーム）
	int energyRegenTimer_ = 0;       // エネルギー回復タイマー

	// HP/ENゲージ用スプライト
	std::unique_ptr<Sprite> hpGaugeBar_;      // HPゲージの枠
	std::unique_ptr<Sprite> hpGaugeFill_;     // HPゲージの中身
	std::unique_ptr<Sprite> enGaugeBar_;      // ENゲージの枠
	std::unique_ptr<Sprite> enGaugeFill_;     // ENゲージの中身

	/// <summary>
	/// 移動処理
	/// </summary>
	void Move();

	/// <summary>
	/// 回転処理（従来版）
	/// </summary>
	void Rotate();

	/// <summary>
	/// レールカメラ対応回転処理
	/// </summary>
	void RotateWithRailCamera();

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
	/// 3Dレティクルの更新処理
	/// </summary>
	void UpdateReticle(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// ゲームパッドでスクリーン座標を3Dレティクルのワールド座標に変換
	/// </summary>
	/// <param name="camera"></param>
	void ConvertGamePadToWorldReticle(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// キーボードでワールド座標に変換されたレティクルを2Dスクリーン座標に変換
	/// </summary>
	/// <param name="camera"></param>
	void ConvertKeyboardToWorldReticle(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// 3Dレティクルのワールド座標を2Dレティクルのスクリーン座標に変換
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	void ConvertWorldToScreenReticle(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// 寿命の尽きたホーミング弾を削除する
	/// </summary>
	void DeleteHomingBullets();

	/// <summary>
	/// エネルギーの更新処理
	/// </summary>
	void UpdateEnergy();

	/// <summary>
	/// HP/ENゲージの初期化
	/// </summary>
	void InitializeGauges();

	/// <summary>
	/// HP/ENゲージの更新
	/// </summary>
	void UpdateGauges();

	/// <summary>
	/// 偏差射撃の計算
	/// </summary>
	/// <param name="enemyPos">敵の位置</param>
	/// <param name="enemyVelocity">敵の速度</param>
	/// <param name="bulletSpeed">弾の速度</param>
	/// <returns>予測位置</returns>
	Vector3 CalculateLeadingShot(const Vector3& enemyPos, const Vector3& enemyVelocity, float bulletSpeed);
};