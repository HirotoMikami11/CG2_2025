#pragma once
#include <memory>
#include "Engine.h"
#include "Objects/Sprite/Sprite.h"
#include "PlayerHealth.h"

/// <summary>
/// プレイヤーのUI管理クラス（ゲージなど）
/// </summary>
class PlayerUI {
public:
	PlayerUI() = default;
	~PlayerUI() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="health">プレイヤーの体力システム</param>
	/// <param name="viewProjectionMatrixSprite">スプライト用ビュープロジェクション行列</param>
	void Update(const PlayerHealth& health, const Matrix4x4& viewProjectionMatrixSprite);

	/// <summary>
	/// UI描画（ゲージ）
	/// </summary>
	void Draw();

	/// <summary>
	/// ImGui表示
	/// </summary>
	void ImGui();

	// UI設定
	void SetGaugePosition(const Vector2& hpPosition, const Vector2& enPosition);
	void SetGaugeSize(const Vector2& size);
	void SetGaugeColors(const Vector4& hpColor, const Vector4& enColor, const Vector4& backgroundColor);

private:
	// システム参照
	DirectXCommon* directXCommon_ = nullptr;

	// HP/ENゲージ用スプライト
	std::unique_ptr<Sprite> hpGaugeBar_;		 // HPゲージの枠
	std::unique_ptr<Sprite> hpGaugeFill_;		  // HPゲージの中身
	std::unique_ptr<Sprite> enGaugeBar_;		// ENゲージの枠
	std::unique_ptr<Sprite> enGaugeFill_;		 // ENゲージの中身

	// ゲージ設定
	Vector2 hpGaugePosition_{ 25.0f, 50.0f };
	Vector2 enGaugePosition_{ 25.0f, 80.0f };
	Vector2 gaugeSize_{ 200.0f, 20.0f };
	Vector2 gaugeFrameSize_{ 204.0f, 24.0f };

	Vector4 hpNormalColor_{ 0.0f, 1.0f, 0.0f, 1.0f };	// 緑色
	Vector4 hpWarningColor_{ 1.0f, 1.0f, 0.0f, 1.0f };	 // 黄色
	Vector4 hpDangerColor_{ 1.0f, 0.0f, 0.0f, 1.0f };	 // 赤色
	Vector4 enNormalColor_{ 0.0f, 0.5f, 1.0f, 1.0f };	// 青色
	Vector4 enLowColor_{ 0.0f, 0.2f, 0.5f, 1.0f };		 // 暗い青色
	Vector4 backgroundColor_{ 0.2f, 0.2f, 0.2f, 1.0f };	 // 暗い灰色

	/// <summary>
	/// HP/ENゲージの初期化
	/// </summary>
	void InitializeGauges();

	/// <summary>
	/// ゲージの更新（色とサイズ）
	/// </summary>
	/// <param name="health">プレイヤーの体力システム</param>
	void UpdateGauges(const PlayerHealth& health);
};