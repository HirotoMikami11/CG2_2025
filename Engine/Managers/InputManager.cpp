#define NOMINMAX
#include "InputManager.h"
#include "Managers/ImGui/ImGuiManager.h"
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


static const float TRIGGER_BUTTON_THRESHOLD = 0.5f;


bool InputManager::IsLeftTriggerDown(int controllerIndex) const {
	return GetLeftTrigger(controllerIndex) >= TRIGGER_BUTTON_THRESHOLD;
}

bool InputManager::IsRightTriggerDown(int controllerIndex) const {
	return GetRightTrigger(controllerIndex) >= TRIGGER_BUTTON_THRESHOLD;
}

bool InputManager::IsLeftTriggerTrigger(int controllerIndex) const {
	if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS) {
		return false;
	}
	if (!gamePadConnected[controllerIndex]) {
		return false;
	}

	// 現在の状態
	float currentTrigger = GetLeftTrigger(controllerIndex);
	bool currentPressed = currentTrigger >= TRIGGER_BUTTON_THRESHOLD;

	// 前フレームの状態を計算
	BYTE prevTriggerValue = preGamePadState[controllerIndex].Gamepad.bLeftTrigger;
	float prevTrigger = 0.0f;
	if (prevTriggerValue >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
		prevTrigger = static_cast<float>(prevTriggerValue) / 255.0f;
	}
	bool prevPressed = prevTrigger >= TRIGGER_BUTTON_THRESHOLD;

	return currentPressed && !prevPressed;
}

bool InputManager::IsRightTriggerTrigger(int controllerIndex) const {
	if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS) {
		return false;
	}
	if (!gamePadConnected[controllerIndex]) {
		return false;
	}

	// 現在の状態
	float currentTrigger = GetRightTrigger(controllerIndex);
	bool currentPressed = currentTrigger >= TRIGGER_BUTTON_THRESHOLD;

	// 前フレームの状態を計算
	BYTE prevTriggerValue = preGamePadState[controllerIndex].Gamepad.bRightTrigger;
	float prevTrigger = 0.0f;
	if (prevTriggerValue >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
		prevTrigger = static_cast<float>(prevTriggerValue) / 255.0f;
	}
	bool prevPressed = prevTrigger >= TRIGGER_BUTTON_THRESHOLD;

	return currentPressed && !prevPressed;
}

bool InputManager::IsLeftTriggerRelease(int controllerIndex) const {
	if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS) {
		return false;
	}
	if (!gamePadConnected[controllerIndex]) {
		return false;
	}

	// 現在の状態
	float currentTrigger = GetLeftTrigger(controllerIndex);
	bool currentPressed = currentTrigger >= TRIGGER_BUTTON_THRESHOLD;

	// 前フレームの状態を計算
	BYTE prevTriggerValue = preGamePadState[controllerIndex].Gamepad.bLeftTrigger;
	float prevTrigger = 0.0f;
	if (prevTriggerValue >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
		prevTrigger = static_cast<float>(prevTriggerValue) / 255.0f;
	}
	bool prevPressed = prevTrigger >= TRIGGER_BUTTON_THRESHOLD;

	return !currentPressed && prevPressed;
}

bool InputManager::IsRightTriggerRelease(int controllerIndex) const {
	if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS) {
		return false;
	}
	if (!gamePadConnected[controllerIndex]) {
		return false;
	}

	// 現在の状態
	float currentTrigger = GetRightTrigger(controllerIndex);
	bool currentPressed = currentTrigger >= TRIGGER_BUTTON_THRESHOLD;

	// 前フレームの状態を計算
	BYTE prevTriggerValue = preGamePadState[controllerIndex].Gamepad.bRightTrigger;
	float prevTrigger = 0.0f;
	if (prevTriggerValue >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
		prevTrigger = static_cast<float>(prevTriggerValue) / 255.0f;
	}
	bool prevPressed = prevTrigger >= TRIGGER_BUTTON_THRESHOLD;

	return !currentPressed && prevPressed;
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

