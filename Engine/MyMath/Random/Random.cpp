#include "Random.h"

Random::Random() : randomEngine_(seedGenerator_()) {
	//シードの設定完了してる
}

Random& Random::GetInstance() {
	// C++11以降、local staticは自動的にスレッドセーフ
	//AIの指示に従った。マルチスレッドの安全性のためらしい
	static Random instance;
	return instance;
}

float Random::GenerateFloat(float min, float max) {
	std::lock_guard<std::mutex> lock(mutex_);
	std::uniform_real_distribution<float> distribution(min, max);
	return distribution(randomEngine_);
}

int Random::GenerateInt(int min, int max) {
	std::lock_guard<std::mutex> lock(mutex_);
	std::uniform_int_distribution<int> distribution(min, max);
	return distribution(randomEngine_);
}

float Random::GenerateNormalized() {
	return GenerateFloat(0.0f, 1.0f);
}

float Random::GenerateFloatWithOffset(float baseValue, float offset) {
	return GenerateFloat(baseValue - offset, baseValue + offset);
}

Vector3 Random::GenerateVector3WithOffset(const Vector3& baseVector, float offset) {
	return Vector3{
		GenerateFloatWithOffset(baseVector.x, offset),
		GenerateFloatWithOffset(baseVector.y, offset),
		GenerateFloatWithOffset(baseVector.z, offset)
	};
}

Vector3 Random::GenerateVector3WithOffset(const Vector3& baseVector, const Vector3& offsetVector) {
	return Vector3{
		GenerateFloatWithOffset(baseVector.x, offsetVector.x),
		GenerateFloatWithOffset(baseVector.y, offsetVector.y),
		GenerateFloatWithOffset(baseVector.z, offsetVector.z)
	};
}