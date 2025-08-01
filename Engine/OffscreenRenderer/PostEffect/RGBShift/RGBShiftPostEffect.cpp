#include "RGBShiftPostEffect.h"
#include "Managers/ImGui/ImGuiManager.h" 

void RGBShiftPostEffect::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;

	// PSOを作成
	CreatePSO();

	// パラメータバッファを作成
	CreateParameterBuffer();

	isInitialized_ = true;

	Logger::Log(Logger::GetStream(), "RGBShiftPostEffect initialized successfully (OffscreenTriangle version)!\n");
}

void RGBShiftPostEffect::Finalize() {
	// パラメータデータのマッピング解除
	if (mappedParameters_) {
		parameterBuffer_->Unmap(0, nullptr);
		mappedParameters_ = nullptr;
	}

	isInitialized_ = false;
	Logger::Log(Logger::GetStream(), "RGBShiftPostEffect finalized (OffscreenTriangle version).\n");
}

void RGBShiftPostEffect::Update(float deltaTime) {
	if (!isEnabled_ || !isInitialized_) {
		return;
	}

	// 時間を更新
	parameters_.time += deltaTime * animationSpeed_;

	// パラメータバッファを更新
	UpdateParameterBuffer();
}

void RGBShiftPostEffect::Apply(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV, D3D12_CPU_DESCRIPTOR_HANDLE outputRTV, OffscreenTriangle* renderTriangle) {
	if (!isEnabled_ || !isInitialized_ || !renderTriangle) {
		return;
	}

	auto commandList = dxCommon_->GetCommandList();

	// レンダーターゲットを設定
	commandList->OMSetRenderTargets(1, &outputRTV, false, nullptr);

	// クリア
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	commandList->ClearRenderTargetView(outputRTV, clearColor, 0, nullptr);

	// ディスクリプタヒープを設定
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeaps[] = {
		dxCommon_->GetDescriptorManager()->GetSRVHeapComPtr()
	};
	commandList->SetDescriptorHeaps(1, descriptorHeaps->GetAddressOf());

	// OffscreenTriangleを使用してカスタムPSOで描画
	renderTriangle->DrawWithCustomPSO(
		rootSignature_.Get(),
		pipelineState_.Get(),
		inputSRV,
		parameterBuffer_->GetGPUVirtualAddress()
	);
}

void RGBShiftPostEffect::CreatePSO() {
	// RootSignatureを構築
	RootSignatureBuilder rsBuilder;
	rsBuilder.AddCBV(0, D3D12_SHADER_VISIBILITY_PIXEL)		// RGBShiftParameters (b0)
		.AddSRV(0, 1, D3D12_SHADER_VISIBILITY_PIXEL)		// Texture (t0)
		.AddStaticSampler(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

	// PSO設定を構築
	auto psoDesc = PSODescriptor::CreatePostEffectColorOnly()
		.SetPixelShader(L"resources/Shader/RGBShift/RGBShift.PS.hlsl");

	// PSOFactory経由で生成
	auto psoInfo = dxCommon_->GetPSOFactory()->CreatePSO(psoDesc, rsBuilder);
	if (!psoInfo.IsValid()) {
		Logger::Log(Logger::GetStream(), "RGBShiftPostEffect: Failed to create PSO\n");
		assert(false);
	}

	rootSignature_ = psoInfo.rootSignature;
	pipelineState_ = psoInfo.pipelineState;

	Logger::Log(Logger::GetStream(), "Complete create RGB Shift PSO (PSOFactory version)!!\n");
}

void RGBShiftPostEffect::CreateParameterBuffer() {
	// 構造体サイズをデバッグ出力
	size_t structSize = sizeof(RGBShiftParameters);
	Logger::Log(Logger::GetStream(), std::format("RGBShiftParameters size: {} bytes\n", structSize));

	// 256バイト境界に揃える（DirectX12の要件）
	size_t alignedSize = (structSize + 255) & ~255;
	Logger::Log(Logger::GetStream(), std::format("Aligned buffer size: {} bytes\n", alignedSize));

	// パラメータバッファ作成
	parameterBuffer_ = CreateBufferResource(dxCommon_->GetDeviceComPtr(), alignedSize);
	parameterBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedParameters_));

	// 初期データを設定
	UpdateParameterBuffer();

	Logger::Log(Logger::GetStream(), "Complete create RGB Shift parameter buffer (OffscreenTriangle version)!!\n");
}

Microsoft::WRL::ComPtr<IDxcBlob> RGBShiftPostEffect::CompileShader(
	const std::wstring& filePath, const wchar_t* profile) {

	// DirectXCommonのCompileShader関数を使用
	return DirectXCommon::CompileShader(
		filePath,
		profile,
		dxCommon_->GetDxcUtils(),
		dxCommon_->GetDxcCompiler(),
		dxCommon_->GetIncludeHandler());
}

void RGBShiftPostEffect::UpdateParameterBuffer() {
	if (mappedParameters_) {
		*mappedParameters_ = parameters_;
	}
}

void RGBShiftPostEffect::ApplyPreset(EffectPreset preset) {
	switch (preset) {
	case EffectPreset::OFF:
		SetEnabled(false);
		break;

	case EffectPreset::SUBTLE:
		parameters_.rgbShiftStrength = 0.5f;
		SetEnabled(true);
		break;

	case EffectPreset::MEDIUM:
		parameters_.rgbShiftStrength = 1.5f;
		SetEnabled(true);
		break;

	case EffectPreset::INTENSE:
		parameters_.rgbShiftStrength = 3.0f;
		SetEnabled(true);
		break;
	}

	UpdateParameterBuffer();
}

void RGBShiftPostEffect::SetRGBShiftStrength(float strength) {
	parameters_.rgbShiftStrength = std::clamp(strength, 0.0f, 10.0f);
	UpdateParameterBuffer();
}

void RGBShiftPostEffect::ImGui() {
#ifdef _DEBUG
	if (ImGui::CollapsingHeader(name_.c_str())) {
		// エフェクトの状態表示
		ImGui::Text("Effect Status: %s", isEnabled_ ? "ENABLED" : "DISABLED");
		ImGui::Text("Initialized: %s", isInitialized_ ? "YES" : "NO");

		if (isEnabled_) {
			// プリセット選択
			if (ImGui::TreeNode("Presets")) {
				if (ImGui::Button("Subtle")) ApplyPreset(EffectPreset::SUBTLE);
				ImGui::SameLine();
				if (ImGui::Button("Medium")) ApplyPreset(EffectPreset::MEDIUM);
				ImGui::SameLine();
				if (ImGui::Button("Intense")) ApplyPreset(EffectPreset::INTENSE);

				ImGui::TreePop();
			}

			// 個別パラメータ調整
			if (ImGui::TreeNode("Manual Settings")) {
				float rgbShiftStrength = parameters_.rgbShiftStrength;
				if (ImGui::SliderFloat("RGB Shift Strength", &rgbShiftStrength, 0.0f, 5.0f)) {
					SetRGBShiftStrength(rgbShiftStrength);
				}

				if (ImGui::SliderFloat("Animation Speed", &animationSpeed_, 0.0f, 3.0f)) {
				}

				ImGui::TreePop();
			}

			// 情報表示
			ImGui::Separator();
			ImGui::Text("Current Time: %.2f", parameters_.time);
		}

	}
#endif
}