void InputManager::ImGui()
{
	if (ImGui::CollapsingHeader("InputManager")) {


		/// ゲームパッドが接続されているか確認
		bool anyConnected = false;
		for (int i = 0; i < MAX_CONTROLLERS; ++i) {
			if (IsGamePadConnected(i)) {
				anyConnected = true;
				break;
			}
		}
		// ゲームパッドが接続されていなかったら早期リターンする
		if (!anyConnected) {
			ImGui::Text("not GamePad Connect");
			return;
		}

		// 現在入力されている信号を表示
		for (int controllerIndex = 0; controllerIndex < MAX_CONTROLLERS; ++controllerIndex) {
			if (!IsGamePadConnected(controllerIndex)) {
				continue;
			}

			ImGui::Text("Controller No. %d Player", controllerIndex);

			ImGui::Separator();
#pragma region "ボタン表示"

			// ボタン表示
			ImGui::Text("Buttons");
			// 十字キー
			ImGui::Text("D-Pad:");
			ImGui::SameLine();
			if (IsGamePadButtonDown(GamePadButton::UP, controllerIndex)) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "UP");
			} else {
				ImGui::Text("UP");
			}
			ImGui::SameLine();
			if (IsGamePadButtonDown(GamePadButton::DOWN, controllerIndex)) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "DOWN");
			} else {
				ImGui::Text("DOWN");
			}
			ImGui::SameLine();
			if (IsGamePadButtonDown(GamePadButton::LEFT, controllerIndex)) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "LEFT");
			} else {
				ImGui::Text("LEFT");
			}
			ImGui::SameLine();
			if (IsGamePadButtonDown(GamePadButton::RIGHT, controllerIndex)) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "RIGHT");
			} else {
				ImGui::Text("RIGHT");
			}

			// ABXYボタン
			ImGui::Text("ABXY:");
			ImGui::SameLine();
			if (IsGamePadButtonDown(GamePadButton::A, controllerIndex)) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "A");
			} else {
				ImGui::Text("A");
			}
			ImGui::SameLine();
			if (IsGamePadButtonDown(GamePadButton::B, controllerIndex)) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "B");
			} else {
				ImGui::Text("B");
			}
			ImGui::SameLine();
			if (IsGamePadButtonDown(GamePadButton::X, controllerIndex)) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "X");
			} else {
				ImGui::Text("X");
			}
			ImGui::SameLine();
			if (IsGamePadButtonDown(GamePadButton::Y, controllerIndex)) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "Y");
			} else {
				ImGui::Text("Y");
			}
			// Stickボタン
			ImGui::Text("Stick:");
			ImGui::SameLine();
			if (IsGamePadButtonDown(GamePadButton::LS, controllerIndex)) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "LS");
			} else {
				ImGui::Text("LS");
			}
			ImGui::SameLine();
			if (IsGamePadButtonDown(GamePadButton::RS, controllerIndex)) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "RS");
			} else {
				ImGui::Text("RS");
			}
			// ショルダーボタン
			ImGui::Text("Shoulder:");
			ImGui::SameLine();
			if (IsGamePadButtonDown(GamePadButton::LB, controllerIndex)) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "LB");
			} else {
				ImGui::Text("LB");
			}
			ImGui::SameLine();
			if (IsGamePadButtonDown(GamePadButton::RB, controllerIndex)) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "RB");
			} else {
				ImGui::Text("RB");
			}

			// その他のボタン
			ImGui::Text("Other:");
			ImGui::SameLine();
			if (IsGamePadButtonDown(GamePadButton::START, controllerIndex)) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "START");
			} else {
				ImGui::Text("START");
			}
			ImGui::SameLine();
			if (IsGamePadButtonDown(GamePadButton::BACK, controllerIndex)) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "BACK");
			} else {
				ImGui::Text("BACK");
			}
#pragma endregion
			ImGui::Separator();
#pragma region "スティック表示"

			// スティック表示（横並び）
			ImGui::Text("Analog Sticks");

			// 左スティック
			ImGui::BeginGroup();
			ImGui::Text("Left Stick");
			float lx = GetAnalogStick(AnalogStick::LEFT_X, controllerIndex);
			float ly = GetAnalogStick(AnalogStick::LEFT_Y, controllerIndex);
			DrawJoystickVisualizer(lx, ly);
			ImGui::Text("X: %.3f", lx);
			ImGui::Text("Y: %.3f", ly);
			ImGui::EndGroup();

			// 右スティックを同じ行に
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20.0f);

			ImGui::BeginGroup();
			ImGui::Text("Right Stick");
			float rx = GetAnalogStick(AnalogStick::RIGHT_X, controllerIndex);
			float ry = GetAnalogStick(AnalogStick::RIGHT_Y, controllerIndex);
			DrawJoystickVisualizer(rx, ry);
			ImGui::Text("X: %.3f", rx);
			ImGui::Text("Y: %.3f", ry);
			ImGui::EndGroup();

#pragma endregion
			ImGui::Separator();
