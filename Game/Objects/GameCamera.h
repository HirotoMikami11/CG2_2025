#pragma once
#include "CameraController/BaseCamera.h"
#include "BaseSystem/DirectXCommon/DirectXCommon.h"

// 前方宣言
class Player;

/// <summary>
/// プレイヤー追従カメラ
/// </summary>
class GameCamera : public BaseCamera {
public:
	/// <summary>
	/// カメラの移動範囲を矩形で設定する
	/// </summary>
	struct Rect {
		float left = 0.0f;
		float right = 1.0f;
		float bottom = 0.0f;
		float top = 1.0f;
	};

	/// <summary>
	/// コンストラクタ
	/// </summary>
	GameCamera();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~GameCamera() override;

	/// <summary>
	/// カメラの初期化
	/// </summary>
	/// <param name="position">初期位置</param>
	/// <param name="rotation">初期回転（デフォルト：{0,0,0}）</param>
	void Initialize(const Vector3& position, const Vector3& rotation = { 0.0f, 0.0f, 0.0f }) override;


	/// <summary>
	/// カメラの更新
	/// </summary>
	void Update() override;

	/// <summary>
	/// ImGui表示
	/// </summary>
	void ImGui() override;

	Matrix4x4 GetViewProjectionMatrix() const override { return viewProjectionMatrix_; }
	Matrix4x4 GetSpriteViewProjectionMatrix() const override { return spriteViewProjectionMatrix_; }
	Vector3 GetPosition() const override { return cameraTransform_.translate; }
	void SetPosition(const Vector3& position) override { cameraTransform_.translate = position; }
	std::string GetCameraType() const override { return "PlayerFollow"; }

	/// <summary>
	/// 追従対象がいない場合は無効
	/// </summary>
	/// <returns>追従対象が設定されていればtrue</returns>
	bool IsActive() const override { return target_ != nullptr; }


	/// <summary>
	/// 追従対象の設定
	/// </summary>
	/// <param name="target">プレイヤー</param>
	void SetTarget(Player* target) { target_ = target; }

	/// <summary>
	/// カメラの移動範囲設定
	/// </summary>
	/// <param name="area">移動可能範囲</param>
	void SetMovableArea(const Rect& area) { movableArea_ = area; }

	/// <summary>
	/// カメラオフセット設定
	/// </summary>
	/// <param name="offset">プレイヤーからのオフセット</param>
	void SetOffset(const Vector3& offset) { targetOffset_ = offset; }

	/// <summary>
	/// 最初にしっかり追従させるためのリセット関数
	/// </summary>
	void Reset();

private:
	Player* target_;												// 追従対象（プレイヤー）
	Vector3Transform cameraTransform_;								// カメラのトランスフォーム
	Matrix4x4 viewProjectionMatrix_, spriteViewProjectionMatrix_;	// ビュープロジェクション行列

	Rect movableArea_;												// カメラの移動範囲
	Vector3 targetPosition_;										// カメラの目標座標
	Vector3 targetOffset_;											// 追従対象（プレイヤー）とカメラの座標の差（オフセット）

	/// 追従対象の各方向へのカメラ移動範囲（画面内に収まるぎりぎり）
	static inline const Rect followMargin_ = { -13.0f, 13.0f, -1.0f, 1.0f };

	/// 座標補間割合（1フレームで目標に近づく割合）
	static inline const float kInterpolationRate_ = (5.0f / 60.0f);

	/// 速度掛け算（プレイヤーの速度に合わせる）
	static inline const float kVelocityBias_ = 20.0f;//速度バイアス



	/// <summary>
	/// プレイヤー追従の更新処理
	/// </summary>
	void UpdateFollow();

	/// <summary>
	/// 3D用行列の更新
	/// </summary>
	void UpdateMatrix();

	/// <summary>
	/// スプライト用行列の更新
	/// </summary>
	void UpdateSpriteMatrix();
};