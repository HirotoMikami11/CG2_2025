#define NOMINMAX
#include "SlideEffect.h"
#include <algorithm>

#include "Objects/Sprite/Sprite.h"
#include "Engine.h"
#include "CameraController/CameraController.h"


SlideEffect::SlideEffect(Direction direction)
	: directXCommon_(nullptr)
	, currentState_(State::None)
	, duration_(0.0f)
	, timer_(0.0f)
	, progress_(0.0f)
	, direction_(direction)
{
}

SlideEffect::~SlideEffect() = default;

void SlideEffect::Initialize(DirectXCommon* directXCommon) {
	directXCommon_ = directXCommon;

	// 画面サイズ取得
	float screenWidth = GraphicsConfig::kClientWidth;
	float screenHeight = GraphicsConfig::kClientHeight;

	// スプライトの初期位置とサイズ
	Vector2 position = { screenWidth / 2.0f, screenHeight / 2.0f };
	Vector2 size = { screenWidth, screenHeight };

	slideSprite_ = std::make_unique<Sprite>();
	slideSprite_->Initialize(directXCommon_, "white", position, size);
	slideSprite_->SetColor({ 0.0f, 0.0f, 0.0f, 1.0f }); // 黒色

	// 方向に応じた開始・終了位置を設定
	switch (direction_) {
	case Direction::Left:
		startPosition_ = { screenWidth * 1.5f, screenHeight / 2.0f };
		endPosition_ = { -screenWidth / 2.0f, screenHeight / 2.0f };
		break;
	case Direction::Right:
		startPosition_ = { -screenWidth / 2.0f, screenHeight / 2.0f };
		endPosition_ = { screenWidth * 1.5f, screenHeight / 2.0f };
		break;
	case Direction::Up:
		startPosition_ = { screenWidth / 2.0f, screenHeight * 1.5f };
		endPosition_ = { screenWidth / 2.0f, -screenHeight / 2.0f };
		break;
	case Direction::Down:
		startPosition_ = { screenWidth / 2.0f, -screenHeight / 2.0f };
		endPosition_ = { screenWidth / 2.0f, screenHeight * 1.5f };
		break;
	}
}

void SlideEffect::Update(float deltaTime) {
	if (currentState_ == State::None || currentState_ == State::Completed) {
		return;
	}

	timer_ += deltaTime;
	progress_ = std::min(timer_ / duration_, 1.0f);

	Vector2 currentPosition;

	switch (currentState_) {
	case State::Enter:
		// Enter: 画面外から画面中央へ
		if (direction_ == Direction::Left || direction_ == Direction::Up) {
			currentPosition = Lerp(startPosition_, { GraphicsConfig::kClientWidth / 2.0f, GraphicsConfig::kClientHeight / 2.0f }, progress_);
		} else {
			currentPosition = Lerp(startPosition_, { GraphicsConfig::kClientWidth / 2.0f, GraphicsConfig::kClientHeight / 2.0f }, progress_);
		}
		break;

	case State::Exit:
		// Exit: 画面中央から画面外へ
		currentPosition = Lerp({ GraphicsConfig::kClientWidth / 2.0f, GraphicsConfig::kClientHeight / 2.0f }, endPosition_, progress_);
		break;

	default:
		break;
	}

	slideSprite_->SetPosition(currentPosition);

	// カメラコントローラーから行列を取得して更新
	CameraController* cameraController = CameraController::GetInstance();
	if (cameraController) {
		Matrix4x4 viewProjectionMatrixSprite = cameraController->GetViewProjectionMatrixSprite();
		slideSprite_->Update(viewProjectionMatrixSprite);
	}

	// 完了チェック
	if (progress_ >= 1.0f) {
		currentState_ = State::Completed;
	}
}

void SlideEffect::Draw() {
	if (!IsActive() || !slideSprite_) {
		return;
	}

	slideSprite_->Draw();
}

void SlideEffect::Start(State state, float duration) {
	if (state == State::None || state == State::Completed) {
		return;
	}

	currentState_ = state;
	duration_ = duration;
	timer_ = 0.0f;
	progress_ = 0.0f;

	// 初期位置設定
	Vector2 initialPosition;
	if (state == State::Enter) {
		initialPosition = startPosition_;
	} else {
		initialPosition = { GraphicsConfig::kClientWidth / 2.0f, GraphicsConfig::kClientHeight / 2.0f };
	}

	slideSprite_->SetPosition(initialPosition);
}

void SlideEffect::Stop() {
	if (currentState_ == State::None) {
		return;
	}

	// 現在の状態に応じて最終位置を設定
	Vector2 finalPosition;
	if (currentState_ == State::Enter) {
		finalPosition = { GraphicsConfig::kClientWidth / 2.0f, GraphicsConfig::kClientHeight / 2.0f };
	} else {
		finalPosition = endPosition_;
	}

	slideSprite_->SetPosition(finalPosition);
	currentState_ = State::Completed;
	progress_ = 1.0f;
}

void SlideEffect::Reset() {
	currentState_ = State::None;
	timer_ = 0.0f;
	progress_ = 0.0f;
	duration_ = 0.0f;

	if (slideSprite_) {
		// 画面外に配置
		slideSprite_->SetPosition(startPosition_);
	}
}

bool SlideEffect::IsCompleted() const {
	return currentState_ == State::Completed;
}

bool SlideEffect::IsActive() const {
	return currentState_ != State::None && currentState_ != State::Completed;
}

void SlideEffect::Finalize() {
	if (slideSprite_) {
		slideSprite_.reset();
	}
}