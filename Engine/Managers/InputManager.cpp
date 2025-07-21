#define NOMINMAX
#include "InputManager.h"
#include <cassert>

InputManager* InputManager::GetInstance() {
	static InputManager instance;
	return &instance;
}

void InputManager::Initialize(WinApp* winApp) {
	// DirectInputの初期化
	HRESULT result = DirectInput8Create(
		winApp->GetInstance(),
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&directInput,
		nullptr);
	assert(SUCCEEDED(result));

	/// キーボードデバイスの生成
	result = directInput->CreateDevice(GUID_SysKeyboard, &devKeyboard, NULL);
	assert(SUCCEEDED(result));//作成できなかったら停止

	///入力データ形式のセット
	result = devKeyboard->SetDataFormat(&c_dfDIKeyboard);//キーボードの標準形式
	assert(SUCCEEDED(result));//作成できなかったら停止

	///排他制御レベルのセット
	result = devKeyboard->SetCooperativeLevel(
		winApp->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));//作成できなかったら停止

	/// マウスデバイスの生成
	result = directInput->CreateDevice(GUID_SysMouse, &devMouse, NULL);
	assert(SUCCEEDED(result));//作成できなかったら停止

	///入力データ形式のセット
	result = devMouse->SetDataFormat(&c_dfDIMouse2);
	assert(SUCCEEDED(result));//作成できなかったら停止

	///排他制御レベルのセット
	result = devMouse->SetCooperativeLevel(
		winApp->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	assert(SUCCEEDED(result));//作成できなかったら停止

	// キーボードの初期化
	devKeyboard->Acquire();
	devKeyboard->GetDeviceState(sizeof(key), key);
	// 前回のキー入力状態を初期化
	memcpy(preKey, key, sizeof(key));

	// マウスの初期化
	devMouse->Acquire();
	devMouse->GetDeviceState(sizeof(mouse), &mouse);
	// 前回のマウス入力状態を初期化
	preMouse = mouse;

	// ゲームパッドの初期化
	for (int i = 0; i < MAX_CONTROLLERS; ++i) {
		ZeroMemory(&gamePadState[i], sizeof(XINPUT_STATE));
		ZeroMemory(&preGamePadState[i], sizeof(XINPUT_STATE));
		gamePadConnected[i] = false;
	}

	Logger::Log(Logger::GetStream(), "Complete InputManager initialized !!\n");
}

void InputManager::Finalize() {
	// 全てのコントローラーの振動を停止
	for (int i = 0; i < MAX_CONTROLLERS; ++i) {
		StopGamePadVibration(i);
	}
}

void InputManager::Update() {
	// 前回のキー入力状態を保存
	memcpy(preKey, key, sizeof(key));

	// キーボード情報の取得開始
	HRESULT result = devKeyboard->Acquire();

	// キーの入力状態を取得
	result = devKeyboard->GetDeviceState(sizeof(key), key);
	if (FAILED(result)) {
		// 失敗したら再取得
		devKeyboard->Acquire();
		devKeyboard->GetDeviceState(sizeof(key), key);
	}

	// 前回のマウス入力状態を保存
	preMouse = mouse;
	preMousePosition = mousePosition;

	// マウス情報の取得開始
	result = devMouse->Acquire();
	// マウスの入力状態を取得
	result = devMouse->GetDeviceState(sizeof(mouse), &mouse);
	if (FAILED(result)) {
		// 失敗したら再取得
		devMouse->Acquire();
		devMouse->GetDeviceState(sizeof(mouse), &mouse);
	}

	// マウス座標の取得（スクリーン座標）
	POINT point;
	GetCursorPos(&point);

	// POINTからVector2に変換
	mousePosition.x = static_cast<float>(point.x);
	mousePosition.y = static_cast<float>(point.y);

	// ゲームパッドの状態更新
	for (int i = 0; i < MAX_CONTROLLERS; ++i) {
		// 前回の状態を保存
		preGamePadState[i] = gamePadState[i];

		// 現在の状態を取得
		DWORD dwResult = XInputGetState(i, &gamePadState[i]);

		// 接続状態を更新
		gamePadConnected[i] = (dwResult == ERROR_SUCCESS);

		// 接続されていない場合は状態をクリア
		if (!gamePadConnected[i]) {
			ZeroMemory(&gamePadState[i], sizeof(XINPUT_STATE));
		}
	}
}

