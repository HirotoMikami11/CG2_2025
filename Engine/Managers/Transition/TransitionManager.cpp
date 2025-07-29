#include "TransitionManager.h"
#include "TransitionEffect/FadeEffect.h"
#include "Engine.h"
#include "BaseSystem/Logger/Logger.h"
#include <format>

TransitionManager* TransitionManager::GetInstance() {
	static TransitionManager instance;
	return &instance;
}

void TransitionManager::Initialize() {
	directXCommon_ = Engine::GetInstance()->GetDirectXCommon();

	// デフォルトのフェードエフェクトを登録
	auto fadeEffect = std::make_unique<FadeEffect>();
	fadeEffect->Initialize(directXCommon_);
	RegisterEffect("fade", std::move(fadeEffect));

	Logger::Log(Logger::GetStream(), "TransitionManager initialized\n");
}

void TransitionManager::RegisterEffect(const std::string& name, std::unique_ptr<BaseTransitionEffect> effect) {
	if (!effect) return;

	effects_[name] = std::move(effect);
	Logger::Log(Logger::GetStream(), std::format("Transition effect '{}' registered\n", name));
}

BaseTransitionEffect* TransitionManager::GetEffect(const std::string& name) {
	auto it = effects_.find(name);
	if (it != effects_.end()) {
		return it->second.get();
	}
	return nullptr;
}

void TransitionManager::StartTransition(
	const std::string& effectName,
	float enterDuration,
	float exitDuration,
	TransitionCallback onTransition,
	TransitionCallback onComplete)
{
	if (IsTransitioning()) {
		return;
	}

	auto effect = GetEffect(effectName);
	if (!effect) {
		Logger::Log(Logger::GetStream(), std::format("Transition effect '{}' not found\n", effectName));
		return;
	}

	activeEffect_ = effect;
	enterDuration_ = enterDuration;
	exitDuration_ = exitDuration;
	onTransitionCallback_ = onTransition;
	onCompleteCallback_ = onComplete;

	// エフェクト開始
	currentState_ = TransitionState::EnterEffect;
	activeEffect_->Start(BaseTransitionEffect::State::Enter, enterDuration_);

	Logger::Log(Logger::GetStream(), std::format("Transition started with effect '{}'\n", effectName));
}

void TransitionManager::FadeTransition(
	float duration,
	TransitionCallback onTransition,
	TransitionCallback onComplete)
{
	StartTransition("fade", duration, duration, onTransition, onComplete);
}

void TransitionManager::CustomTransition(
	BaseTransitionEffect* effect,
	float enterDuration,
	float exitDuration,
	TransitionCallback onTransition,
	TransitionCallback onComplete)
{
	if (!effect || IsTransitioning()) {
		return;
	}

	activeEffect_ = effect;
	enterDuration_ = enterDuration;
	exitDuration_ = exitDuration;
	onTransitionCallback_ = onTransition;
	onCompleteCallback_ = onComplete;

	currentState_ = TransitionState::EnterEffect;
	activeEffect_->Start(BaseTransitionEffect::State::Enter, enterDuration_);
}

void TransitionManager::Update(float deltaTime) {
	if (!activeEffect_ || currentState_ == TransitionState::None) {
		return;
	}

	// エフェクトの更新
	activeEffect_->Update(deltaTime);

	// 状態遷移の処理
	ProcessTransition();
}

void TransitionManager::Draw() {
	if (activeEffect_ && currentState_ != TransitionState::None) {
		activeEffect_->Draw();
	}
}

void TransitionManager::Stop() {
	if (activeEffect_) {
		activeEffect_->Stop();
	}

	currentState_ = TransitionState::Completed;

	// コールバックをクリア
	onTransitionCallback_ = nullptr;
	onCompleteCallback_ = nullptr;
}

bool TransitionManager::IsTransitioning() const {
	return currentState_ != TransitionState::None &&
		currentState_ != TransitionState::Completed;
}

void TransitionManager::ProcessTransition() {
	switch (currentState_) {
	case TransitionState::EnterEffect:
		// エンターエフェクト完了チェック
		if (activeEffect_->IsCompleted()) {
			currentState_ = TransitionState::Transition;

			// 遷移コールバック実行
			if (onTransitionCallback_) {
				onTransitionCallback_();
			}

			// エグジットエフェクト開始
			activeEffect_->Start(BaseTransitionEffect::State::Exit, exitDuration_);
			currentState_ = TransitionState::ExitEffect;
		}
		break;

	case TransitionState::ExitEffect:
		// エグジットエフェクト完了チェック
		if (activeEffect_->IsCompleted()) {
			currentState_ = TransitionState::Completed;

			// 完了コールバック実行
			if (onCompleteCallback_) {
				onCompleteCallback_();
			}

			// リセット
			activeEffect_->Reset();
			activeEffect_ = nullptr;
			currentState_ = TransitionState::None;
		}
		break;

	default:
		break;
	}
}

void TransitionManager::Finalize() {
	// すべてのエフェクトを終了処理
	for (auto& [name, effect] : effects_) {
		effect->Finalize();
	}
	effects_.clear();

	activeEffect_ = nullptr;
	currentState_ = TransitionState::None;
}