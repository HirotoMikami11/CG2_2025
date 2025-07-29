#pragma once
#include<memory>
#include<string>

///BaseSystem
#include "BaseSystem/Logger/Logger.h"
#include "BaseSystem/WinApp/WinApp.h"
#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "BaseSystem/Logger/Dump.h"

///Managers
#include "Managers/Audio/AudioManager.h"
#include "Managers/Texture/TextureManager.h"
#include "Managers/Model/ModelManager.h"
#include "Managers/Input/inputManager.h"
#include "Managers/ImGui/ImGuiManager.h" 
#include "FrameTimer/FrameTimer.h"
#include "OffscreenRenderer/OffscreenRenderer.h"

///Objects
#include "CameraController/CameraController.h"
#include "Objects/GameObject/GameObject.h"
#include "Objects/Sprite/Sprite.h"
#include "Objects/Light/Light.h"

class Engine {
public:
	//シングルトン
	static Engine* GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="title">タイトルバーの文字</param>
	void Initialize(const std::wstring& title);

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <returns></returns>
	void Update();

	/// <summary>
	/// 3D描画前処理（オフスクリーン描画開始）
	/// </summary>
	void StartDrawOffscreen();

	/// <summary>
	/// 3D描画後処理（オフスクリーン描画終了)
	/// </summary>
	void EndDrawOffscreen();

	/// <summary>
	/// UI描画前処理（バックバッファ描画開始）
	/// </summary>
	void StartDrawBackBuffer();

	/// <summary>
	/// UI描画後処理（バックバッファ描画終了）
	/// </summary>
	void EndDrawBackBuffer();

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// Imgui
	/// </summary>
	void ImGui();

	// ゲッター
	WinApp* GetWinApp() const { return winApp_.get(); }
	DirectXCommon* GetDirectXCommon() const { return directXCommon_.get(); }
	OffscreenRenderer* GetOffscreenRenderer() const { return offscreenRenderer_.get(); }
	bool IsClosedWindow() const { return ClosedWindow_; }

private:
	/// <summary>
	/// コピー禁止
	/// </summary>
	Engine() = default;
	~Engine() = default;
	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;

	/// <summary>
	/// WinApp,DirectXCommonの初期化
	/// </summary>
	/// <param name="title"></param>
	void InitializeBase(const std::wstring& title);

	/// <summary>
	/// それぞれのマネージャーの初期化
	/// </summary>
	void InitializeManagers();

	/// <summary>
	/// デフォルトで読み込んでおきたいもの
	/// </summary>
	void LoadDefaultResources();

	// 基盤システム
	std::unique_ptr<WinApp> winApp_;
	std::unique_ptr<DirectXCommon> directXCommon_;

	// オフスクリーン
	std::unique_ptr<OffscreenRenderer> offscreenRenderer_;

	// マネージャー
	InputManager* inputManager_;
	TextureManager* textureManager_;
	AudioManager* audioManager_;
	ImGuiManager* imguiManager_;
	ModelManager* modelManager_;
	FrameTimer* frameTimer_;

	// カメラコントローラー
	CameraController* cameraController_;

	//ウィンドウを閉じるか否か
	bool ClosedWindow_ = false;
};