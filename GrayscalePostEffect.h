#pragma once
#include "PostEffect.h"
#include "Sprite.h"
#include "MyFunction.h"
#include "BaseSystem/Logger/Logger.h"
#include "ImGuiManager.h"

/// <summary>
/// グレースケールポストエフェクト（Sprite使用版）
/// </summary>
class GrayscalePostEffect : public PostEffect {
public:
	/// <summary>
	/// グレースケール専用のパラメータ
	/// </summary>
	struct GrayscaleParameters {
		Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };		// カラー

		float time = 0.0f;								// 時間（アニメーション用）
		float grayIntensity = 0.0f;						// グレースケールの度合い (0.0f～1.0f)
		float unused1 = 0.0f;							// パディング
		float unused2 = 0.0f;							// パディング（16バイト境界）
	};

	/// <summary>
	/// エフェクトプリセット
	/// </summary>
	enum class EffectPreset {
		OFF,            // エフェクトなし
		SUBTLE,         // 軽微なグレースケール
		MEDIUM,         // 中程度のグレースケール
		INTENSE,        // 完全グレースケール
	};

public:
	GrayscalePostEffect() { name_ = "Grayscale Effect"; }
	~GrayscalePostEffect() = default;

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
	void SetGrayIntensity(float intensity);
	float GetGrayIntensity() const { return parameters_.grayIntensity; }

private:
	void CreatePSO();
	void CreateParameterBuffer();
	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const std::wstring& filePath, const wchar_t* profile);
	void UpdateParameterBuffer();

private:
	// エフェクトの状態
	bool isInitialized_ = false;

	// グレースケールパラメータ
	GrayscaleParameters parameters_;

	// グレースケールエフェクト用PSO
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob_;
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob_;

	// バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> parameterBuffer_;
	GrayscaleParameters* mappedParameters_ = nullptr;

	// アニメーション用
	float animationSpeed_ = 1.0f;
};