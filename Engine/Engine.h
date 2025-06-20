#pragma once
#include<memory>
#include<string>

///BaseSystem
#include "BaseSystem/Logger/Logger.h"
#include "BaseSystem/WinApp/WinApp.h"
#include "../DirectXCommon.h"
#include "BaseSystem/Logger/Dump.h"

///Managers
#include "../AudioManager.h"
#include "../InputManager.h"
#include "../TextureManager.h"
#include "../ImGuiManager.h"
#include "FrameTimer/FrameTimer.h"
#include "OffscreenRenderer/OffscreenRenderer.h"

///Objects
#include "../CameraController.h"
#include "../GameObject.h"
#include "../Sprite.h"
#include "../Light.h"



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
	/// 描画前処理
	/// </summary>
	void StartDraw();

	/// <summary>
	/// 描画後処理
	/// </summary>
	void EndDraw();

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
	// TODO: 今後ゲームシーンの方でいろいろ操作できるようにしたい
	//場合によってはゲームシーンに移植
	std::unique_ptr<OffscreenRenderer> offscreenRenderer_;

	// マネージャー
	InputManager* inputManager_;
	TextureManager* textureManager_;
	AudioManager* audioManager_;
	ImGuiManager* imguiManager_;
	FrameTimer* frameTimer_;

	//ウィンドウを閉じるか否か
	bool ClosedWindow_ = false;
};