#pragma once
#include "OffscreenRenderer/PostEffect/PostEffect.h"
#include "MyMath/MyFunction.h"
#include "BaseSystem/Logger/Logger.h"

/// <summary>
/// 深度フォグポストエフェクト（OffscreenTriangle使用版）
/// </summary>
class DepthFogPostEffect : public PostEffect {
public:
	/// <summary>
	/// 深度フォグ専用のパラメータ
	/// </summary>
	struct DepthFogParameters {
		Vector4 fogColor = { 0.02f, 0.08f, 0.25f, 1.00f };		// フォグの色

		float fogNear = 0.2f;								// フォグ開始距離（非線形変換対応）
		float fogFar = 40.0f;								// フォグ終了距離（非線形変換対応）
		float fogDensity = 1.0f;							// フォグの密度 (0.0f～1.0f)
		float time = 0.0f;									// アニメーション用時間
	};

	/// <summary>
	/// エフェクトプリセット
	/// </summary>
	enum class EffectPreset {
		NONE,			// エフェクトなし
		LIGHT,			// 薄いフォグ
		HEAVY,			// 濃いフォグ
		UNDERWATER		// 水中エフェクト（青いフォグ）
	};

public:
	DepthFogPostEffect() { name_ = "Depth Fog Effect"; }
	~DepthFogPostEffect() = default;

	// PostEffect インターフェース実装
	void Initialize(DirectXCommon* dxCommon) override;
	void Finalize() override;
	void Update(float deltaTime) override;
	void Apply(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV, D3D12_CPU_DESCRIPTOR_HANDLE outputRTV, OffscreenTriangle* renderTriangle) override;

	/// <summary>
	/// 深度テクスチャも受け取る専用Apply
	/// </summary>
	/// <param name="inputSRV">入力カラーテクスチャのSRV</param>
	/// <param name="depthSRV">深度テクスチャのSRV</param>
	/// <param name="outputRTV">出力先のRTV</param>
	/// <param name="renderTriangle">描画用三角形</param>
	void Apply(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV, D3D12_GPU_DESCRIPTOR_HANDLE depthSRV, D3D12_CPU_DESCRIPTOR_HANDLE outputRTV, OffscreenTriangle* renderTriangle);

	bool IsEnabled() const override { return isEnabled_; }
	void SetEnabled(bool enabled) override { isEnabled_ = enabled; }
	void ImGui() override;
	const std::string& GetName() const override { return name_; }
	// 深度が必要なのでtrueでオーバーライド
	bool RequiresDepthTexture() const override { return true; }

	// 固有メソッド
	void ApplyPreset(EffectPreset preset);
	void SetFogColor(const Vector4& color);
	void SetFogDistance(float fogNear, float fogFar);
	void SetFogDensity(float density);
	float GetFogNear() const { return parameters_.fogNear; }
	float GetFogFar() const { return parameters_.fogFar; }
	float GetFogDensity() const { return parameters_.fogDensity; }
	Vector4 GetFogColor() const { return parameters_.fogColor; }

private:
	void CreatePSO();
	void CreateParameterBuffer();
	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const std::wstring& filePath, const wchar_t* profile);
	void UpdateParameterBuffer();

private:
	// エフェクトの状態
	bool isInitialized_ = false;

	// 深度フォグパラメータ
	DepthFogParameters parameters_;

	// 深度フォグエフェクト用PSO
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob_;
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob_;

	// バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> parameterBuffer_;
	DepthFogParameters* mappedParameters_ = nullptr;

	// アニメーション用
	float animationSpeed_ = 1.0f;
};