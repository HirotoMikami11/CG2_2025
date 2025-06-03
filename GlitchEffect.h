#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <cstddef>  // offsetof用に追加

#include "DirectXCommon.h"
#include "Logger.h"
#include "MyMath.h"
#include "MyFunction.h"

/// <summary>
/// グリッチエフェクトを管理するクラス
/// </summary>
class GlitchEffect {
public:
	/// <summary>
	/// グリッチエフェクト用のマテリアルデータ（シェーダーと対応）
	/// </summary>
	struct GlitchMaterialData {
		Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
		int32_t enableLighting = 0;           // 使わない
		int32_t useLambertianReflectance = 0; // 使わない
		Matrix4x4 uvTransform = MakeIdentity4x4();

		// グリッチエフェクト用パラメータ
		float rgbShiftStrength = 1.0f;        // RGBシフトの強度 (0.0f～5.0f)
		float time = 0.0f;                    // 時間（アニメーション用）
		float glitchIntensity = 0.5f;         // 全体的なグリッチ強度 (0.0f～1.0f)
		float unused = 0.0f;                  // パディング用
	};

	/// <summary>
	/// エフェクトプリセット
	/// </summary>
	enum class EffectPreset {
		OFF,           // エフェクトなし
		SUBTLE,        // 軽微なグリッチ
		MEDIUM,        // 中程度のグリッチ
		INTENSE,       // 強烈なグリッチ
		CHAOS          // 混沌
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
	/// グリッチエフェクト用のPSOを作成
	/// </summary>
	void CreatePSO();

	/// <summary>
	/// グリッチエフェクト用の定数バッファを作成
	/// </summary>
	void CreateConstantBuffer();

	/// <summary>
	/// ImGui でのパラメータ調整UI
	/// </summary>
	void ImGui();

	//=============================================================================
	// Getter
	//=============================================================================

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
	/// マテリアルバッファを取得
	/// </summary>
	ID3D12Resource* GetMaterialBuffer() const { return materialBuffer_.Get(); }

	/// <summary>
	/// マテリアルデータを取得
	/// </summary>
	const GlitchMaterialData& GetMaterialData() const { return materialData_; }

	//=============================================================================
	// Setter
	//=============================================================================

	/// <summary>
	/// エフェクトの有効/無効を設定
	/// </summary>
	void SetEnabled(bool enabled) { isEnabled_ = enabled; }

	/// <summary>
	/// RGBシフトの強度を設定
	/// </summary>
	void SetRGBShiftStrength(float strength) {
		materialData_.rgbShiftStrength = std::clamp(strength, 0.0f, 10.0f);
		UpdateConstantBuffer();
	}

	/// <summary>
	/// グリッチ強度を設定
	/// </summary>
	void SetGlitchIntensity(float intensity) {
		materialData_.glitchIntensity = std::clamp(intensity, 0.0f, 1.0f);
		UpdateConstantBuffer();
	}

	/// <summary>
	/// 色を設定
	/// </summary>
	void SetColor(const Vector4& color) {
		materialData_.color = color;
		UpdateConstantBuffer();
	}

	/// <summary>
	/// UVトランスフォームを設定
	/// </summary>
	void SetUVTransform(const Matrix4x4& transform) {
		materialData_.uvTransform = transform;
		UpdateConstantBuffer();
	}

private:
	/// <summary>
	/// シェーダーをコンパイル
	/// </summary>
	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(
		const std::wstring& filePath,
		const wchar_t* profile);

	/// <summary>
	/// 定数バッファを更新
	/// </summary>
	void UpdateConstantBuffer();

private:
	// DirectXCommonへの参照
	DirectXCommon* dxCommon_ = nullptr;

	// エフェクトの状態
	bool isEnabled_ = false;
	bool isInitialized_ = false;

	// マテリアルデータ
	GlitchMaterialData materialData_;

	// グリッチエフェクト用PSO
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob_;
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob_;

	// 定数バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> materialBuffer_;
	GlitchMaterialData* mappedMaterialData_ = nullptr;

	// アニメーション用
	float animationSpeed_ = 1.0f;

};