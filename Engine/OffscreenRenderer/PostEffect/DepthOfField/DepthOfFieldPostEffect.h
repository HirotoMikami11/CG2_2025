#pragma once
#include "OffscreenRenderer/PostEffect/PostEffect.h"
#include "MyMath/MyFunction.h"
#include "BaseSystem/Logger/Logger.h"

/// <summary>
/// 深度被写界深度ポストエフェクト（OffscreenTriangle使用版）
/// </summary>
class DepthOfFieldPostEffect : public PostEffect {
public:
	/// <summary>
	/// 被写界深度専用のパラメータ
	/// </summary>
	struct DepthOfFieldParameters {
		Vector4 focusParams = { 10.0f, 5.0f, 1.0f, 0.0f };	// x:焦点距離, y:焦点範囲, z:ボケ強度, w:未使用

		float focusDistance = 3.0f;		// 焦点距離
		float focusRange = 11.0f;			// 焦点範囲（この範囲内はシャープ）
		float blurStrength = 2.0f;			// ボケの強度
		float time = 0.0f;					// アニメーション用時間
	};

	//カメラから何単位離れた場所にピントを合わせるかがDistance
	//focusDistanceが10の時、カメラから10離れた先にピントが合う

	//ピントが合ってシャープに見える範囲の幅がRange
	//focusRangeが10の時、焦点距離(Distance)から+-10の範囲にピントが合い鮮明に見える

	/// <summary>
	/// エフェクトプリセット
	/// </summary>
	enum class EffectPreset {
		NONE,			// エフェクトなし
	};

public:
	DepthOfFieldPostEffect() { name_ = "Depth of Field Effect"; }
	~DepthOfFieldPostEffect() = default;

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
	void SetFocusDistance(float distance);
	void SetFocusRange(float range);
	void SetBlurStrength(float strength);

	float GetFocusDistance() const { return parameters_.focusDistance; }
	float GetFocusRange() const { return parameters_.focusRange; }
	float GetBlurStrength() const { return parameters_.blurStrength; }

private:
	void CreatePSO();
	void CreateParameterBuffer();
	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const std::wstring& filePath, const wchar_t* profile);
	void UpdateParameterBuffer();

private:
	// エフェクトの状態
	bool isInitialized_ = false;

	// 被写界深度パラメータ
	DepthOfFieldParameters parameters_;

	// 被写界深度エフェクト用PSO
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob_;
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob_;

	// バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> parameterBuffer_;
	DepthOfFieldParameters* mappedParameters_ = nullptr;

	// アニメーション用
	float animationSpeed_ = 1.0f;
};