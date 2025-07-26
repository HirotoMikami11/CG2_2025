#pragma once
#include "OffscreenRenderer/PostEffect/PostEffect.h"
#include "MyMath/MyFunction.h"
#include "BaseSystem/Logger/Logger.h"

/// <summary>
/// RGBシフトポストエフェクト（OffscreenTriangle使用版）
/// </summary>
class RGBShiftPostEffect : public PostEffect {
public:
	/// <summary>
	/// RGBシフト専用のパラメータ
	/// </summary>
	struct RGBShiftParameters {
		float rgbShiftStrength = 1.0f;		// RGBシフトの強度
		float time = 0.0f;					// 時間（アニメーション用）
		float unused1 = 0.0f;				// パディング
		float unused2 = 0.0f;				// パディング（16バイト境界合わせ）
	};

	/// <summary>
	/// エフェクトプリセット
	/// </summary>
	enum class EffectPreset {
		OFF,			// エフェクトなし
		SUBTLE,			// 軽微なRGBシフト
		MEDIUM,			// 中程度のRGBシフト
		INTENSE,		// 強烈なRGBシフト
	};

public:
	RGBShiftPostEffect() { name_ = "RGB Shift Effect"; }
	~RGBShiftPostEffect() = default;

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
	void SetRGBShiftStrength(float strength);
	float GetRGBShiftStrength() const { return parameters_.rgbShiftStrength; }

private:
	void CreatePSO();
	void CreateParameterBuffer();
	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const std::wstring& filePath, const wchar_t* profile);
	void UpdateParameterBuffer();

private:
	// エフェクトの状態
	bool isInitialized_ = false;

	// RGBシフトパラメータ
	RGBShiftParameters parameters_;

	// RGBシフトエフェクト用PSO
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob_;
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob_;

	// バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> parameterBuffer_;
	RGBShiftParameters* mappedParameters_ = nullptr;

	// アニメーション用
	float animationSpeed_ = 1.0f;
};