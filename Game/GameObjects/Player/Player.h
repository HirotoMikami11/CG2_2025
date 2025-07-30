#pragma once
#include <algorithm>
#include <list>
#include <memory>
#include "Engine.h"

#include "Objects/GameObject/GameObject.h"
#include "Objects/Sprite/Sprite.h"
#include "GameObjects/PlayerBullet/PlayerBullet.h"
#include "GameObjects/Collider.h"	//衝突判定
#include "CollisionManager/CollisionConfig.h"	//衝突属性のフラグを定義する

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
	/// UI用描画
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
	/// レティクルのワールド座標を取得
	/// </summary>
	/// <returns>レティクルのワールド座標</returns>
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
	/// 親オブジェクトを設定（Transform3Dの親子関係）
	/// KamataEngineと同様の親子関係実装
	/// </summary>
	/// <param name="parent">親のTransform3D</param>
	void SetParent(const Transform3D* parent) {
		if (gameObject_) {
			gameObject_->GetTransform().SetParent(parent);
		}
	}

private:
	// ゲームオブジェクト
	std::unique_ptr<Model3D> gameObject_;

	// プレイヤーの弾リスト
	std::list<std::unique_ptr<PlayerBullet>> bullets_;

	// 入力
	InputManager* input_ = nullptr;

	// システム参照
	DirectXCommon* directXCommon_ = nullptr;

	// レティクル関連
	// 3Dレティクル用のTransform3D
	Transform3D transform3DReticle_;
	// 2Dレティクル用のスプライト
	std::unique_ptr<Sprite> sprite2DReticle_;
	// ビューポート行列
	Matrix4x4 matViewport_;
	Vector3 spritePosition_;
	Vector3 posNear_;
	Vector3 posFar_;

	// 移動制限
	static constexpr float kMoveLimitX = 18.0f; // X軸の移動制限
	static constexpr float kMoveLimitY = 10.0f; // Y軸の移動制限
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

	/// <summary>
	/// 3Dレティクルの更新
	/// </summary>
	void UpdateReticle();

	/// <summary>
	/// 3Dレティクルのワールド座標を2Dレティクルのスクリーン座標に変換
	/// </summary>
	void ConvertWorldToScreenReticle();

	/// <summary>
	/// マウスカーソルのスクリーン座標を3Dレティクルのワールド座標に変換
	/// </summary>
	void ConvertMouseToWorldReticle();

	/// <summary>
	/// ゲームパッドでスクリーン座標を3Dレティクルのワールド座標に変換
	/// </summary>
	void ConvertGamepadToWorldReticle();
};