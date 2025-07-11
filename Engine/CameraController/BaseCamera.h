#pragma once
#include "MyMath/MyFunction.h"
#include <string>

/// <summary>
/// カメラの基底クラス
/// </summary>
class BaseCamera {
public:
	/// <summary>
	/// 仮想デストラクタ
	/// </summary>
	virtual ~BaseCamera() = default;

	/// <summary>
	/// カメラの初期化
	/// </summary>
	/// <param name="position">初期位置</param>
	/// <param name="rotation">初期回転（デフォルト：{0,0,0}）</param>
	virtual void Initialize(const Vector3& position, const Vector3& rotation = { 0.0f, 0.0f, 0.0f }) = 0;

	/// <summary>
	/// カメラの更新
	/// </summary>
	virtual void Update() = 0;

	/// <summary>
	/// ImGui表示
	/// </summary>
	virtual void ImGui() = 0;


	// ビュープロジェクション行列を取得
	virtual Matrix4x4 GetViewProjectionMatrix() const = 0;
	virtual Matrix4x4 GetSpriteViewProjectionMatrix() const = 0;

	virtual Vector3 GetPosition() const = 0;
	virtual void SetPosition(const Vector3& position) = 0;
	virtual std::string GetCameraType() const = 0;

	// カメラが有効かどうかを取得
	virtual bool IsActive() const { return true; }

protected:

	BaseCamera() = default;
	BaseCamera(const BaseCamera&) = delete;
	BaseCamera& operator=(const BaseCamera&) = delete;
};