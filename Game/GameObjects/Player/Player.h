#pragma once
#include <algorithm>
#include <list>
#include <memory>
#include "Engine.h"

#include "Objects/GameObject/GameObject.h"
#include "GameObjects/PlayerBullet/PlayerBullet.h"

/// <summary>
/// プレイヤークラス
/// </summary>
class Player {
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
	void Initialize(DirectXCommon* dxCommon);

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
	/// ワールド座標を取得
	/// </summary>
	/// <returns>ワールド座標</returns>
	Vector3 GetWorldPosition() const;

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

private:
	// ゲームオブジェクト
	std::unique_ptr<Sphere> gameObject_;

	// プレイヤーの弾リスト
	std::list<std::unique_ptr<PlayerBullet>> bullets_;

	// 入力
	InputManager* input_ = nullptr;

	// システム参照
	DirectXCommon* directXCommon_ = nullptr;

	// 移動制限
	static constexpr float kMoveLimitX = 33.0f; // X軸の移動制限
	static constexpr float kMoveLimitY = 18.0f; // Y軸の移動制限
	static constexpr float kCharacterSpeed = 0.2f; // 移動速度
	static constexpr float kRotSpeed = 0.02f; // 回転速度
	static constexpr float kBulletSpeed = 1.0f; // 弾の速度

	/// <summary>
	/// 移動処理
	/// </summary>
	void Move();

	/// <summary>
	/// 回転処理
	/// </summary>
	void Rotate();

	/// <summary>
	/// 攻撃処理
	/// </summary>
	void Attack();

	/// <summary>
	/// 寿命の尽きた弾を削除する
	/// </summary>
	void DeleteBullets();
};