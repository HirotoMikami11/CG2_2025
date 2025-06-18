#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <wrl.h>
#include <string>

#include "DirectXCommon.h"
#include "Logger.h"
#include "MyMath.h"
#include "MyFunction.h"
#include "ImGuiManager.h" 

/// <summary>
/// RGBシフトのグリッチエフェクトクラス
/// </summary>
class GlitchEffect {
public:
	/// <summary>
	/// RGBシフト専用のパラメータ
	/// </summary>
	struct GlitchParameters {
		float rgbShiftStrength = 1.0f;		// RGBシフトの強度 (0.0f～10.0f)
		float time = 0.0f;					// 時間（アニメーション用）
		float unused1 = 0.0f;				// パディング
		float unused2 = 0.0f;				// パディング（16バイト境界）
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
	GlitchEffect() = default;
	~GlitchEffect() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// 更新処理（時間を進める）
	/// </summary>
	/// <param name="deltaTime">フレーム時間</param>
	void Update(float deltaTime = 1.0f / 60.0f);

	/// <summary>
	/// ImGui でのパラメータ調整UI
	/// </summary>
	void ImGui();



	// Getter

	/// <summary>
	/// エフェクトが有効かどうか
	/// </summary>
	bool IsEnabled() const { return isEnabled_; }

	/// <summary>
	/// グリッチ用ルートシグネチャを取得
	/// </summary>
	ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); }

	/// <summary>
	/// グリッチ用PSOを取得
	/// </summary>
	ID3D12PipelineState* GetPipelineState() const { return pipelineState_.Get(); }

	/// <summary>
	/// パラメータバッファを取得
	/// </summary>
	ID3D12Resource* GetMaterialBuffer() const { return parameterBuffer_.Get(); }

	/// <summary>
	/// パラメータデータを取得
	/// </summary>
	const GlitchParameters& GetParameters() const { return parameters_; }

	// Setter

	/// <summary>
	/// エフェクトの有効/無効を設定
	/// </summary>
	void SetEnabled(bool enabled) { isEnabled_ = enabled; }

	/// <summary>
	/// プリセットを適用
	/// </summary>
	void ApplyPreset(EffectPreset preset);

	/// <summary>
	/// RGBシフトの強度を設定
	/// </summary>
	void SetRGBShiftStrength(float strength) {
		parameters_.rgbShiftStrength = std::clamp(strength, 0.0f, 10.0f);
		UpdateParameterBuffer();
	}

private:
	// DirectXCommonへの参照
	DirectXCommon* dxCommon_ = nullptr;

	// エフェクトの状態
	bool isEnabled_ = false;
	bool isInitialized_ = false;

	// RGBシフトパラメータ
	GlitchParameters parameters_;

	// グリッチエフェクト用PSO
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob_;
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob_;

	// パラメータバッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> parameterBuffer_;
	GlitchParameters* mappedParameters_ = nullptr;

	// アニメーション用
	float animationSpeed_ = 1.0f;

private:
	/// <summary>
	/// グリッチエフェクト用のPSOを作成
	/// </summary>
	void CreatePSO();

	/// <summary>
	/// パラメータバッファを作成
	/// </summary>
	void CreateParameterBuffer();

	/// <summary>
	/// シェーダーをコンパイル
	/// </summary>
	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(
		const std::wstring& filePath,
		const wchar_t* profile);

	/// <summary>
	/// パラメータバッファを更新
	/// </summary>
	void UpdateParameterBuffer();

};