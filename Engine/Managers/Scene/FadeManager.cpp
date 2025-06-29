#define NOMINMAX // C+標準のstd::maxを使えるようにするため(windows.hが上書きしてしまっている)
#include "FadeManager.h"
#include "Engine.h"
#include <algorithm>

FadeManager::FadeManager()
	: status_(Status::None)
	, duration_(0.0f)
	, counter_(0.0f)
	, directXCommon_(nullptr)
	, cameraController_(nullptr) {
}

FadeManager::~FadeManager() = default;

void FadeManager::Initialize() {
	// システム参照の取得
	directXCommon_ = Engine::GetInstance()->GetDirectXCommon();
	cameraController_ = CameraController::GetInstance();

	// フェード用スプライトの初期化
	InitializeFadeSprite();
}

void FadeManager::InitializeFadeSprite() {
	if (!directXCommon_) return;

	// 画面サイズ取得
	WinApp* winApp = Engine::GetInstance()->GetWinApp();
	float screenWidth = GraphicsConfig::kClientWidth;
	float screenHeight = GraphicsConfig::kClientHeight;

	// スプライトの中央に合わせる
	Vector2 position = { screenWidth / 2.0f, screenHeight / 2.0f };
	Vector2 size = { screenWidth, screenHeight };
	sprite_ = std::make_unique<Sprite>();
	sprite_->Initialize(directXCommon_, "white", position, size);

	// 初期色は完全に透明な黒
	sprite_->SetColor({ 0.0f, 0.0f, 0.0f, 0.0f });
}

void FadeManager::Update() {
	float alpha;

	switch (status_) {
	case Status::None:
		// 何もしない
		break;

	case Status::FadeIn:
		// フェードインの更新処理

		// デルタタイムを使用して時間を更新
		counter_ += 1.0f / 60.0f;

		// フェード持続時間に達したら打ち止め
		if (counter_ >= duration_) {
			counter_ = duration_;
		}

		// 0.0f〜1.0fの間で、経過時間がフェード持続時間に近づくほどαの値を小さくする
		alpha = 1.0f - (counter_ / duration_);
		alpha = std::max(0.0f, std::min(alpha, 1.0f));

		if (sprite_) {
			sprite_->SetColor({ 0.0f, 0.0f, 0.0f, alpha });

			if (cameraController_) {
				Matrix4x4 viewProjectionMatrixSprite = cameraController_->GetViewProjectionMatrixSprite();
				sprite_->Update(viewProjectionMatrixSprite);
			}
		}
		break;

	case Status::FadeOut:
		// フェードアウトの更新処理

		// デルタタイムを使用して時間を更新
		counter_ += 1.0f/60.0f;

		// フェード持続時間に達したら打ち止め
		if (counter_ >= duration_) {
			counter_ = duration_;
		}

		// 0.0f〜1.0fの間で、経過時間がフェード持続時間に近づくほどαの値を大きくする
		alpha = counter_ / duration_;
		alpha = std::max(0.0f, std::min(alpha, 1.0f));

		if (sprite_) {
			sprite_->SetColor({ 0.0f, 0.0f, 0.0f, alpha });

			if (cameraController_) {
				Matrix4x4 viewProjectionMatrixSprite = cameraController_->GetViewProjectionMatrixSprite();
				sprite_->Update(viewProjectionMatrixSprite);
			}
		}
		break;

	default:
		break;
	}
}

void FadeManager::Start(Status status, float duration) {
	// 取得した情報を代入
	status_ = status;
	duration_ = duration;
	counter_ = 0.0f;
}

void FadeManager::Stop() {
	// 現在のフェード状態を保存
	Status currentStatus = status_;

	// 状態をNoneに変更
	status_ = Status::None;

	if (sprite_) {
		// フェードの状態に応じて適切なα値を設定
		if (currentStatus == Status::FadeIn) {
			// フェードインの場合は完全に透明に（明るく）
			sprite_->SetColor({ 0.0f, 0.0f, 0.0f, 0.0f });
		} else if (currentStatus == Status::FadeOut) {
			// フェードアウトの場合は完全に不透明に（暗く）
			sprite_->SetColor({ 0.0f, 0.0f, 0.0f, 1.0f });
		}
	}
}

void FadeManager::Draw() {
	// 内部的にフェードが終了していない間は、Noneでも描画する。
	// これがないと、FadeOutの時にStop関数が呼び出された場合に明るくなってしまう
	if (!IsFinished()) {
		if (status_ == Status::None) {
			return;
		}
	}

	if (sprite_) {
		sprite_->Draw();
	}
}

bool FadeManager::IsFinished() const {
	// フェード状態による分岐
	switch (status_) {
	case Status::FadeIn:
	case Status::FadeOut:
		if (counter_ >= duration_) {
			return true;
		} else {
			return false;
		}
	case Status::None:
	default:
		return true;
	}
}

void FadeManager::Finalize() {
	if (sprite_) {
		sprite_.reset();
	}
}