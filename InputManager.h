#pragma once
#define DIRECTINPUT_VERSION 0x0800//DirectInputのバーション指定
#include <dinput.h>
#include <wrl.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include "WinApp.h"
#include "Logger.h"

/// <summary>
/// 入力管理クラス（キーボード、マウス）
/// </summary>
class InputManager {
public:
	// シングルトンパターン
	static InputManager* GetInstance();

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="winApp">WinAppクラスのインスタンス</param>
	void Initialize(WinApp* winApp);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// 毎フレーム更新処理
	/// </summary>
	void Update();

	///*-----------------------------------------------------------------------*///
	//																			//
	///								キーボード関連								   ///
	//																			//
	///*-----------------------------------------------------------------------*///

	/// <summary>
	/// キーが押されている状態か
	/// </summary>
	bool IsKeyDown(uint8_t keyCode) const;

	/// <summary>
	/// キーが離されている状態か
	/// </summary>
	bool IsKeyUp(uint8_t keyCode) const;

	/// <summary>
	/// キーが押された瞬間か
	/// </summary>
	bool IsKeyTrigger(uint8_t keyCode) const;

	/// <summary>
	/// キーが離された瞬間か
	/// </summary>
	bool IsKeyRelease(uint8_t keyCode) const;

	///*-----------------------------------------------------------------------*///
	//																			//
	///								マウス関連								   ///
	//																			//
	///*-----------------------------------------------------------------------*///

	/// <summary>
	/// マウスボタンが押されている状態か
	/// </summary>
	/// <param name="buttonNumber">マウスボタン番号(0:左,1:右,2:中,3~7:拡張マウスボタン)</param>
	bool IsMouseButtonDown(int buttonNumber) const;

	/// <summary>
	/// マウスボタンが離されている状態か
	/// </summary>
	/// /// <param name="buttonNumber">マウスボタン番号(0:左,1:右,2:中,3~7:拡張マウスボタン)</param>
	bool IsMouseButtonUp(int buttonNumber) const;

	/// <summary>
	/// マウスボタンが押された瞬間か
	/// </summary>
	/// /// <param name="buttonNumber">マウスボタン番号(0:左,1:右,2:中,3~7:拡張マウスボタン)</param>
	bool IsMouseButtonTrigger(int buttonNumber) const;

	/// <summary>
	/// マウスボタンが離された瞬間か
	/// </summary>
	/// /// <param name="buttonNumber">マウスボタン番号(0:左,1:右,2:中,3~7:拡張マウスボタン)</param>
	bool IsMouseButtonRelease(int buttonNumber) const;
	
	bool IsMoveMouseWheel()const;

	/// <summary>
	/// マウスの移動量を取得（X軸）
	/// </summary>
	int GetMouseMoveX() const { return mouse.lX; }

	/// <summary>
	/// マウスの移動量を取得（Y軸）
	/// </summary>
	int GetMouseMoveY() const { return mouse.lY; }

	/// <summary>
	/// マウスホイールの回転量を取得
	/// </summary>
	int GetMouseWheel() const { return mouse.lZ; }

	/// <summary>
	/// マウスの座標を取得
	/// </summary>
	Vector2 GetMousePosition() const { return mousePosition; }

	/// <summary>
	/// 前のマウス座標
	/// </summary>
	/// <returns></returns>
	Vector2 GetPreMousePosition() const { return preMousePosition; }
private:

	// シングルトン用
	InputManager() = default;
	~InputManager() = default;
	InputManager(const InputManager&) = delete;
	InputManager& operator=(const InputManager&) = delete;

	// DirectInput関連
	Microsoft::WRL::ComPtr<IDirectInput8> directInput;
	Microsoft::WRL::ComPtr<IDirectInputDevice8> devKeyboard;
	Microsoft::WRL::ComPtr<IDirectInputDevice8> devMouse;

	// キーボード関連
	BYTE key[256] = {};
	BYTE preKey[256] = {};

	// マウス関連
	DIMOUSESTATE2 mouse = {};
	DIMOUSESTATE2 preMouse = {};
	Vector2 mousePosition = {};  // マウスの座標（スクリーンの座標）
	Vector2 preMousePosition = {};  // 前のマウスの座標（スクリーンの座標）
};