#include "CameraController.h"
#include "Managers/ImGui/ImGuiManager.h"
#include "Managers/Input/inputManager.h"
#include <algorithm>
#include <vector>

CameraController* CameraController::GetInstance() {
	static CameraController instance;
	return &instance;
}

void CameraController::Initialize(const Vector3& position, const Vector3& rotation) {
	RegisterBuiltInCameras(position, rotation);
	SetActiveCamera("normal");
}

void CameraController::RegisterBuiltInCameras(const Vector3& initialPosition, const Vector3& initialRotation) {
	// カメラを登録
	auto normalCamera = std::make_unique<NormalCamera>();
	normalCamera->Initialize(initialPosition, initialRotation);
	registeredCameras_["normal"] = { std::move(normalCamera), true };

	// Debugカメラを登録
	auto debugCamera = std::make_unique<DebugCamera>();
	debugCamera->Initialize(initialPosition, initialRotation);
	registeredCameras_["debug"] = { std::move(debugCamera), true };
}

void CameraController::RegisterCamera(const std::string& cameraId, std::unique_ptr<BaseCamera> camera) {
	if (!camera) {
		return;
	}

	registeredCameras_[cameraId] = { std::move(camera), false };
}

void CameraController::UnregisterCamera(const std::string& cameraId) {
	auto it = registeredCameras_.find(cameraId);
	if (it == registeredCameras_.end()) {
		return;
	}

	// エンジン標準カメラは削除不可
	if (it->second.isEngineCamera) {
		return;
	}

	// 現在アクティブなカメラを削除する場合は、normalカメラに切り替え
	if (activeCameraId_ == cameraId) {
		activeCameraId_ = "normal";
	}

	registeredCameras_.erase(it);
}

void CameraController::SetActiveCamera(const std::string& cameraId) {
	auto it = registeredCameras_.find(cameraId);
	if (it == registeredCameras_.end()) {
		return;
	}

	if (!it->second.camera->IsActive()) {
		return;
	}

	activeCameraId_ = cameraId;
}

void CameraController::Update() {
	// アクティブなカメラのみ更新
	BaseCamera* activeCamera = GetActiveCamera();
	if (activeCamera) {
		activeCamera->Update();
	}

	// アクティブカメラが無効になった場合はnormalカメラに戻す
	if (!activeCamera || !activeCamera->IsActive()) {
		SetActiveCamera("normal");
		// 新しいアクティブカメラを更新
		activeCamera = GetActiveCamera();
		if (activeCamera) {
			activeCamera->Update();
		}
	}

	HandleDebugInput();
}

void CameraController::HandleDebugInput() {
#ifdef _DEBUG
	// Shift + TAB でデバッグカメラの切り替え
	if (InputManager::GetInstance()->IsKeyTrigger(DIK_TAB) &&
		InputManager::GetInstance()->IsKeyDown(DIK_LSHIFT)) {
		ToggleDebugCamera();
	}
#endif
}
void CameraController::ToggleDebugCamera() {
	if (activeCameraId_ == "debug") {
		SetActiveCamera(lastActiveCameraId_);
	} else {
		lastActiveCameraId_ = activeCameraId_;
		SetActiveCamera("debug");
	}
}




BaseCamera* CameraController::GetActiveCamera() const {
	auto it = registeredCameras_.find(activeCameraId_);
	return (it != registeredCameras_.end()) ? it->second.camera.get() : nullptr;
}

Matrix4x4 CameraController::GetViewProjectionMatrix() const {
	BaseCamera* activeCamera = GetActiveCamera();
	return activeCamera ? activeCamera->GetViewProjectionMatrix() : MakeIdentity4x4();
}

Matrix4x4 CameraController::GetViewProjectionMatrixSprite() const {
	BaseCamera* activeCamera = GetActiveCamera();
	return activeCamera ? activeCamera->GetSpriteViewProjectionMatrix() : MakeIdentity4x4();
}

Vector3 CameraController::GetPosition() const {
	BaseCamera* activeCamera = GetActiveCamera();
	return activeCamera ? activeCamera->GetPosition() : Vector3{ 0.0f, 0.0f, 0.0f };
}

void CameraController::SetPosition(const Vector3& position) {
	BaseCamera* activeCamera = GetActiveCamera();
	if (activeCamera) {
		activeCamera->SetPosition(position);
	}
}

bool CameraController::IsRegistered(const std::string& cameraId) const {
	return registeredCameras_.find(cameraId) != registeredCameras_.end();
}

BaseCamera* CameraController::GetCamera(const std::string& cameraId) const {
	auto it = registeredCameras_.find(cameraId);
	return (it != registeredCameras_.end()) ? it->second.camera.get() : nullptr;
}

std::vector<std::string> CameraController::GetRegisteredCameraIds() const {
	std::vector<std::string> ids;
	ids.reserve(registeredCameras_.size());

	for (const auto& [id, info] : registeredCameras_) {
		ids.push_back(id);
	}

	return ids;
}














void CameraController::ImGui() {
#ifdef _DEBUG
	ImGui::Begin("CameraController");

	// 現在の状態表示
	ImGui::Text("Active Camera: %s", activeCameraId_.c_str());
	BaseCamera* activeCamera = GetActiveCamera();
	if (activeCamera) {
		ImGui::Text("Camera Type: %s", activeCamera->GetCameraType().c_str());
		Vector3 pos = activeCamera->GetPosition();
		ImGui::Text("Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
	}

	ImGui::Separator();

	// カメラ切り替えUI
	ImGui::Text("Cameras:");
	for (const auto& [id, info] : registeredCameras_) {
		ImGui::PushID(id.c_str());

		bool isActive = (id == activeCameraId_);
		bool isAvailable = info.camera && info.camera->IsActive();

		// アクティブなカメラの場合は緑色のボタンにする
		if (isActive) {
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 0.6f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.8f, 0.2f, 0.8f));
		}

		// ボタンサイズ
		if (ImGui::Button(id.c_str(), ImVec2(120, 0))) {
			if (isAvailable && !isActive) {
				SetActiveCamera(id);
			}
		}

		// アクティブなカメラの場合は色をリセット
		if (isActive) {
			ImGui::PopStyleColor(2);
			ImGui::SameLine();
			ImGui::Text("(Active)");
		} else {
			ImGui::SameLine();
			ImGui::Text("(%s%s)",
				isAvailable ? "Available" : "Unavailable",
				info.isEngineCamera ? " Engine" : "");
		}

		ImGui::PopID();
	}

	ImGui::Separator();

	// アクティブカメラのImGui
	if (activeCamera) {
		myImGui::CenterText("Camera Data");
		ImGui::Separator();
		activeCamera->ImGui();
	}

	ImGui::Separator();

	// 操作説明
	ImGui::Text("Controls:");
	ImGui::Text("Shift + TAB: Toggle Debug Camera");

	// Shift + Enter でデバッグカメラの切り替え
	ImGui::End();
#endif
}