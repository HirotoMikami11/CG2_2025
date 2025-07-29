#pragma once
#include "BaseTransitionEffect.h"
#include <memory>
#include "MyMath/MyFunction.h"

/// <summary>
/// スライドトランジションエフェクト
/// </summary>
class SlideEffect : public BaseTransitionEffect {
public:
	enum class Direction {
		Left,
		Right,
		Up,
		Down
	};

	SlideEffect(Direction direction = Direction::Left);
	~SlideEffect() override;

	void Initialize(DirectXCommon* directXCommon) override;
	void Update(float deltaTime) override;
	void Draw() override;
	void Start(State state, float duration) override;
	void Stop() override;
	void Reset() override;
	State GetState() const override { return currentState_; }
	bool IsCompleted() const override;
	bool IsActive() const override;
	float GetProgress() const override { return progress_; }
	void Finalize() override;

	// スライド固有の設定
	void SetDirection(Direction direction) { direction_ = direction; }
	Direction GetDirection() const { return direction_; }

private:
	std::unique_ptr<Sprite> slideSprite_;
	DirectXCommon* directXCommon_;

	State currentState_;
	float duration_;
	float timer_;
	float progress_;
	Direction direction_;

	Vector2 startPosition_;
	Vector2 endPosition_;
};