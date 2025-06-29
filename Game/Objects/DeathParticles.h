#pragma once

#include "Engine.h"
#include "Objects/GameObject/GameObject.h"
#include <array>
#include <numbers>
#include <memory>

/// <summary>
/// 死亡演出用パーティクル
/// </summary>
class DeathParticles {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	DeathParticles();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~DeathParticles();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="position">初期位置</param>
	void Initialize(const Vector3& position);

	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	void Update(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="directionalLight">ライト</param>
	void Draw(const Light& directionalLight);

	/// <summary>
	/// パーティクルが終了したかどうか
	/// </summary>
	/// <returns>終了フラグ</returns>
	bool IsFinished() const { return isFinished_; }

	/// <summary>
	/// パーティクルを開始する
	/// </summary>
	/// <param name="position">開始位置</param>
	void Start(const Vector3& position);

private:
	// パーティクルの個数
	static inline const uint32_t kNumParticles = 8;

	// パーティクルのモデル（個数分）
	std::array<std::unique_ptr<Model3D>, kNumParticles> particles_;

	/// 移動関連の定数
	// 存続時間(消滅までの時間)
	static inline const float kDuration = 2.0f;
	// 移動の速さ
	static inline const float kSpeed = 0.05f;
	// 分割した1個分の角度
	static inline const float kAngleUnit = ((2.0f * std::numbers::pi_v<float>) / kNumParticles);

	// 終了フラグ
	bool isFinished_ = false;
	// 経過時間カウンター
	float counter_ = 0.0f;

	// フェードアウト用の色
	Vector4 color_ = { 1.0f, 1.0f, 1.0f, 1.0f };

	// システム参照
	DirectXCommon* directXCommon_ = nullptr;
};