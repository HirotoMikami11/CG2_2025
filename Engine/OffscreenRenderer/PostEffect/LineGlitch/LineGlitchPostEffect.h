#pragma once
#include "OffscreenRenderer/PostEffect/PostEffect.h"
#include "Objects/Sprite/Sprite.h"
#include "MyMath/MyFunction.h"
#include "BaseSystem/Logger/Logger.h"

/// <summary>
/// ラインをずらすグリッチエフェクト（Sprite使用版）
/// </summary>
class LineGlitchPostEffect : public PostEffect {
public:
	/// <summary>
	/// ライングリッチ専用のパラメータ
	/// </summary>
	struct LineGlitchParameters {
		float time = 0.0f;					// 時間（アニメーション用）
		float noiseIntensity = 1.0f;		// グリッチの強度 (0.0f～10.0f)
		float noiseInterval = 0.8f;			// ノイズが起こる頻度(0近ければ近いほど起こりやすく、増やすほど起こりにくくなる)
		float animationSpeed = 1.0f;		// 全体に適応する時間(内部時間にかける値)
	};

	/// <summary>
	/// エフェクトプリセット
	/// </summary>
	enum class EffectPreset {
		OFF,			// エフェクトなし
		SUBTLE,			// 軽微なライングリッチ
		MEDIUM,			// 中程度のライングリッチ
		INTENSE,		// 強烈なライングリッチ
	};

public:
	LineGlitchPostEffect() { name_ = "LineGlitch Effect"; }
	~LineGlitchPostEffect() = default;

	// PostEffect インターフェース実装
	void Initialize(DirectXCommon* dxCommon) override;
	void Finalize() override;
	void Update(float deltaTime) override;
	void Apply(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV, D3D12_CPU_DESCRIPTOR_HANDLE outputRTV, Sprite* renderSprite) override;
	bool IsEnabled() const override { return isEnabled_; }
	void SetEnabled(bool enabled) override { isEnabled_ = enabled; }
	void ImGui() override;
	const std::string& GetName() const override { return name_; }

	// 固有メソッド
	void ApplyPreset(EffectPreset preset);
	void SetNoiseIntensity(float intensity);
	void SetNoiseInterval(float interval);

private:
	void CreatePSO();
	void CreateParameterBuffer();
	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const std::wstring& filePath, const wchar_t* profile);
	void UpdateParameterBuffer();

private:
	// エフェクトの状態
	bool isInitialized_ = false;

	// ライングリッチパラメータ
	LineGlitchParameters parameters_;

	// グリッチエフェクト用PSO
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob_;
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob_;

	// バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> parameterBuffer_;
	LineGlitchParameters* mappedParameters_ = nullptr;

	// アニメーション用
	float animationSpeed_ = 1.0f;
};