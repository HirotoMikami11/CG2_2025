#pragma once
#include <vector>
#include "CameraController/BaseCamera.h"
#include "Objects/GameObject/GameObject.h"
#include "Engine.h"

/// <summary>
/// レールカメラコントローラー
/// スプライン曲線に沿って移動するカメラ
/// </summary>
class RailCamera : public BaseCamera {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	RailCamera();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~RailCamera() override;

	/// <summary>
	/// カメラの初期化
	/// </summary>
	/// <param name="position">初期位置</param>
	/// <param name="rotation">初期回転</param>
	void Initialize(const Vector3& position, const Vector3& rotation = { 0.0f, 0.0f, 0.0f }) override;

	/// <summary>
	/// カメラの更新
	/// </summary>
	void Update() override;

	/// <summary>
	/// ImGui表示
	/// </summary>
	void ImGui() override;

	// BaseCamera interface
	Matrix4x4 GetViewProjectionMatrix() const override;
	Matrix4x4 GetSpriteViewProjectionMatrix() const override;
	Vector3 GetPosition() const override { return transform_.GetPosition(); }
	void SetPosition(const Vector3& position) override { transform_.SetPosition(position); }
	std::string GetCameraType() const override { return "Rail"; }

	// RailCamera固有機能
	Transform3D& GetTransform() { return transform_; }
	const Transform3D& GetTransform() const { return transform_; }

	/// <summary>
	/// レールカメラの軌道描画
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	/// <param name="directionalLight">平行光源</param>
	void DrawRailTrack(const Matrix4x4& viewProjectionMatrix, const Light& directionalLight);

	// レール移動制御
	void StartMovement() { isMoving_ = true; }
	void StopMovement() { isMoving_ = false; }
	void ResetPosition() { t_ = 0.0f; isMoving_ = false; }
	bool IsMoving() const { return isMoving_; }
	/// <summary>
	/// リソースの明示的解放
	/// GameSceneのFinalize時に呼び出す
	/// </summary>
	void ReleaseResources();
private:
	// トランスフォーム
	Transform3D transform_;

	// カメラパラメータ
	Matrix4x4 projectionMatrix_;
	Matrix4x4 spriteProjectionMatrix_;

	// デバッグ用のカメラモデル
	std::unique_ptr<Model3D> cameraModel_;

	// スプライン曲線制御点(通過点)
	std::vector<Vector3> controlPoints_;

	// レール移動用
	float t_;					// スプライン曲線上の位置パラメータ (0.0 - 1.0)
	float speed_;				// 移動速度
	bool isMoving_;				// 移動中フラグ
	float lookAheadDistance_;	// 注視点の先読み距離

	// 初期値保存用
	Vector3 initialPosition_;
	Vector3 initialRotation_;

	// システム参照
	DirectXCommon* directXCommon_ = nullptr;

	/// <summary>
	/// スプライン曲線上の位置を計算してカメラを更新
	/// </summary>
	void UpdateCameraPosition();

	/// <summary>
	/// カメラモデルの更新（描画時に呼び出し）
	/// </summary>
	void UpdateCameraModel();

	/// <summary>
	/// 注視点を計算する
	/// </summary>
	/// <param name="currentT">現在のtパラメータ</param>
	/// <returns>注視点の座標</returns>
	Vector3 CalculateLookAtTarget(float currentT);

	/// <summary>
	/// 指定座標・回転でデフォルト値を設定
	/// </summary>
	/// <param name="position">初期座標</param>
	/// <param name="rotation">初期回転</param>
	void SetDefaultCamera(const Vector3& position, const Vector3& rotation);
};