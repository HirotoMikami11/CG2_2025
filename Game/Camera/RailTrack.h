#pragma once
#include <vector>
#include "MyMath/MyFunction.h"

/// <summary>
/// レールカメラの軌道データ管理クラス
/// </summary>
class RailTrack {
public:
	/// <summary>
	/// 長さテーブルのエントリ
	/// </summary>
	struct LengthTableEntry {
		float t;			// パラメータt
		float length;		// 累積長さ
		float segmentLength;// このセグメントの長さ
	};

	RailTrack() = default;
	~RailTrack() = default;

	/// <summary>
	/// 制御点を設定
	/// </summary>
	void SetControlPoints(const std::vector<Vector3>& controlPoints);

	/// <summary>
	/// 制御点を取得
	/// </summary>
	const std::vector<Vector3>& GetControlPoints() const { return controlPoints_; }

	/// <summary>
	/// スプライン上の位置を計算
	/// </summary>
	Vector3 CalculatePosition(float t) const;

	/// <summary>
	/// 長さテーブルを構築
	/// </summary>
	void BuildLengthTable(int resolution = 1000);

	/// <summary>
	/// 長さからtパラメータを取得
	/// </summary>
	float GetTFromLength(float targetLength) const;

	/// <summary>
	/// 総長さを取得
	/// </summary>
	float GetTotalLength() const { return totalLength_; }

	/// <summary>
	/// 制御点が有効かチェック
	/// </summary>
	bool IsValid() const { return controlPoints_.size() >= 4; }

private:
	// 制御点
	std::vector<Vector3> controlPoints_;

	// 長さテーブル
	std::vector<LengthTableEntry> lengthTable_;
	float totalLength_ = 0.0f;
};