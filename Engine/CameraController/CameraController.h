#pragma once
#include "BaseCamera.h"
#include "Camera.h"
#include "DebugCamera.h"
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>

/// <summary>
/// カメラ登録・管理するコントローラー
/// </summary>
class CameraController {
public:
	/// <summary>
	/// シングルトンインスタンス取得
	/// </summary>
	static CameraController* GetInstance();

	/// <summary>
	/// カメラシステムの初期化
	/// 標準カメラ（Normal、Debug）を自動登録
	/// </summary>
	void Initialize(const Vector3& position);

	/// <summary>
	/// 全カメラの更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// ImGuiデバッグ表示
	/// </summary>
	void ImGui();


	// カメラ登録システム

	/// <summary>
	/// カメラを登録
	/// </summary>
	/// <param name="cameraId">カメラ識別ID</param>
	/// <param name="camera">カメラインスタンス</param>
	void RegisterCamera(const std::string& cameraId, std::unique_ptr<BaseCamera> camera);

	/// <summary>
	/// カメラの登録解除
	/// </summary>
	/// <param name="cameraId">カメラ識別ID</param>
	void UnregisterCamera(const std::string& cameraId);

	/// <summary>
	/// アクティブカメラを手動設定
	/// </summary>
	/// <param name="cameraId">アクティブにするカメラID</param>
	void SetActiveCamera(const std::string& cameraId);

	/// <summary>
	/// アクティブカメラのIDを取得
	/// </summary>
	/// <returns>現在アクティブなカメラのID</returns>
	std::string GetActiveCameraId() const { return activeCameraId_; }

	/// <summary>
	/// 登録されているカメラの一覧を取得
	/// </summary>
	/// <returns>カメラID一覧</returns>
	std::vector<std::string> GetRegisteredCameraIds() const;



	/// <summary>
	/// デバッグカメラが使用中かどうか確認
	/// </summary>
	/// <returns>デバッグカメラ使用中ならtrue</returns>
	bool IsUsingDebugCamera() const { return activeCameraId_ == "debug"; }

	/// <summary>
	/// デバッグカメラのON/OFF切り替え
	/// </summary>
	void ToggleDebugCamera();

	/// <summary>
	/// 指定されたカメラが登録されているかチェック
	/// </summary>
	bool IsRegistered(const std::string& cameraId) const;


	Matrix4x4 GetViewProjectionMatrix() const;
	Matrix4x4 GetViewProjectionMatrixSprite() const;

	Vector3 GetPosition() const;
	void SetPosition(const Vector3& position);


	BaseCamera* GetCamera(const std::string& cameraId) const;

private:
	CameraController() = default;
	~CameraController() = default;
	CameraController(const CameraController&) = delete;
	CameraController& operator=(const CameraController&) = delete;

	/// <summary>
	/// カメラ情報構造体
	/// エンジン搭載のカメラは消せない
	/// </summary>
	struct CameraInfo {
		std::unique_ptr<BaseCamera> camera; // カメラインスタンス
		bool isEngineCamera; // エンジン標準カメラかどうか
	};


	// 登録されたカメラの管理マップ
	std::unordered_map<std::string, CameraInfo> registeredCameras_;

	// 現在アクティブなカメラのID
	std::string activeCameraId_ = "normal";
	// デバッグカメラ切り替え前のカメラID（戻り先）
	std::string lastActiveCameraId_ = "normal";


	// 現在アクティブなカメラを取得
	BaseCamera* GetActiveCamera() const;

	// エンジン標準カメラの登録
	void RegisterBuiltInCameras(const Vector3& initialPosition);

	//デバッグ入力処理（Shift+TABでの切り替え）
	void HandleDebugInput();
};