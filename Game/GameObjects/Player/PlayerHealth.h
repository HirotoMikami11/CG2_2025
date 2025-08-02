#pragma once

/// <summary>
/// プレイヤーの体力・エネルギー管理クラス
/// </summary>
class PlayerHealth {
public:
	PlayerHealth();
	~PlayerHealth() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新処理（エネルギー回復など）
	/// </summary>
	void Update();

	/// <summary>
	/// ImGui表示
	/// </summary>
	void ImGui();

	// HP関連
	float GetCurrentHP() const { return currentHP_; }
	float GetMaxHP() const { return maxHP_; }
	float GetHPRatio() const { return currentHP_ / maxHP_; }

	void SetMaxHP(float maxHP) { maxHP_ = maxHP; }
	void TakeDamage(float damage);
	void Heal(float amount);
	bool IsDead() const { return currentHP_ <= 0.0f; }

	// EN（エネルギー）関連
	float GetCurrentEN() const { return currentEN_; }
	float GetMaxEN() const { return maxEN_; }
	float GetENRatio() const { return currentEN_ / maxEN_; }

	void SetMaxEN(float maxEN) { maxEN_ = maxEN; }
	bool ConsumeEnergy(float amount);
	void RestoreEnergy(float amount);
	bool HasEnoughEnergy(float required) const { return currentEN_ >= required; }

	// エネルギー設定
	void SetEnergyCostPerShot(float cost) { energyCostPerShot_ = cost; }
	void SetEnergyRegenRate(float rate) { energyRegenRate_ = rate; }
	void SetEnergyRegenDelay(int delay) { energyRegenDelay_ = delay; }

	float GetEnergyCostPerShot() const { return energyCostPerShot_; }

	/// <summary>
	/// エネルギー回復タイマーをリセット（攻撃時などに呼び出し）
	/// </summary>
	void ResetEnergyRegenTimer() { energyRegenTimer_ = energyRegenDelay_; }

private:
	// HP/ENシステム
	float maxHP_ = 100.0f;				// 最大HP（酸素）
	float currentHP_ = 100.0f;			// 現在のHP
	float maxEN_ = 100.0f;				// 最大EN（エネルギー）
	float currentEN_ = 100.0f;			 // 現在のEN

	float energyCostPerShot_ = 0.5f;	// 1発撃つごとのエネルギー消費量
	float energyRegenRate_ = 1.0f;		// エネルギー回復速度（毎フレーム）
	int energyRegenDelay_ = 60;			// エネルギー回復開始までの待機時間（フレーム）
	int energyRegenTimer_ = 0;			// エネルギー回復タイマー

	/// <summary>
	/// エネルギーの更新処理
	/// </summary>
	void UpdateEnergy();
};