///*-----------------------------------------------------------------------*///
//																			//
///								キーボード関連								   ///
//																			//
///*-----------------------------------------------------------------------*///

bool InputManager::IsKeyDown(uint8_t keyCode) const {
	return (key[keyCode] & 0x80) != 0;
}

bool InputManager::IsKeyUp(uint8_t keyCode) const {
	return (key[keyCode] & 0x80) == 0;
}

bool InputManager::IsKeyTrigger(uint8_t keyCode) const {
	return (key[keyCode] & 0x80) != 0 && (preKey[keyCode] & 0x80) == 0;
}

bool InputManager::IsKeyRelease(uint8_t keyCode) const {
	return (key[keyCode] & 0x80) == 0 && (preKey[keyCode] & 0x80) != 0;
}

///*-----------------------------------------------------------------------*///
//																			//
///								マウス関連								   ///
//																			//
///*-----------------------------------------------------------------------*///

bool InputManager::IsMouseButtonDown(int buttonNumber) const {
	// ボタン番号の範囲チェック（0〜7）
	if (buttonNumber < 0 || buttonNumber >= 8) {
		return false;
	}
	return (mouse.rgbButtons[buttonNumber] & 0x80) != 0;
}

bool InputManager::IsMouseButtonUp(int buttonNumber) const {
	// ボタン番号の範囲チェック（0〜7）
	if (buttonNumber < 0 || buttonNumber >= 8) {
		return true;  // 範囲外なら押されていないとみなす
	}
	return (mouse.rgbButtons[buttonNumber] & 0x80) == 0;
}

bool InputManager::IsMouseButtonTrigger(int buttonNumber) const {
	// ボタン番号の範囲チェック（0〜7）
	if (buttonNumber < 0 || buttonNumber >= 8) {
		return false;
	}
	return (mouse.rgbButtons[buttonNumber] & 0x80) != 0 &&
		(preMouse.rgbButtons[buttonNumber] & 0x80) == 0;
}

bool InputManager::IsMouseButtonRelease(int buttonNumber) const {
	// ボタン番号の範囲チェック（0〜7）
	if (buttonNumber < 0 || buttonNumber >= 8) {
		return false;
	}
	return (mouse.rgbButtons[buttonNumber] & 0x80) == 0 &&
		(preMouse.rgbButtons[buttonNumber] & 0x80) != 0;
}

bool InputManager::IsMoveMouseWheel() const {
	//前フレームの情報が今の座標と違う場合にtrue
	if (preMouse.lZ != mouse.lZ) {
		return true;
	}
	return false;
}

///*-----------------------------------------------------------------------*///
//																			//
///							ゲームパッド関連								   ///
//																			//
///*-----------------------------------------------------------------------*///

bool InputManager::IsGamePadConnected(int controllerIndex) const {
	if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS) {
		return false;
	}
	return gamePadConnected[controllerIndex];
}

bool InputManager::IsGamePadButtonDown(GamePadButton button, int controllerIndex) const {
	if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS) {
		return false;
	}
	if (!gamePadConnected[controllerIndex]) {
		return false;
	}
	return (gamePadState[controllerIndex].Gamepad.wButtons & static_cast<WORD>(button)) != 0;
}

bool InputManager::IsGamePadButtonUp(GamePadButton button, int controllerIndex) const {
	if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS) {
		return true;
	}
	if (!gamePadConnected[controllerIndex]) {
		return true;
	}
	return (gamePadState[controllerIndex].Gamepad.wButtons & static_cast<WORD>(button)) == 0;
}

bool InputManager::IsGamePadButtonTrigger(GamePadButton button, int controllerIndex) const {
	if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS) {
		return false;
	}
	if (!gamePadConnected[controllerIndex]) {
		return false;
	}

	bool currentPressed = (gamePadState[controllerIndex].Gamepad.wButtons & static_cast<WORD>(button)) != 0;
	bool prevPressed = (preGamePadState[controllerIndex].Gamepad.wButtons & static_cast<WORD>(button)) != 0;

	return currentPressed && !prevPressed;
}

