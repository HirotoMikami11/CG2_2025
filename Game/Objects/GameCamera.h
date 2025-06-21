#pragma once

#include "MyMath/MyFunction.h"
#include "CameraController/CameraController.h"

// 前方宣言
class Player;

/// <summary>
/// プレイヤー追従カメラのゲームロジック
/// システム層のCameraControllerを使用してゲーム固有のカメラ制御を行う
/// </summary>
class GameCamera {
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

private:
	// システム層のカメラコントローラー参照
	CameraController* cameraController_ = nullptr;

	// 追従対象（プレイヤー）
	Player* target_ = nullptr;

	// カメラの移動範囲
	Rect movableArea_ = { 0.0f, 100.0f, 0.0f, 100.0f };

	// 追従対象の各方向へのカメラ移動範囲（画面内に収まるぎりぎり）
	static inline const Rect followMargin_ = { -13.0f, 13.0f, -1.0f, 1.0f };

	// カメラの目標座標
	Vector3 targetPosition_ = { 0.0f, 0.0f, 0.0f };

	// 座標補間割合（1フレームで目標に近づく割合）
	static inline const float kInterpolationRate_ = (5.0f / 60.0f);

	// 追従対象（プレイヤー）とカメラの座標の差（オフセット）
	Vector3 targetOffset_ = { 0.0f, 0.0f, -38.0f };

	// 速度掛け算（プレイヤーの速度に合わせる）
	static inline const float kVelocityBias_ = 20.0f;

public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	GameCamera();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~GameCamera();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="cameraController">システム層のカメラコントローラー</param>
	void Initialize(CameraController* cameraController);

	/// <summary>
	/// 更新（デバッグカメラ時は追従処理をスキップ）
	/// </summary>
	void Update();

	/// <summary>
	/// 最初にしっかり追従させるためのリセット関数
	/// </summary>
	void Reset();

	/// <summary>
	/// ImGui表示（カメラモード表示とパラメータ調整）
	/// </summary>
	void ImGui();

	// Setter
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

	// Getter
	/// <summary>
	/// 現在のカメラ位置取得
	/// </summary>
	/// <returns>カメラ位置</returns>
	Vector3 GetCameraPosition() const;

	/// <summary>
	/// 目標位置取得
	/// </summary>
	/// <returns>目標位置</returns>
	Vector3 GetTargetPosition() const { return targetPosition_; }

	/// <summary>
	/// 追従が有効かどうか取得
	/// </summary>
	/// <returns>追従有効ならtrue</returns>
	bool IsFollowingActive() const;
};