#pragma region "トリガーの具合"
			// トリガー表示（アナログ値＋ボタン判定）
			float leftTrigger = GetLeftTrigger(controllerIndex);
			float rightTrigger = GetRightTrigger(controllerIndex);

			ImGui::Text("Triggers (Analog + Button)");

			// 左トリガー
			ImGui::Text("LT: %.3f", leftTrigger);
			ImGui::SameLine();
			ImGui::ProgressBar(leftTrigger, ImVec2(100, 0));
			ImGui::SameLine();
			if (IsLeftTriggerDown(controllerIndex)) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "[PRESSED]");
			} else {
				ImGui::Text("[RELEASED]");
			}

			// 右トリガー
			ImGui::Text("RT: %.3f", rightTrigger);
			ImGui::SameLine();
			ImGui::ProgressBar(rightTrigger, ImVec2(100, 0));
			ImGui::SameLine();
			if (IsRightTriggerDown(controllerIndex)) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "[PRESSED]");
			} else {
				ImGui::Text("[RELEASED]");
			}

			// トリガーボタンのTrigger/Release判定表示
			ImGui::Text("Trigger Events:");
			if (IsLeftTriggerTrigger(controllerIndex)) {
				ImGui::SameLine();
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "LT-TRIGGER");
			}
			if (IsLeftTriggerRelease(controllerIndex)) {
				ImGui::SameLine();
				ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "LT-RELEASE");
			}
			if (IsRightTriggerTrigger(controllerIndex)) {
				ImGui::SameLine();
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "RT-TRIGGER");
			}
			if (IsRightTriggerRelease(controllerIndex)) {
				ImGui::SameLine();
				ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "RT-RELEASE");
			}
#pragma endregion

			ImGui::Separator();
#pragma region "振動テスト"
			// 振動テスト
			ImGui::Text("Vibration Test");
			static float vibrationLeft = 0.0f;
			static float vibrationRight = 0.0f;

			ImGui::SliderFloat("Left Motor", &vibrationLeft, 0.0f, 1.0f);
			ImGui::SliderFloat("Right Motor", &vibrationRight, 0.0f, 1.0f);

			if (ImGui::Button("Apply Vibration")) {
				SetGamePadVibration(vibrationLeft, vibrationRight, controllerIndex);
			}
			ImGui::SameLine();
			if (ImGui::Button("Stop Vibration")) {
				StopGamePadVibration(controllerIndex);
				vibrationLeft = 0.0f;
				vibrationRight = 0.0f;
			}

			// 複数のコントローラーがある場合の区切り
			if (controllerIndex < MAX_CONTROLLERS - 1) {
				ImGui::Separator();
				ImGui::Text("");
			}
#pragma endregion
		}
	}
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



void InputManager::DrawJoystickVisualizer(float lx, float ly) {
	// 円の半径
	const float radius = 50.0f;
	const float canvas_size = radius * 2.5f;

	// ImGuiに描画領域を確保
	ImGui::Dummy(ImVec2(canvas_size, canvas_size));

	// 描画用の座標を取得
	ImVec2 screen_pos = ImGui::GetItemRectMin();
	ImVec2 center = ImVec2(screen_pos.x + canvas_size * 0.5f, screen_pos.y + canvas_size * 0.5f);

	// 正規化された値（-1.0～1.0）を円内の座標に変換
	ImVec2 stick_pos = ImVec2(
		center.x + lx * radius * 0.9f,  // 0.9倍で円の内側に制限
		center.y - ly * radius * 0.9f   // Y軸反転
	);

	// 描画
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	// 背景円
	draw_list->AddCircleFilled(center, radius, IM_COL32(40, 40, 40, 255));

	// 外周円
	draw_list->AddCircle(center, radius, IM_COL32(200, 200, 200, 255), 32, 2.0f);

	// 十字線
	draw_list->AddLine(
		ImVec2(center.x - radius * 0.8f, center.y),
		ImVec2(center.x + radius * 0.8f, center.y),
		IM_COL32(120, 120, 120, 255), 1.0f
	);
	draw_list->AddLine(
		ImVec2(center.x, center.y - radius * 0.8f),
		ImVec2(center.x, center.y + radius * 0.8f),
		IM_COL32(120, 120, 120, 255), 1.0f
	);

	// デッドゾーン円（実際のデッドゾーンの大きさに合わせる）
	draw_list->AddCircle(center, radius * 0.25f, IM_COL32(255, 255, 255, 255), 16, 1.0f);

	// 中心点
	draw_list->AddCircleFilled(center, 3.0f, IM_COL32(255, 255, 255, 255));

	// スティック位置
	draw_list->AddCircleFilled(stick_pos, 8.0f, IM_COL32(255, 0, 0, 255));
}