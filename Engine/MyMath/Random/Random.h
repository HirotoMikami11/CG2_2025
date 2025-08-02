#pragma once
#include <random>
#include <mutex>
#include "MyMath/MyMath.h"

/// <summary>
/// 乱数生成クラス（シングルトン）
/// </summary>
class Random {
public:
	/// <summary>
	/// インスタンス取得（スレッドセーフ）
	/// </summary>
	static Random& GetInstance();

	/// <summary>
	/// 指定範囲のfloat乱数を生成
	/// </summary>
	/// <param name="min">最小値</param>
	/// <param name="max">最大値</param>
	/// <returns>乱数値</returns>
	float GenerateFloat(float min, float max);

	/// <summary>
	/// 指定範囲のint乱数を生成
	/// </summary>
	/// <param name="min">最小値</param>
	/// <param name="max">最大値</param>
	/// <returns>乱数値</returns>
	int GenerateInt(int min, int max);

	/// <summary>
	/// 0.0f～1.0fの乱数を生成
	/// </summary>
	/// <returns>乱数値</returns>
	float GenerateNormalized();

	/// <summary>
	/// 基準値からオフセット範囲内の乱数を生成
	/// </summary>
	/// <param name="baseValue">基準値</param>
	/// <param name="offset">オフセット幅（±offset）</param>
	/// <returns>baseValue ± offset範囲の乱数値</returns>
	float GenerateFloatWithOffset(float baseValue, float offset);

	/// <summary>
	/// Vector3の各成分にオフセットを適用した乱数ベクトルを生成
	/// </summary>
	/// <param name="baseVector">基準ベクトル</param>
	/// <param name="offset">各成分のオフセット幅</param>
	/// <returns>オフセットが適用されたベクトル</returns>
	Vector3 GenerateVector3WithOffset(const Vector3& baseVector, float offset);

	/// <summary>
	/// Vector3の各成分に個別のオフセットを適用した乱数ベクトルを生成
	/// </summary>
	/// <param name="baseVector">基準ベクトル</param>
	/// <param name="offsetVector">各成分のオフセット幅</param>
	/// <returns>オフセットが適用されたベクトル</returns>
	Vector3 GenerateVector3WithOffset(const Vector3& baseVector, const Vector3& offsetVector);

private:
	Random();
	~Random() = default;

	// シングルトンのためコピー・ムーブ禁止
	Random(const Random&) = delete;
	Random& operator=(const Random&) = delete;
	Random(Random&&) = delete;
	Random& operator=(Random&&) = delete;

	// 乱数エンジン
	std::random_device seedGenerator_;
	std::mt19937_64 randomEngine_;

	// スレッドセーフティのためのミューテックス
	//マルチスレッド環境で安全にするためのものらしい?
	std::mutex mutex_;
};