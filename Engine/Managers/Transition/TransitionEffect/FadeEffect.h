#pragma once
#include "MyMath/MyFunction.h"
#include "BaseTransitionEffect.h"
#include <memory>


// 前方宣言
class CameraController;

/// <summary>
/// 基本的なフェードエフェクト
/// </summary>
class FadeEffect : public BaseTransitionEffect {
public:
	FadeEffect();
	~FadeEffect() override;

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

	// フェード固有の設定
	void SetColor(const Vector4& color) { fadeColor_ = color; }
	const Vector4& GetColor() const { return fadeColor_; }

private:
	std::unique_ptr<Sprite> fadeSprite_;
	DirectXCommon* directXCommon_;

	State currentState_;
	float duration_;
	float timer_;
	float progress_;
	Vector4 fadeColor_;
};