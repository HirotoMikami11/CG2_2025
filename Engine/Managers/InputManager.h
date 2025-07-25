#pragma once
#define DIRECTINPUT_VERSION 0x0800//DirectInputのバーション指定
#include <dinput.h>	//キーボードマウス
#include <wrl.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include <Xinput.h>	//ゲームパッド
#include <cmath>
#pragma comment(lib, "xinput.lib")

#include "BaseSystem/WinApp/WinApp.h"
#include "BaseSystem/Logger/Logger.h"


/// <summary>
/// 入力管理クラス（キーボード、マウス）
/// </summary>
class InputManager {
public:
	// ゲームパッドのボタン定義
	enum class GamePadButton {
		//十字キー
		UP = XINPUT_GAMEPAD_DPAD_UP,			//十字上
		DOWN = XINPUT_GAMEPAD_DPAD_DOWN,		//十字下
		LEFT = XINPUT_GAMEPAD_DPAD_LEFT,		//十字左
		RIGHT = XINPUT_GAMEPAD_DPAD_RIGHT,		//十字右
		//真ん中のボタン
		START = XINPUT_GAMEPAD_START,			//
		BACK = XINPUT_GAMEPAD_BACK,				//

		//スティック押し込み
		LS = XINPUT_GAMEPAD_LEFT_THUMB,     // 左スティック押し込み
		RS = XINPUT_GAMEPAD_RIGHT_THUMB,    // 右スティック押し込み
		//背面
		LB = XINPUT_GAMEPAD_LEFT_SHOULDER,		//LB
		RB = XINPUT_GAMEPAD_RIGHT_SHOULDER,		//RB
		//ボタン
		A = XINPUT_GAMEPAD_A,					//A
		B = XINPUT_GAMEPAD_B,					//B
		X = XINPUT_GAMEPAD_X,					//X
		Y = XINPUT_GAMEPAD_Y					//Y
	};

	// アナログスティック定義
	enum class AnalogStick {
		LEFT_X,
		LEFT_Y,
		RIGHT_X,
		RIGHT_Y
	};

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

	///*-----------------------------------------------------------------------*///
	//																			//
	///							ゲームパッド関連								   ///
	//																			//
	///*-----------------------------------------------------------------------*///

	/// <summary>
	/// ゲームパッドが接続されているか
	/// </summary>
	/// <param name="controllerIndex">コントローラー番号(0~1)</param>
	bool IsGamePadConnected(int controllerIndex = 0) const;

	/// <summary>
	/// ゲームパッドのボタンが押されている状態か
	/// </summary>
	/// <param name="button">ボタン</param>
	/// <param name="controllerIndex">コントローラー番号(0~1)</param>
	bool IsGamePadButtonDown(GamePadButton button, int controllerIndex = 0) const;

	/// <summary>
	/// ゲームパッドのボタンが離されている状態か
	/// </summary>
	/// <param name="button">ボタン</param>
	/// <param name="controllerIndex">コントローラー番号(0~1)</param>
	bool IsGamePadButtonUp(GamePadButton button, int controllerIndex = 0) const;

	/// <summary>
	/// ゲームパッドのボタンが押された瞬間か
	/// </summary>
	/// <param name="button">ボタン</param>
	/// <param name="controllerIndex">コントローラー番号(0~1)</param>
	bool IsGamePadButtonTrigger(GamePadButton button, int controllerIndex = 0) const;

	/// <summary>
	/// ゲームパッドのボタンが離された瞬間か
	/// </summary>
	/// <param name="button">ボタン</param>
	/// <param name="controllerIndex">コントローラー番号(0~1)</param>
	bool IsGamePadButtonRelease(GamePadButton button, int controllerIndex = 0) const;

	/// <summary>
	/// アナログスティックの値を取得(-1.0f ~ 1.0f)
	/// </summary>
	/// <param name="stick">スティック</param>
	/// <param name="controllerIndex">コントローラー番号(0~1)</param>
	float GetAnalogStick(AnalogStick stick, int controllerIndex = 0) const;

	/// <summary>
	/// 左トリガーの値を取得(0.0f ~ 1.0f)
	/// </summary>
	/// <param name="controllerIndex">コントローラー番号(0~1)</param>
	float GetLeftTrigger(int controllerIndex = 0) const;

	/// <summary>
	/// 右トリガーの値を取得(0.0f ~ 1.0f)
	/// </summary>
	/// <param name="controllerIndex">コントローラー番号(0~1)</param>
	float GetRightTrigger(int controllerIndex = 0) const;


	/// <summary>
	/// 左トリガーがボタンとして押されている状態か (0.5f以上で押されたと判定)
	/// </summary>
	/// <param name="controllerIndex">コントローラー番号(0~1)</param>
	bool IsLeftTriggerDown(int controllerIndex = 0) const;

	/// <summary>
	/// 右トリガーがボタンとして押されている状態か  (0.5f以上で押されたと判定)
	/// </summary>
	/// <param name="controllerIndex">コントローラー番号(0~1)</param>
	bool IsRightTriggerDown(int controllerIndex = 0) const;

	/// <summary>
	/// 左トリガーがボタンとして押された瞬間か
	/// </summary>
	/// <param name="controllerIndex">コントローラー番号(0~1)</param>
	bool IsLeftTriggerTrigger(int controllerIndex = 0) const;

	/// <summary>
	/// 右トリガーがボタンとして押された瞬間か
	/// </summary>
	/// <param name="controllerIndex">コントローラー番号(0~1)</param>
	bool IsRightTriggerTrigger(int controllerIndex = 0) const;

	/// <summary>
	/// 左トリガーがボタンとして離された瞬間か
	/// </summary>
	/// <param name="controllerIndex">コントローラー番号(0~1)</param>
	bool IsLeftTriggerRelease(int controllerIndex = 0) const;

	/// <summary>
	/// 右トリガーがボタンとして離された瞬間か
	/// </summary>
	/// <param name="controllerIndex">コントローラー番号(0~1)</param>
	bool IsRightTriggerRelease(int controllerIndex = 0) const;

	/// <summary>
	/// ゲームパッドの振動を設定
	/// </summary>
	/// <param name="leftMotor">左モーター強度(0.0f ~ 1.0f)</param>
	/// <param name="rightMotor">右モーター強度(0.0f ~ 1.0f)</param>
	/// <param name="controllerIndex">コントローラー番号(0~1)</param>
	void SetGamePadVibration(float leftMotor, float rightMotor, int controllerIndex = 0);

	/// <summary>
	/// ゲームパッドの振動を停止
	/// </summary>
	/// <param name="controllerIndex">コントローラー番号(0~1)</param>
	void StopGamePadVibration(int controllerIndex = 0);

	/// <summary>
	/// imgui
	/// </summary>
	void ImGui();



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

	// ゲームパッド関連（最大4つのコントローラー）
	static const int MAX_CONTROLLERS = 2;
	XINPUT_STATE gamePadState[MAX_CONTROLLERS] = {};
	XINPUT_STATE preGamePadState[MAX_CONTROLLERS] = {};
	bool gamePadConnected[MAX_CONTROLLERS] = {};
	// トリガーボタンがボタンとして押される値（0.0f～1.0fの範囲で）cppに移動
	//static const float TRIGGER_BUTTON_THRESHOLD = 0.5f;


	// スティックのデッドゾーン処理
	float ApplyDeadZone(SHORT value, SHORT deadZone) const;

	/// <summary>
	/// stickの信号を表示
	/// </summary>
	/// <param name="lx"></param>
	/// <param name="ly"></param>
	/// <returns></returns>
	void DrawJoystickVisualizer(float lx, float ly);

};