#pragma once
#include <memory>
#include "Engine.h"
#include "Objects/GameObject/GameObject.h"
#include "Objects/Sprite/Sprite.h"
#include "Managers/Input/inputManager.h"

/// <summary>
/// プレイヤーのレティクル管理クラス（2D/3Dレティクル）
/// </summary>
class PlayerReticle {
public:
	PlayerReticle() = default;
	~PlayerReticle() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	/// <param name="viewProjectionMatrixSprite">スプライト用ビュープロジェクション行列</param>
	void Update(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewProjectionMatrixSprite);

	/// <summary>
	/// 3Dレティクルの描画
	/// </summary>
	/// <param name="directionalLight">平行光源</param>
	void Draw3D(const Light& directionalLight);

	/// <summary>
	/// 2Dレティクルの描画
	/// </summary>
	void Draw2D();

	/// <summary>
	/// ImGui表示
	/// </summary>
	void ImGui();

	// Getter
	Vector3 GetWorldPosition3DReticle() const;
	Vector2 GetPosition2DReticle() const;
	bool ShouldShow2DReticle() const { return show2DReticle_; }

	// Setter
	void SetShow2DReticle(bool show) { show2DReticle_ = show; }
	void SetReticleSpeed(float speed) { kReticleSpeed_ = speed; }
	void SetReticleDistance(float keyboardDistance, float gamepadDistance) {
		kDistancePlayerTo3DReticleKeyboard_ = keyboardDistance;
		kDistancePlayerTo3DReticleGamepad_ = gamepadDistance;
	}

private:
	// システム参照
	DirectXCommon* directXCommon_ = nullptr;
	InputManager* input_ = nullptr;

	// 3Dレティクル用のゲームオブジェクト
	std::unique_ptr<Model3D> reticle3D_;

	// 2Dレティクル用のスプライト
	std::unique_ptr<Sprite> sprite2DReticle_;

	// ビューポート行列（座標変換用）
	Matrix4x4 matViewport_;

	// ゲームパッド用のNearFar座標
	Vector3 posNear_;
	Vector3 posFar_;
	Vector3 spritePosition_;

	// 表示制御
	bool show2DReticle_ = true;

	// 設定値
	float kReticleSpeed_ = 10.0f; // レティクル移動速度
	float kDistancePlayerTo3DReticleKeyboard_ = 50.0f; // プレイヤーから3Dレティクルまでの距離(キーボード)
	float kDistancePlayerTo3DReticleGamepad_ = 50.0f;  // プレイヤーから3Dレティクルまでの距離(ゲームパッド)

	/// <summary>
	/// 3Dレティクルの更新処理
	/// </summary>
	void UpdateReticle(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// ゲームパッドでスクリーン座標を3Dレティクルのワールド座標に変換
	/// </summary>
	void ConvertGamePadToWorldReticle(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// キーボードで上下左右キーでレティクルを操作
	/// </summary>
	void ConvertKeyboardToWorldReticle(const Matrix4x4& viewProjectionMatrix);
};