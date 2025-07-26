#pragma once
#include "OffscreenRenderer/PostEffect/PostEffect.h"
#include "Objects/Sprite/Sprite.h"
#include "MyMath/MyFunction.h"
#include "BaseSystem/Logger/Logger.h"

/// <summary>
/// ビネットポストエフェクト（OffscreenTriangle使用版）
/// </summary>
class VignettePostEffect : public PostEffect {
public:
	/// <summary>
	/// ビネット専用のパラメータ
	/// </summary>
	struct VignetteParameters {
		Vector4 vignetteColor = { 0.0f, 0.0f, 0.0f, 1.0f };		// ビネットの色（通常は黒色）

		float time = 0.0f;								// 時間（アニメーション用）
		float vignetteStrength = 0.6f;					// ビネットの強度 (0.0f～1.0f)
		float vignetteRadius = 0.4f;					// ビネットの半径 (0.0f～1.0f)
		float vignetteSoftness = 0.3f;					// ビネットの柔らかさ (0.0f～1.0f)
	};

	/// <summary>
	/// エフェクトプリセット
	/// </summary>
	enum class EffectPreset {
		OFF,            // エフェクトなし
		SUBTLE,         // 軽微なビネット
		MEDIUM,         // 中程度のビネット
		INTENSE,        // 強いビネット
		CINEMATIC,      // 映画的ビネット
	};

public:
	VignettePostEffect() { name_ = "Vignette Effect"; }
	~VignettePostEffect() = default;

	// PostEffect インターフェース実装
	void Initialize(DirectXCommon* dxCommon) override;
	void Finalize() override;
	void Update(float deltaTime) override;
	void Apply(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV, D3D12_CPU_DESCRIPTOR_HANDLE outputRTV, OffscreenTriangle* renderTriangle) override;
	bool IsEnabled() const override { return isEnabled_; }
	void SetEnabled(bool enabled) override { isEnabled_ = enabled; }
	void ImGui() override;
	const std::string& GetName() const override { return name_; }

	// 固有メソッド
	void ApplyPreset(EffectPreset preset);
	void SetVignetteStrength(float strength);
	void SetVignetteRadius(float radius);
	void SetVignetteSoftness(float softness);
	void SetVignetteColor(const Vector4& color);

	float GetVignetteStrength() const { return parameters_.vignetteStrength; }
	float GetVignetteRadius() const { return parameters_.vignetteRadius; }
	float GetVignetteSoftness() const { return parameters_.vignetteSoftness; }
	const Vector4& GetVignetteColor() const { return parameters_.vignetteColor; }

private:
	void CreatePSO();
	void CreateParameterBuffer();
	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const std::wstring& filePath, const wchar_t* profile);
	void UpdateParameterBuffer();

private:
	// エフェクトの状態
	bool isInitialized_ = false;

	// ビネットパラメータ
	VignetteParameters parameters_;

	// ビネットエフェクト用PSO
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob_;
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob_;

	// バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> parameterBuffer_;
	VignetteParameters* mappedParameters_ = nullptr;

	// アニメーション用
	float animationSpeed_ = 1.0f;
};