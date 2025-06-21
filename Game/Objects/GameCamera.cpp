#include "GameCamera.h"
#include "Objects/Player.h"
#include "Managers/ImGuiManager.h"
#include <algorithm>

GameCamera::GameCamera() {
}

GameCamera::~GameCamera() {
}

void GameCamera::Initialize(CameraController* cameraController) {
	// システム層のカメラコントローラーを参照
	cameraController_ = cameraController;

	// 初期目標位置をカメラの現在位置に設定
	if (cameraController_) {
		targetPosition_ = cameraController_->GetPosition();
	}
}

void GameCamera::Update() {
	// 追従対象が設定されていない場合は何もしない
	if (!target_ || !cameraController_) {
		return;
	}

	// デバッグカメラが有効な場合は追従処理をスキップ
	if (cameraController_->IsUsingDebugCamera()) {
		return;
	}

	// 追従対象の位置と速度を取得
	Vector3 targetPosition = target_->GetPosition();
	Vector3 targetVelocity = target_->GetVelocity();

	// 追従対象とオフセットと追従対象の速度からカメラの目標座標を計算
	Vector3 velocityOffset = Multiply(targetVelocity, kVelocityBias_);
	Vector3 totalOffset = Add(targetOffset_, velocityOffset);
	targetPosition_ = Add(targetPosition, totalOffset);

	// 現在のカメラ位置を取得
	Vector3 currentCameraPosition = GetCameraPosition();

	// 座標補間によりゆったり追従する
	Vector3 newCameraPosition = Lerp(currentCameraPosition, targetPosition_, kInterpolationRate_);

	// 障害物などで詰まっても、画面内に収まるようにカメラの移動範囲制限
	newCameraPosition.x = std::clamp(newCameraPosition.x,
		targetPosition.x + followMargin_.left,
		targetPosition.x + followMargin_.right);
	newCameraPosition.y = std::clamp(newCameraPosition.y,
		targetPosition.y + followMargin_.bottom,
		targetPosition.y + followMargin_.top);

	// マップチップの両端から先を見えないようにカメラの移動範囲制限
	newCameraPosition.x = std::clamp(newCameraPosition.x, movableArea_.left, movableArea_.right);
	newCameraPosition.y = std::clamp(newCameraPosition.y, movableArea_.bottom, movableArea_.top);

	// カメラ位置を更新
	cameraController_->SetPositon(newCameraPosition);
}

void GameCamera::Reset() {
	// 追従対象が設定されていない場合は何もしない
	if (!target_ || !cameraController_) {
		return;
	}

	// 追従対象の位置を取得
	Vector3 targetPosition = target_->GetPosition();

	// 追従対象とオフセットからカメラの座標を計算
	Vector3 resetPosition = Add(targetPosition, targetOffset_);

	// 移動範囲制限を適用
	resetPosition.x = std::clamp(resetPosition.x, movableArea_.left, movableArea_.right);
	resetPosition.y = std::clamp(resetPosition.y, movableArea_.bottom, movableArea_.top);

	// カメラ位置を即座に設定（補間なし）
	cameraController_->SetPositon(resetPosition);

	// 目標位置も同じに設定
	targetPosition_ = resetPosition;
}

Vector3 GameCamera::GetCameraPosition() const {
	if (!cameraController_) {
		return { 0.0f, 0.0f, 0.0f };
	}

	// CameraControllerから現在のカメラ位置を取得
	return cameraController_->GetPosition();
}

bool GameCamera::IsFollowingActive() const {
	if (!cameraController_ || !target_) {
		return false;
	}

	// デバッグカメラが有効でない場合に追従が有効
	return !cameraController_->IsUsingDebugCamera();
}

void GameCamera::ImGui() {
#ifdef _DEBUG
	// デバッグカメラ状態の表示
	bool isUsingDebugCamera = cameraController_ ? cameraController_->IsUsingDebugCamera() : false;

	// カメラモード表示
	ImGui::Text("Camera Mode: %s", isUsingDebugCamera ? "Debug Camera" : "Game Camera");
	if (ImGui::TreeNode("GameCamera")) {


		if (isUsingDebugCamera) {
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Following disabled (Debug Camera active)");
			ImGui::Text("Press Shift+Tab to return to Game Camera");
			ImGui::Separator();
		}

		if (target_) {

			Vector3 cameraPos = GetCameraPosition();

			ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", cameraPos.x, cameraPos.y, cameraPos.z);
			ImGui::Text("Target Position: (%.2f, %.2f, %.2f)", targetPosition_.x, targetPosition_.y, targetPosition_.z);

			// オフセット調整
			if (ImGui::SliderFloat3("Offset", &targetOffset_.x, -100.0f, 100.0f)) {
				// オフセット変更時にリアルタイム反映
			}

			// 補間速度調整（constなので一時変数で）
			static float interpolationRate = kInterpolationRate_;
			if (ImGui::SliderFloat("Interpolation Rate", &interpolationRate, 0.01f, 1.0f)) {

			}
			ImGui::Text("(Note: Interpolation rate is const in code)");

			// 速度バイアス調整（constなので一時変数で）
			static float velocityBias = kVelocityBias_;
			if (ImGui::SliderFloat("Velocity Bias", &velocityBias, 0.0f, 50.0f)) {

			}
			ImGui::Text("(Note: Velocity bias is const in code)");

			if (ImGui::Button("Reset Camera")) {
				Reset();
			}

			ImGui::Separator();

			ImGui::Text("Movable Area");
			ImGui::SliderFloat("Left", &movableArea_.left, -200.0f, 200.0f);
			ImGui::SliderFloat("Right", &movableArea_.right, -200.0f, 200.0f);
			ImGui::SliderFloat("Bottom", &movableArea_.bottom, -200.0f, 200.0f);
			ImGui::SliderFloat("Top", &movableArea_.top, -200.0f, 200.0f);

			// 移動範囲の妥当性チェック
			if (movableArea_.left >= movableArea_.right) {
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Warning: Left >= Right");
			}
			if (movableArea_.bottom >= movableArea_.top) {
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Warning: Bottom >= Top");
			}

		} else {
			ImGui::Text("No target set");
			ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Call SetTarget() to enable following");
		}
		ImGui::TreePop();
	}
#endif
}