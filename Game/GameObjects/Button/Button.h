#pragma once

#include <memory>
#include <functional>
#include "Objects/Sprite/Sprite.h"
#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "Managers/Input/InputManager.h"

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>  // DIK_SPACE定数のために必要

/// <summary>
/// インタラクティブなボタンクラス
/// スプライトベースで入力処理とプレス効果を持つ
/// </summary>
class Button
{
public:
	Button() = default;
	~Button() = default;

	/// <summary>
	/// ボタンの初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="textureName">ボタンのテクスチャ名</param>
	/// <param name="position">ボタンの位置</param>
	/// <param name="size">ボタンのサイズ</param>
	/// <param name="anchor">アンカーポイント</param>
	void Initialize(
		DirectXCommon* dxCommon,
		const std::string& textureName,
		const Vector2& position,
		const Vector2& size,
		const Vector2& anchor = { 0.5f, 0.5f }
	);

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	void Update(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	/// <summary>
	/// ImGui用のデバッグ表示
	/// </summary>
	void ImGui();

	// 状態取得
	bool IsPressed() const { return isPressed_; }
	bool IsJustPressed() const { return isJustPressed_; }
	bool IsJustReleased() const { return isJustReleased_; }
	bool IsVisible() const { return sprite_->IsVisible(); }
	bool IsActive() const { return isActive_; }

	// 基本的なプロパティ
	Vector2 GetPosition() const { return sprite_->GetPosition(); }
	Vector2 GetSize() const { return normalSize_; }
	const std::string& GetName() const { return name_; }

	// 設定
	void SetPosition(const Vector2& position) {
		basePosition_ = position;
		sprite_->SetPosition(position);
	}
	void SetSize(const Vector2& size);
	void SetVisible(bool visible) { sprite_->SetVisible(visible); }
	void SetActive(bool active) { isActive_ = active; }
	void SetName(const std::string& name) { name_ = name; }
	void SetTexture(const std::string& textureName) { sprite_->SetTexture(textureName); }
	void SetColor(const Vector4& color) { sprite_->SetColor(color); }

	// プレス効果の設定
	void SetPressScale(float pressScale) { pressScale_ = pressScale; }
	void SetTransitionSpeed(float speed) { transitionSpeed_ = speed; }

	// 入力キーの設定
	void SetKeyboardKey(int keyCode) { keyboardKey_ = keyCode; }
	void SetGamePadButton(InputManager::GamePadButton button) { gamePadButton_ = button; }

	// アイドルアニメーション設定
	void SetIdleAnimationEnabled(bool enabled) { idleAnimationEnabled_ = enabled; }
	void SetIdleAnimationSpeed(float speed) { idleAnimationSpeed_ = speed; }
	void SetIdleAnimationRange(float range) { idleAnimationRange_ = range; }
	bool IsIdleAnimationEnabled() const { return idleAnimationEnabled_; }

private:
	/// <summary>
	/// 入力状態をチェック
	/// </summary>
	bool CheckInput();

	/// <summary>
	/// サイズアニメーションを更新
	/// </summary>
	void UpdateSizeAnimation();

	/// <summary>
	/// アイドル時の上下アニメーションを更新
	/// </summary>
	void UpdateIdleAnimation();

private:
	// スプライト
	std::unique_ptr<Sprite> sprite_;

	// 状態管理
	bool isActive_ = true;
	bool isPressed_ = false;
	bool wasPressed_ = false;
	bool isJustPressed_ = false;
	bool isJustReleased_ = false;

	// 基本情報
	std::string name_ = "Button";

	// サイズ管理
	Vector2 normalSize_{ 100.0f, 100.0f };
	Vector2 currentSize_{ 100.0f, 100.0f };
	float pressScale_ = 0.9f;  // プレス時のスケール倍率
	float transitionSpeed_ = 8.0f;  // サイズ変更の速度

	// 入力設定
	int keyboardKey_ = DIK_SPACE;  // デフォルトはスペースキー
	InputManager::GamePadButton gamePadButton_ = InputManager::GamePadButton::A;  // デフォルトはAボタン

	// システム参照
	InputManager* inputManager_ = nullptr;

	// アイドルアニメーション
	float idleAnimationTimer_ = 0.0f;
	float idleAnimationSpeed_ = 2.0f;     // アニメーションの速度（数値が大きいほど速い）
	float idleAnimationRange_ = 5.0f;     // 上下の移動範囲
	Vector2 basePosition_{ 0.0f, 0.0f };  // 基準位置
	bool idleAnimationEnabled_ = true;
};