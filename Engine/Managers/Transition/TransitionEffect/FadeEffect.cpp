#define NOMINMAX
#include "Engine.h"
#include "CameraController/CameraController.h"
#include <algorithm>
#include "FadeEffect.h"
#include "Objects/Sprite/Sprite.h"

FadeEffect::FadeEffect()
	: directXCommon_(nullptr)
	, currentState_(State::None)
	, duration_(0.0f)
	, timer_(0.0f)
	, progress_(0.0f)
	, fadeColor_({ 0.0f, 0.0f, 0.0f, 1.0f }) // デフォルトは黒
{
}

FadeEffect::~FadeEffect() = default;

void FadeEffect::Initialize(DirectXCommon* directXCommon) {
	directXCommon_ = directXCommon;

	// 画面サイズ取得
	float screenWidth = GraphicsConfig::kClientWidth;
	float screenHeight = GraphicsConfig::kClientHeight;

	// スプライトの中央に合わせる
	Vector2 position = { screenWidth / 2.0f, screenHeight / 2.0f };
	Vector2 size = { screenWidth, screenHeight };

	fadeSprite_ = std::make_unique<Sprite>();
	fadeSprite_->Initialize(directXCommon_, "white", position, size);

	// 初期状態は完全に透明
	fadeSprite_->SetColor({ fadeColor_.x, fadeColor_.y, fadeColor_.z, 0.0f });
}

void FadeEffect::Update(float deltaTime) {
	if (currentState_ == State::None || currentState_ == State::Completed) {
		return;
	}

	timer_ += deltaTime;
	progress_ = std::min(timer_ / duration_, 1.0f);

	float alpha = 0.0f;

	switch (currentState_) {
	case State::Enter:
		// Enter: 0→1（画面を覆う）
		alpha = progress_;
		break;

	case State::Exit:
		// Exit: 1→0（画面を見せる）
		alpha = 1.0f - progress_;
		break;

	default:
		break;
	}

	fadeSprite_->SetColor({ fadeColor_.x, fadeColor_.y, fadeColor_.z, alpha });

	// カメラコントローラーから行列を取得して更新
	CameraController* cameraController = CameraController::GetInstance();
	if (cameraController) {
		Matrix4x4 viewProjectionMatrixSprite = cameraController->GetViewProjectionMatrixSprite();
		fadeSprite_->Update(viewProjectionMatrixSprite);
	}

	// 完了チェック
	if (progress_ >= 1.0f) {
		currentState_ = State::Completed;
	}
}

void FadeEffect::Draw() {
	if (!IsActive() || !fadeSprite_) {
		return;
	}

	fadeSprite_->Draw();
}

void FadeEffect::Start(State state, float duration) {
	if (state == State::None || state == State::Completed) {
		return;
	}

	currentState_ = state;
	duration_ = duration;
	timer_ = 0.0f;
	progress_ = 0.0f;

	// 初期アルファ値設定
	float initialAlpha = (state == State::Enter) ? 0.0f : 1.0f;
	fadeSprite_->SetColor({ fadeColor_.x, fadeColor_.y, fadeColor_.z, initialAlpha });
}

void FadeEffect::Stop() {
	if (currentState_ == State::None) {
		return;
	}

	// 現在の状態に応じて最終状態を設定
	float finalAlpha = (currentState_ == State::Enter) ? 1.0f : 0.0f;
	fadeSprite_->SetColor({ fadeColor_.x, fadeColor_.y, fadeColor_.z, finalAlpha });

	currentState_ = State::Completed;
	progress_ = 1.0f;
}

void FadeEffect::Reset() {
	currentState_ = State::None;
	timer_ = 0.0f;
	progress_ = 0.0f;
	duration_ = 0.0f;

	if (fadeSprite_) {
		fadeSprite_->SetColor({ fadeColor_.x, fadeColor_.y, fadeColor_.z, 0.0f });
	}
}

bool FadeEffect::IsCompleted() const {
	return currentState_ == State::Completed;
}

bool FadeEffect::IsActive() const {
	return currentState_ != State::None && currentState_ != State::Completed;
}

void FadeEffect::Finalize() {
	if (fadeSprite_) {
		fadeSprite_.reset();
	}
}