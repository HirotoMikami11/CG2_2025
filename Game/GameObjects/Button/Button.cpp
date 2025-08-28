#include "Button.h"
#include "Managers/ImGui/ImGuiManager.h"
#include <cmath>

void Button::Initialize(DirectXCommon* dxCommon, const std::string& textureName, const Vector2& position, const Vector2& size, const Vector2& anchor)
{
	// InputManagerの取得
	inputManager_ = InputManager::GetInstance();

	// サイズを保存
	normalSize_ = size;
	currentSize_ = size;

	// 基準位置を保存
	basePosition_ = position;

	// スプライトを初期化
	sprite_ = std::make_unique<Sprite>();
	sprite_->Initialize(dxCommon, textureName, position, size, anchor);

	// 名前を設定
	ObjectIDManager* idManager = ObjectIDManager::GetInstance();
	name_ = idManager->GenerateName("Button");

	// 初期状態を設定
	isPressed_ = false;

	// アイドルアニメーションの初期化
	idleAnimationTimer_ = 0.0f;
}

void Button::Update(const Matrix4x4& viewProjectionMatrix)
{
	if (!isActive_) {
		return;
	}

	// 前フレームの状態を保存
	wasPressed_ = isPressed_;

	// 入力チェック
	isPressed_ = CheckInput();

	// プレス状態の変化を検出
	isJustPressed_ = isPressed_ && !wasPressed_;
	isJustReleased_ = !isPressed_ && wasPressed_;

	// サイズアニメーションを更新
	UpdateSizeAnimation();

	// アイドルアニメーションを更新（サイズアニメーション中でない時のみ）
	UpdateIdleAnimation();

	// スプライトを更新
	sprite_->Update(viewProjectionMatrix);
}

void Button::Draw()
{
	if (!isActive_) {
		return;
	}

	sprite_->Draw();
}

void Button::ImGui()
{
#ifdef _DEBUG
	if (ImGui::TreeNode(name_.c_str())) {
		// ボタンの状態表示
		ImGui::Text("State: %s", isPressed_ ? "Pressed" : "Released");
		ImGui::Text("Just Pressed: %s", isJustPressed_ ? "Yes" : "No");
		ImGui::Text("Just Released: %s", isJustReleased_ ? "Yes" : "No");

		ImGui::Separator();

		// アクティブ状態
		ImGui::Checkbox("Active", &isActive_);

		// サイズ設定
		if (ImGui::CollapsingHeader("Size Settings")) {
			Vector2 tempNormalSize = normalSize_;
			if (ImGui::DragFloat2("Normal Size", &tempNormalSize.x, 1.0f, 1.0f, 1000.0f)) {
				SetSize(tempNormalSize);
			}

			ImGui::DragFloat("Press Scale", &pressScale_, 0.01f, 0.1f, 1.0f);
			ImGui::DragFloat("Transition Speed", &transitionSpeed_, 0.1f, 0.1f, 20.0f);
		}

		// アイドルアニメーション設定
		if (ImGui::CollapsingHeader("Idle Animation")) {
			ImGui::Checkbox("Enabled", &idleAnimationEnabled_);
			ImGui::DragFloat("Speed", &idleAnimationSpeed_, 0.1f, 0.1f, 10.0f);
			ImGui::DragFloat("Range", &idleAnimationRange_, 0.1f, 0.0f, 50.0f);
			ImGui::Text("Current Timer: %.2f", idleAnimationTimer_);
		}

		// 入力設定
		if (ImGui::CollapsingHeader("Input Settings")) {
			ImGui::Text("Keyboard Key: %d", keyboardKey_);
			ImGui::Text("GamePad Button: %d", static_cast<int>(gamePadButton_));
		}

		// スプライト設定
		if (ImGui::CollapsingHeader("Sprite")) {
			sprite_->ImGui();
		}

		ImGui::TreePop();
	}
#endif
}

void Button::SetSize(const Vector2& size)
{
	normalSize_ = size;
	currentSize_ = size;
	sprite_->SetSize(size);
}

bool Button::CheckInput()
{
	if (!inputManager_ || !isActive_) {
		return false;
	}

	// キーボード入力チェック（IsKeyDownを使用）
	bool keyPressed = inputManager_->IsKeyDown(keyboardKey_);

	// ゲームパッド入力チェック（IsGamePadButtonDownを使用）
	bool gamePadPressed = inputManager_->IsGamePadButtonDown(gamePadButton_);

	return keyPressed || gamePadPressed;
}

void Button::UpdateSizeAnimation()
{
	Vector2 targetSize;

	// プレス状態に応じてターゲットサイズを決定
	if (isPressed_) {
		targetSize = { normalSize_.x * pressScale_, normalSize_.y * pressScale_ };
	} else {
		targetSize = normalSize_;
	}

	// 現在のサイズをターゲットサイズに向かって補間
	float deltaTime = 1.0f / 60.0f;  // 60FPS想定
	float lerpFactor = 1.0f - powf(0.1f, transitionSpeed_ * deltaTime);

	currentSize_.x += (targetSize.x - currentSize_.x) * lerpFactor;
	currentSize_.y += (targetSize.y - currentSize_.y) * lerpFactor;

	// スプライトのサイズを更新
	sprite_->SetSize(currentSize_);
}

void Button::UpdateIdleAnimation()
{
	// アイドルアニメーションが無効、または押されている時は基準位置に戻す
	if (!idleAnimationEnabled_ || isPressed_) {
		//sprite_->SetPosition(basePosition_);
		// 押されていない時にタイマーをリセットして、次回スムーズに開始
		if (!isPressed_) {
			idleAnimationTimer_ = 0.0f;
		}
		return;
	}

	// アイドルアニメーションのタイマーを更新
	float deltaTime = 1.0f / 60.0f;  // 60FPS想定
	idleAnimationTimer_ += deltaTime * idleAnimationSpeed_;

	// sin波を使って滑らかな上下運動を作成
	// -1.0f から 1.0f の範囲で振動
	float sinValue = sinf(idleAnimationTimer_);

	// sinValueを0.0f-1.0fの範囲に変換してから-idleAnimationRange_からidleAnimationRange_にマップ
	float yOffset = sinValue * idleAnimationRange_;

	// 基準位置にオフセットを加えて最終位置を計算
	Vector2 currentPosition = basePosition_;
	currentPosition.y += yOffset;

	// スプライトの位置を更新
	sprite_->SetPosition(currentPosition);
}