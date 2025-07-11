#include "GameCamera.h"
#include "Objects/Player.h"
#include "Managers/ImGuiManager.h"
#include <algorithm>

GameCamera::GameCamera()
	: target_(nullptr)
	, cameraTransform_{}
	, viewProjectionMatrix_{}
	, spriteViewProjectionMatrix_{}
	, movableArea_{ 0.0f, 100.0f, 0.0f, 100.0f }
	, targetPosition_{ 0.0f, 0.0f, 0.0f }
	, targetOffset_{ 0.0f, 0.0f, -38.0f } {
}

GameCamera::~GameCamera() = default;

void GameCamera::Initialize(const Vector3& position, const Vector3& rotation) {
	// カメラのデフォルト設定
	cameraTransform_.scale = { 1.0f, 1.0f, 1.0f };
	cameraTransform_.rotate = rotation;
	cameraTransform_.translate = position;


	// 初期目標位置をカメラの現在位置に設定
	targetPosition_ = position;

	// 行列の初期化
	UpdateMatrix();
	UpdateSpriteMatrix();
}

void GameCamera::Update() {
	// 追従対象が設定されていない場合は何もしない
	if (!target_) {
		return;
	}


	UpdateFollow();
	UpdateMatrix();
	UpdateSpriteMatrix();


}

void GameCamera::UpdateFollow() {
	if (!target_) return;

	// 追従対象の位置と速度を取得
	Vector3 targetPosition = target_->GetPosition();
	Vector3 targetVelocity = target_->GetVelocity();

	// 追従対象とオフセットと追従対象の速度からカメラの目標座標を計算
	Vector3 velocityOffset = Multiply(targetVelocity, kVelocityBias_);
	Vector3 totalOffset = Add(targetOffset_, velocityOffset);
	targetPosition_ = Add(targetPosition, totalOffset);

	// 現在のカメラ位置を取得
	Vector3 currentCameraPosition = cameraTransform_.translate;

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
	cameraTransform_.translate = newCameraPosition;
}

void GameCamera::UpdateMatrix() {
	// 3D用のビュープロジェクション行列を計算
	Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform_.scale, cameraTransform_.rotate, cameraTransform_.translate);
	Matrix4x4 viewMatrix = Matrix4x4Inverse(cameraMatrix);
	Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, (float(GraphicsConfig::kClientWidth) / float(GraphicsConfig::kClientHeight)), 0.1f, 100.0f);
	viewProjectionMatrix_ = Matrix4x4Multiply(viewMatrix, projectionMatrix);
}

void GameCamera::UpdateSpriteMatrix() {
	// スプライト用のビュープロジェクション行列を計算
	Matrix4x4 spriteViewMatrix = MakeIdentity4x4();
	Matrix4x4 spriteProjectionMatrix = MakeOrthograpicMatrix(
		0.0f, 0.0f,
		float(GraphicsConfig::kClientWidth),
		float(GraphicsConfig::kClientHeight),
		0.0f, 100.0f
	);
	spriteViewProjectionMatrix_ = Matrix4x4Multiply(spriteViewMatrix, spriteProjectionMatrix);
}

void GameCamera::Reset() {
	// 追従対象が設定されていない場合は何もしない
	if (!target_) return;

	// 追従対象の位置を取得
	Vector3 targetPosition = target_->GetPosition();

	// 追従対象とオフセットからカメラの座標を計算
	Vector3 resetPosition = Add(targetPosition, targetOffset_);

	// 移動範囲制限を適用
	resetPosition.x = std::clamp(resetPosition.x, movableArea_.left, movableArea_.right);
	resetPosition.y = std::clamp(resetPosition.y, movableArea_.bottom, movableArea_.top);

	//カメラの位置設定
	cameraTransform_.translate = resetPosition;
	targetPosition_ = resetPosition;
	UpdateMatrix();
	UpdateSpriteMatrix();
}

void GameCamera::ImGui() {
#ifdef _DEBUG
	ImGui::Text("GameCamera (Player Follow)");
	ImGui::Separator();

	if (target_) {
		Vector3 cameraPos = GetPosition();
		Vector3 playerPos = target_->GetPosition();

		ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", cameraPos.x, cameraPos.y, cameraPos.z);
		ImGui::Text("Target Position: (%.2f, %.2f, %.2f)", targetPosition_.x, targetPosition_.y, targetPosition_.z);
		ImGui::Text("Player Position: (%.2f, %.2f, %.2f)", playerPos.x, playerPos.y, playerPos.z);


		ImGui::Separator();

		// オフセット調整
		if (ImGui::SliderFloat3("Offset", &targetOffset_.x, -100.0f, 100.0f)) {
		}

		// 補間速度調整
		static float interpolationRate = kInterpolationRate_;
		if (ImGui::SliderFloat("Interpolation Rate", &interpolationRate, 0.01f, 1.0f)) {
		}

		// 速度バイアス調整
		static float velocityBias = kVelocityBias_;
		if (ImGui::SliderFloat("Velocity Bias", &velocityBias, 0.0f, 50.0f)) {
		}

		if (ImGui::Button("Reset Camera")) {
			Reset();
		}

		ImGui::Separator();

		ImGui::Text("Movable Area");
		if (ImGui::SliderFloat("Left", &movableArea_.left, -200.0f, 200.0f) ||
			ImGui::SliderFloat("Right", &movableArea_.right, -200.0f, 200.0f) ||
			ImGui::SliderFloat("Bottom", &movableArea_.bottom, -200.0f, 200.0f) ||
			ImGui::SliderFloat("Top", &movableArea_.top, -200.0f, 200.0f)) {
			// 移動範囲変更時は即座に反映
		}

		// 移動範囲の妥当性チェック
		if (movableArea_.left >= movableArea_.right) {
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Warning: Left >= Right");
		}
		if (movableArea_.bottom >= movableArea_.top) {
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Warning: Bottom >= Top");
		}

	}
#endif
}