bool InputManager::IsGamePadButtonRelease(GamePadButton button, int controllerIndex) const {
	if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS) {
		return false;
	}
	if (!gamePadConnected[controllerIndex]) {
		return false;
	}

	bool currentPressed = (gamePadState[controllerIndex].Gamepad.wButtons & static_cast<WORD>(button)) != 0;
	bool prevPressed = (preGamePadState[controllerIndex].Gamepad.wButtons & static_cast<WORD>(button)) != 0;

	return !currentPressed && prevPressed;
}

float InputManager::GetAnalogStick(AnalogStick stick, int controllerIndex) const {
	if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS) {
		return 0.0f;
	}
	if (!gamePadConnected[controllerIndex]) {
		return 0.0f;
	}

	const XINPUT_GAMEPAD& gamepad = gamePadState[controllerIndex].Gamepad;

	switch (stick) {
	case AnalogStick::LEFT_X:
		return ApplyDeadZone(gamepad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
	case AnalogStick::LEFT_Y:
		return ApplyDeadZone(gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
	case AnalogStick::RIGHT_X:
		return ApplyDeadZone(gamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
	case AnalogStick::RIGHT_Y:
		return ApplyDeadZone(gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
	default:
		return 0.0f;
	}
}

float InputManager::GetLeftTrigger(int controllerIndex) const {
	if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS) {
		return 0.0f;
	}
	if (!gamePadConnected[controllerIndex]) {
		return 0.0f;
	}

	BYTE triggerValue = gamePadState[controllerIndex].Gamepad.bLeftTrigger;

	// デッドゾーン処理
	if (triggerValue < XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
		return 0.0f;
	}

	// 0.0f ~ 1.0f に正規化
	return static_cast<float>(triggerValue) / 255.0f;
}

float InputManager::GetRightTrigger(int controllerIndex) const {
	if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS) {
		return 0.0f;
	}
	if (!gamePadConnected[controllerIndex]) {
		return 0.0f;
	}

	BYTE triggerValue = gamePadState[controllerIndex].Gamepad.bRightTrigger;

	// デッドゾーン処理
	if (triggerValue < XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
		return 0.0f;
	}

	// 0.0f ~ 1.0f に正規化
	return static_cast<float>(triggerValue) / 255.0f;
}

void InputManager::SetGamePadVibration(float leftMotor, float rightMotor, int controllerIndex) {
	if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS) {
		return;
	}
	if (!gamePadConnected[controllerIndex]) {
		return;
	}

	// 値を0.0f ~ 1.0f にクランプ
	leftMotor = std::max(0.0f, std::min(1.0f, leftMotor));
	rightMotor = std::max(0.0f, std::min(1.0f, rightMotor));

	XINPUT_VIBRATION vibration;
	vibration.wLeftMotorSpeed = static_cast<WORD>(leftMotor * 65535.0f);
	vibration.wRightMotorSpeed = static_cast<WORD>(rightMotor * 65535.0f);

	XInputSetState(controllerIndex, &vibration);
}

void InputManager::StopGamePadVibration(int controllerIndex) {
	SetGamePadVibration(0.0f, 0.0f, controllerIndex);
}

float InputManager::ApplyDeadZone(SHORT value, SHORT deadZone) const {
	// デッドゾーン処理
	if (std::abs(value) < deadZone) {
		return 0.0f;
	}

	// デッドゾーンを超えた分を0 ~ 1の範囲に正規化
	float normalizedValue;
	if (value > 0) {
		normalizedValue = static_cast<float>(value - deadZone) / static_cast<float>(32767 - deadZone);
	} else {
		normalizedValue = static_cast<float>(value + deadZone) / static_cast<float>(32768 - deadZone);
	}

	// -1.0f ~ 1.0f にクランプ
	return std::max(-1.0f, std::min(1.0f, normalizedValue));
}