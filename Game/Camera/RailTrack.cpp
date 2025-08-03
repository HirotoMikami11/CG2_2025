#include "RailTrack.h"
#include <algorithm>

void RailTrack::SetControlPoints(const std::vector<Vector3>& controlPoints) {
	controlPoints_ = controlPoints;
	// 制御点が変更されたら長さテーブルをクリア
	lengthTable_.clear();
	totalLength_ = 0.0f;
}

Vector3 RailTrack::CalculatePosition(float t) const {
	if (!IsValid()) {
		return Vector3{ 0.0f, 0.0f, 0.0f };
	}
	return CatmullRomPosition(controlPoints_, t);
}

void RailTrack::BuildLengthTable(int resolution) {
	lengthTable_.clear();

	if (!IsValid()) {
		totalLength_ = 0.0f;
		return;
	}

	lengthTable_.reserve(resolution + 1);

	float totalLength = 0.0f;
	Vector3 previousPos = CalculatePosition(0.0f);

	// 最初のエントリ
	lengthTable_.push_back({ 0.0f, 0.0f, 0.0f });

	// 曲線を細かく分割して長さを計算
	for (int i = 1; i <= resolution; ++i) {
		float t = static_cast<float>(i) / static_cast<float>(resolution);
		Vector3 currentPos = CalculatePosition(t);

		float segmentLength = Length(currentPos - previousPos);
		totalLength += segmentLength;

		lengthTable_.push_back({ t, totalLength, segmentLength });

		previousPos = currentPos;
	}

	totalLength_ = totalLength;
}

float RailTrack::GetTFromLength(float targetLength) const {
	if (lengthTable_.empty() || targetLength <= 0.0f) {
		return 0.0f;
	}

	if (targetLength >= totalLength_) {
		return 1.0f;
	}

	// バイナリサーチで対応するエントリを見つける
	auto it = std::lower_bound(lengthTable_.begin(), lengthTable_.end(), targetLength,
		[](const LengthTableEntry& entry, float length) {
			return entry.length < length;
		});

	if (it == lengthTable_.end()) {
		return 1.0f;
	}

	if (it == lengthTable_.begin()) {
		return 0.0f;
	}

	// 線形補間
	auto prevIt = it - 1;
	float ratio = (targetLength - prevIt->length) / (it->length - prevIt->length);
	return prevIt->t + ratio * (it->t - prevIt->t);
}