#pragma once
#include <memory>
#include <functional>
#include "Objects/Sprite/Sprite.h"
#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "CameraController/CameraController.h"

/// <summary>
/// フェード
/// </summary>
class FadeManager {
public:
	/// <summary>
	/// フェードの状態
	/// </summary>
	enum class Status {
		None,    // フェードなし
		FadeIn,  // フェードイン
		FadeOut, // フェードアウト
	};

	FadeManager();
	~FadeManager();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// フェード開始
	/// </summary>
	/// <param name="status">フェードの状態</param>
	/// <param name="duration">持続時間</param>
	void Start(Status status, float duration);

	/// <summary>
	/// フェード停止（強制終了）
	/// </summary>
	void Stop();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// 終了判定
	/// </summary>
	/// <returns></returns>
	bool IsFinished() const;

	/// <summary>
	/// フェード中かどうか
	/// </summary>
	/// <returns></returns>
	bool IsFading() const { return status_ != Status::None; }

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

private:
	void InitializeFadeSprite();

	// 現在のフェードの状態
	Status status_;

	// フェードの暗幕
	std::unique_ptr<Sprite> sprite_;

	// フェードの持続時間
	float duration_;
	// 経過時間カウンター
	float counter_;

	// システム参照
	DirectXCommon* directXCommon_;
	CameraController* cameraController_;
};