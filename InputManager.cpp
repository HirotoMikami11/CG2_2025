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

	Logger::Log(Logger::GetStream(), "Complete InputManager initialized !!\n");
}


void InputManager::Finalize() {

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


