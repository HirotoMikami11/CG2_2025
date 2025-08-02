#define NOMINMAX
#include "PlayerHealth.h"
#include "Managers/ImGui/ImGuiManager.h"
#include <algorithm>

PlayerHealth::PlayerHealth() {
	Initialize();
}

void PlayerHealth::Initialize() {
	currentHP_ = maxHP_;
	currentEN_ = maxEN_;
	energyRegenTimer_ = 0;
}

void PlayerHealth::Update() {
	UpdateEnergy();
}

void PlayerHealth::TakeDamage(float damage) {
	currentHP_ -= damage;
	currentHP_ = std::max(0.0f, currentHP_); // 0以下にならないように
}

void PlayerHealth::Heal(float amount) {
	currentHP_ += amount;
	currentHP_ = std::min(currentHP_, maxHP_); // 最大値を超えないように
}

bool PlayerHealth::ConsumeEnergy(float amount) {
	if (currentEN_ < amount) {
		return false; // エネルギー不足
	}

	currentEN_ -= amount;
	currentEN_ = std::max(0.0f, currentEN_); // 0以下にならないように

	// エネルギー回復タイマーをリセット
	ResetEnergyRegenTimer();

	return true;
}

void PlayerHealth::RestoreEnergy(float amount) {
	currentEN_ += amount;
	currentEN_ = std::min(currentEN_, maxEN_); // 最大値を超えないように
}

void PlayerHealth::UpdateEnergy() {
	// エネルギー回復タイマーを更新
	if (energyRegenTimer_ > 0) {
		energyRegenTimer_--;
	}

	// タイマーが0になったらエネルギーを回復
	if (energyRegenTimer_ <= 0 && currentEN_ < maxEN_) {
		currentEN_ += energyRegenRate_;
		currentEN_ = std::min(currentEN_, maxEN_); // 最大値を超えないように
	}
}

void PlayerHealth::ImGui() {
#ifdef _DEBUG
	if (ImGui::TreeNode("Health System")) {
		// HP情報
		ImGui::Text("HP: %.1f / %.1f", currentHP_, maxHP_);
		ImGui::ProgressBar(GetHPRatio(), ImVec2(200, 20), "HP");

		if (ImGui::DragFloat("Max HP", &maxHP_, 1.0f, 1.0f, 1000.0f)) {
			// 最大HPが変更された場合、現在HPも調整
			currentHP_ = std::min(currentHP_, maxHP_);
		}

		// EN情報
		ImGui::Separator();
		ImGui::Text("EN: %.1f / %.1f", currentEN_, maxEN_);
		ImGui::ProgressBar(GetENRatio(), ImVec2(200, 20), "EN");
		ImGui::Text("Energy Regen Timer: %d", energyRegenTimer_);

		if (ImGui::DragFloat("Max EN", &maxEN_, 1.0f, 1.0f, 1000.0f)) {
			// 最大ENが変更された場合、現在ENも調整
			currentEN_ = std::min(currentEN_, maxEN_);
		}

		// エネルギー設定
		ImGui::Separator();
		ImGui::DragFloat("Energy Cost Per Shot", &energyCostPerShot_, 0.1f, 0.0f, 10.0f);
		ImGui::DragFloat("Energy Regen Rate", &energyRegenRate_, 0.1f, 0.0f, 10.0f);
		ImGui::DragInt("Energy Regen Delay", &energyRegenDelay_, 1, 0, 300);

		// テスト用ボタン
		ImGui::Separator();
		if (ImGui::Button("Take Damage (10)")) {
			TakeDamage(10.0f);
		}
		ImGui::SameLine();
		if (ImGui::Button("Heal (10)")) {
			Heal(10.0f);
		}
		ImGui::SameLine();
		if (ImGui::Button("Consume Energy (5)")) {
			ConsumeEnergy(5.0f);
		}

		ImGui::TreePop();
	}
#endif
}