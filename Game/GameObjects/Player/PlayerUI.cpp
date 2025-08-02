#include "PlayerUI.h"
#include "Managers/ImGui/ImGuiManager.h"

void PlayerUI::Initialize(DirectXCommon* dxCommon) {
	directXCommon_ = dxCommon;
	InitializeGauges();
}

void PlayerUI::Update(const PlayerHealth& health, const Matrix4x4& viewProjectionMatrixSprite) {
	UpdateGauges(health);

	// 全スプライトの更新
	if (hpGaugeBar_) hpGaugeBar_->Update(viewProjectionMatrixSprite);
	if (hpGaugeFill_) hpGaugeFill_->Update(viewProjectionMatrixSprite);
	if (enGaugeBar_) enGaugeBar_->Update(viewProjectionMatrixSprite);
	if (enGaugeFill_) enGaugeFill_->Update(viewProjectionMatrixSprite);
}

void PlayerUI::Draw() {
	// HP/ENゲージの描画
	if (hpGaugeBar_) hpGaugeBar_->Draw();
	if (hpGaugeFill_) hpGaugeFill_->Draw();
	if (enGaugeBar_) enGaugeBar_->Draw();
	if (enGaugeFill_) enGaugeFill_->Draw();
}

void PlayerUI::InitializeGauges() {
	// HPゲージの枠
	hpGaugeBar_ = std::make_unique<Sprite>();
	hpGaugeBar_->Initialize(
		directXCommon_,
		"white",
		hpGaugePosition_,
		gaugeFrameSize_,
		{ 0.0f, 0.5f }
	);
	hpGaugeBar_->SetColor(backgroundColor_);

	// HPゲージの中身
	hpGaugeFill_ = std::make_unique<Sprite>();
	hpGaugeFill_->Initialize(
		directXCommon_,
		"white",
		{ hpGaugePosition_.x + 2.0f, hpGaugePosition_.y },
		gaugeSize_,
		{ 0.0f, 0.5f }
	);
	hpGaugeFill_->SetColor(hpNormalColor_);

	// ENゲージの枠
	enGaugeBar_ = std::make_unique<Sprite>();
	enGaugeBar_->Initialize(
		directXCommon_,
		"white",
		enGaugePosition_,
		gaugeFrameSize_,
		{ 0.0f, 0.5f }
	);
	enGaugeBar_->SetColor(backgroundColor_);

	// ENゲージの中身
	enGaugeFill_ = std::make_unique<Sprite>();
	enGaugeFill_->Initialize(
		directXCommon_,
		"white",
		{ enGaugePosition_.x + 2.0f, enGaugePosition_.y },
		gaugeSize_,
		{ 0.0f, 0.5f }
	);
	enGaugeFill_->SetColor(enNormalColor_);
}

void PlayerUI::UpdateGauges(const PlayerHealth& health) {
	// HPゲージの更新
	if (hpGaugeFill_) {
		float hpRatio = health.GetHPRatio();
		Vector2 currentSize = hpGaugeFill_->GetSize();
		currentSize.x = gaugeSize_.x * hpRatio; // 最大幅に対する割合
		hpGaugeFill_->SetSize(currentSize);

		// HPの量に応じて色を変更
		if (hpRatio < 0.3f) {
			hpGaugeFill_->SetColor(hpDangerColor_); // 赤色
		} else if (hpRatio < 0.6f) {
			hpGaugeFill_->SetColor(hpWarningColor_); // 黄色
		} else {
			hpGaugeFill_->SetColor(hpNormalColor_); // 緑色
		}
	}

	// ENゲージの更新
	if (enGaugeFill_) {
		float enRatio = health.GetENRatio();
		Vector2 currentSize = enGaugeFill_->GetSize();
		currentSize.x = gaugeSize_.x * enRatio; // 最大幅に対する割合
		enGaugeFill_->SetSize(currentSize);

		// ENの量に応じて色を変更
		if (enRatio < 0.3f) {
			enGaugeFill_->SetColor(enLowColor_); // 暗い青色
		} else {
			enGaugeFill_->SetColor(enNormalColor_); // 通常の青色
		}
	}
}

void PlayerUI::SetGaugePosition(const Vector2& hpPosition, const Vector2& enPosition) {
	hpGaugePosition_ = hpPosition;
	enGaugePosition_ = enPosition;

	// 既存のスプライトがあれば位置を更新
	if (hpGaugeBar_) hpGaugeBar_->SetPosition(hpGaugePosition_);
	if (hpGaugeFill_) hpGaugeFill_->SetPosition({ hpGaugePosition_.x + 2.0f, hpGaugePosition_.y });
	if (enGaugeBar_) enGaugeBar_->SetPosition(enGaugePosition_);
	if (enGaugeFill_) enGaugeFill_->SetPosition({ enGaugePosition_.x + 2.0f, enGaugePosition_.y });
}

void PlayerUI::SetGaugeSize(const Vector2& size) {
	gaugeSize_ = size;
	gaugeFrameSize_ = { size.x + 4.0f, size.y + 4.0f };

	// 既存のスプライトがあればサイズを更新
	if (hpGaugeBar_) hpGaugeBar_->SetSize(gaugeFrameSize_);
	if (hpGaugeFill_) hpGaugeFill_->SetSize(gaugeSize_);
	if (enGaugeBar_) enGaugeBar_->SetSize(gaugeFrameSize_);
	if (enGaugeFill_) enGaugeFill_->SetSize(gaugeSize_);
}

void PlayerUI::SetGaugeColors(const Vector4& hpColor, const Vector4& enColor, const Vector4& backgroundColor) {
	hpNormalColor_ = hpColor;
	enNormalColor_ = enColor;
	backgroundColor_ = backgroundColor;

	// 色を更新
	if (hpGaugeBar_) hpGaugeBar_->SetColor(backgroundColor_);
	if (hpGaugeFill_) hpGaugeFill_->SetColor(hpNormalColor_);
	if (enGaugeBar_) enGaugeBar_->SetColor(backgroundColor_);
	if (enGaugeFill_) enGaugeFill_->SetColor(enNormalColor_);
}

void PlayerUI::ImGui() {
#ifdef _DEBUG
	if (ImGui::TreeNode("Player UI")) {
		// ゲージ位置設定
		if (ImGui::DragFloat2("HP Gauge Position", &hpGaugePosition_.x, 1.0f, 0.0f, 1280.0f)) {
			SetGaugePosition(hpGaugePosition_, enGaugePosition_);
		}
		if (ImGui::DragFloat2("EN Gauge Position", &enGaugePosition_.x, 1.0f, 0.0f, 720.0f)) {
			SetGaugePosition(hpGaugePosition_, enGaugePosition_);
		}

		// ゲージサイズ設定
		if (ImGui::DragFloat2("Gauge Size", &gaugeSize_.x, 1.0f, 10.0f, 400.0f)) {
			SetGaugeSize(gaugeSize_);
		}

		// 色設定
		ImGui::Separator();
		ImGui::ColorEdit4("HP Normal Color", &hpNormalColor_.x);
		ImGui::ColorEdit4("HP Warning Color", &hpWarningColor_.x);
		ImGui::ColorEdit4("HP Danger Color", &hpDangerColor_.x);
		ImGui::ColorEdit4("EN Normal Color", &enNormalColor_.x);
		ImGui::ColorEdit4("EN Low Color", &enLowColor_.x);
		ImGui::ColorEdit4("Background Color", &backgroundColor_.x);
		ImGui::TreePop();
	}
#endif
}