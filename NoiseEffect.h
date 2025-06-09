#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <cstddef>
#include <random>

#include "DirectXCommon.h"
#include "Logger.h"
#include "MyMath.h"
#include "MyFunction.h"

/// <summary>
/// ノイズエフェクトを管理するクラス
/// </summary>
class NoiseEffect {
public:
	/// <summary>
	/// ノイズエフェクト用のマテリアルデータ（シェーダーと対応）
	/// </summary>
	struct NoiseMaterialData {
		//もともとある奴らのデータ
		Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };					// 16 bytes (0-15)

		int32_t enableLighting = 0;									// 4 bytes  (16-19)
		int32_t useLambertianReflectance = 0;						// 4 bytes  (20-23)
		Vector2 padding1 = { 0.0f, 0.0f };							// 8 bytes  (24-31) パディング

		Matrix4x4 uvTransform = MakeIdentity4x4();					// 64 bytes (32-95)

		// ノイズエフェクト用パラメータ
		float time = 0.0f;											// 4 bytes  (96-99)			//受け取る時間
		float noiseIntensity = 0.5f;								// 4 bytes  (100-103)		//ノイズの強度
		float noiseInterval = 0.0f;									// 4 bytes  (104-107)		//演出間隔
		float animationSpeed = 1.0f;								// 4 bytes  (108-111)		//アニメーションの速さ

		// 16 bytes (112-127)
		float blockIntensity = 0.2f;											//ブロックずらしの強さ
		float blockProbability = 0.05f;											//ブロックずらしの確率
		float reverseProbability = 0.01f;										//ブロック反転の確率
		float scanLineProbability = 0.03f;										//走査線の確率

		float blackIntensity = 0.2f;							// 4 bytes  (128-131)		//グレー―スケールの強さ
		float colorNoiseInternsity = 0.3f;						// 4 bytes  (132-135)		//
		Vector2	blockDivision = { 0.0f, 0.0f };					// 8 bytes  (136-143)	//ブロックずらしの画面分割数
	};

	/// <summary>
	/// エフェクトプリセット
	/// </summary>
	enum class NoisePreset {
		OFF,           // エフェクトなし
		SUBTLE,        // 軽微なノイズ
		MEDIUM,        // 中程度のノイズ
		HEAVY,         // 重いノイズ
		DIGITAL_RAIN   // デジタルレインのような効果
	};

public:
	NoiseEffect() = default;
	~NoiseEffect() = default;

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
	/// ノイズエフェクト用のPSOを作成
	/// </summary>
	void CreatePSO();

	/// <summary>
	/// ノイズエフェクト用の定数バッファを作成
	/// </summary>
	void CreateConstantBuffer();

	/// <summary>
	/// ImGui でのパラメータ調整UI
	/// </summary>
	void ImGui();

	/// <summary>
	/// プリセットを適用
	/// </summary>
	void ApplyPreset(NoisePreset preset);

	//=============================================================================
	// Getter
	//=============================================================================

	/// <summary>
	/// エフェクトが有効かどうか
	/// </summary>
	bool IsEnabled() const { return isEnabled_; }

	/// <summary>
	/// ノイズ用ルートシグネチャを取得
	/// </summary>
	ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); }

	/// <summary>
	/// ノイズ用PSOを取得
	/// </summary>
	ID3D12PipelineState* GetPipelineState() const { return pipelineState_.Get(); }

	/// <summary>
	/// マテリアルバッファを取得
	/// </summary>
	ID3D12Resource* GetMaterialBuffer() const { return materialBuffer_.Get(); }

	/// <summary>
	/// マテリアルデータを取得
	/// </summary>
	const NoiseMaterialData& GetMaterialData() const { return materialData_; }

	//=============================================================================
	// Setter
	//=============================================================================

	/// <summary>
	/// エフェクトの有効/無効を設定
	/// </summary>
	void SetEnabled(bool enabled) { isEnabled_ = enabled; }

	/// <summary>
	/// ノイズ強度を設定
	/// </summary>
	void SetNoiseIntensity(float intensity) {
		materialData_.noiseIntensity = std::clamp(intensity, 0.0f, 1.0f);
		UpdateConstantBuffer();
	}

	/// <summary>
	/// ブロックサイズを設定
	/// </summary>
	void SetNoiseInterval(float size) {
		materialData_.noiseInterval = std::clamp(size, 0.0f, 1.0f);
		UpdateConstantBuffer();
	}

	/// <summary>
	/// ノイズの色を設定
	/// </summary>


	/// <summary>
	/// ブロック密度を設定
	/// </summary>
	void SetcolorNoiseInternsity(float density) {
		materialData_.colorNoiseInternsity = std::clamp(density, 0.0f, 1.0f);
		UpdateConstantBuffer();
	}

	/// <summary>
	/// ブロックずらしの画面分割数を設定
	/// </summary>
	/// <param name="division"></param>
	void SetBlockDivision(Vector2 division) {
		materialData_.blockDivision.x = std::clamp(division.x, 0.0f, 50.0f);		//
		materialData_.blockDivision.y = std::clamp(division.y, 0.0f, 30.0f);		//
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
	NoiseMaterialData materialData_;

	// ノイズエフェクト用PSO
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob_;
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob_;

	// 定数バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> materialBuffer_;
	NoiseMaterialData* mappedMaterialData_ = nullptr;

	// アニメーション用
	float animationSpeed_ = 1.0f;

	// ランダム生成用
	std::random_device randomDevice_;
	std::mt19937 randomEngine_;
};