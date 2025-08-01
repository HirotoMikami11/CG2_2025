#include "LineGlitchPostEffect.h"
#include "Managers/ImGui/ImGuiManager.h" 

void LineGlitchPostEffect::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;

	// PSOを作成
	CreatePSO();

	// パラメータバッファを作成
	CreateParameterBuffer();

	isInitialized_ = true;

	Logger::Log(Logger::GetStream(), "LineGlitchPostEffect initialized successfully (OffscreenTriangle version)!\n");
}

void LineGlitchPostEffect::Finalize() {
	// パラメータデータのマッピング解除
	if (mappedParameters_) {
		parameterBuffer_->Unmap(0, nullptr);
		mappedParameters_ = nullptr;
	}

	isInitialized_ = false;
	Logger::Log(Logger::GetStream(), "LineGlitchPostEffect finalized (OffscreenTriangle version).\n");
}

void LineGlitchPostEffect::Update(float deltaTime) {
	if (!isEnabled_ || !isInitialized_) {
		return;
	}

	// 時間を更新
	parameters_.time += deltaTime * animationSpeed_;

	// パラメータバッファを更新
	UpdateParameterBuffer();
}

void LineGlitchPostEffect::Apply(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV, D3D12_CPU_DESCRIPTOR_HANDLE outputRTV, OffscreenTriangle* renderTriangle) {
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

void LineGlitchPostEffect::CreatePSO() {
	// RootSignatureを構築
	RootSignatureBuilder rsBuilder;
	rsBuilder.AddCBV(0, D3D12_SHADER_VISIBILITY_PIXEL)		// LineGlitchParameters (b0)
		.AddSRV(0, 1, D3D12_SHADER_VISIBILITY_PIXEL)		// Texture (t0)
		.AddStaticSampler(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

	// PSO設定を構築
	auto psoDesc = PSODescriptor::CreatePostEffectColorOnly()
		.SetPixelShader(L"resources/Shader/LineGlitch/LineGlitch.PS.hlsl");

	// PSOFactory経由で生成
	auto psoInfo = dxCommon_->GetPSOFactory()->CreatePSO(psoDesc, rsBuilder);
	if (!psoInfo.IsValid()) {
		Logger::Log(Logger::GetStream(), "LineGlitchPostEffect: Failed to create PSO\n");
		assert(false);
	}

	rootSignature_ = psoInfo.rootSignature;
	pipelineState_ = psoInfo.pipelineState;

	Logger::Log(Logger::GetStream(), "Complete create LineGlitch PSO (PSOFactory version)!!\n");
}


void LineGlitchPostEffect::CreateParameterBuffer() {
	// 構造体サイズをデバッグ出力
	size_t structSize = sizeof(LineGlitchParameters);
	Logger::Log(Logger::GetStream(), std::format("LineGlitchParameters size: {} bytes\n", structSize));

	// 256バイト境界に揃える（DirectX12の要件）
	size_t alignedSize = (structSize + 255) & ~255;
	Logger::Log(Logger::GetStream(), std::format("Aligned buffer size: {} bytes\n", alignedSize));

	// パラメータバッファ作成
	parameterBuffer_ = CreateBufferResource(dxCommon_->GetDeviceComPtr(), alignedSize);
	parameterBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedParameters_));

	// 初期データを設定
	UpdateParameterBuffer();

	Logger::Log(Logger::GetStream(), "Complete create LineGlitch parameter buffer (OffscreenTriangle version)!!\n");
}

Microsoft::WRL::ComPtr<IDxcBlob> LineGlitchPostEffect::CompileShader(
	const std::wstring& filePath, const wchar_t* profile) {

	// DirectXCommonのCompileShader関数を使用
	return DirectXCommon::CompileShader(
		filePath,
		profile,
		dxCommon_->GetDxcUtils(),
		dxCommon_->GetDxcCompiler(),
		dxCommon_->GetIncludeHandler());
}

void LineGlitchPostEffect::UpdateParameterBuffer() {
	if (mappedParameters_) {
		*mappedParameters_ = parameters_;
	}
}

void LineGlitchPostEffect::ApplyPreset(EffectPreset preset) {
	switch (preset) {
	case EffectPreset::OFF:
		SetEnabled(false);
		break;

	case EffectPreset::SUBTLE:
		parameters_.noiseIntensity = 0.5f;
		parameters_.noiseInterval = 0.9f;
		parameters_.animationSpeed = 1.0f;
		SetEnabled(true);
		break;

	case EffectPreset::MEDIUM:
		parameters_.noiseIntensity = 1.5f;
		parameters_.noiseInterval = 0.8f;
		parameters_.animationSpeed = 1.2f;
		SetEnabled(true);
		break;

	case EffectPreset::INTENSE:
		parameters_.noiseIntensity = 3.0f;
		parameters_.noiseInterval = 0.6f;
		parameters_.animationSpeed = 1.5f;
		SetEnabled(true);
		break;
	}

	UpdateParameterBuffer();
}

void LineGlitchPostEffect::SetNoiseIntensity(float intensity) {
	parameters_.noiseIntensity = std::clamp(intensity, 0.0f, 10.0f);
	UpdateParameterBuffer();
}

void LineGlitchPostEffect::SetNoiseInterval(float interval) {
	parameters_.noiseInterval = std::clamp(interval, 0.0f, 1.0f);
	UpdateParameterBuffer();
}

void LineGlitchPostEffect::ImGui() {
#ifdef _DEBUG
	if (ImGui::CollapsingHeader(name_.c_str())) {
		// エフェクトの状態表示
		ImGui::Text("Effect Status: %s", isEnabled_ ? "ENABLED" : "DISABLED");
		ImGui::Text("Initialized: %s", isInitialized_ ? "YES" : "NO");
		ImGui::Text("Render Method: OffscreenTriangle");

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
				float noiseIntensity = parameters_.noiseIntensity;
				if (ImGui::SliderFloat("Noise Intensity", &noiseIntensity, 0.0f, 3.0f)) {
					SetNoiseIntensity(noiseIntensity);
				}

				float noiseInterval = parameters_.noiseInterval;
				if (ImGui::SliderFloat("Noise Interval", &noiseInterval, 0.0f, 1.0f)) {
					SetNoiseInterval(noiseInterval);
				}

				if (ImGui::SliderFloat("Animation Speed", &animationSpeed_, 0.0f, 3.0f)) {
					parameters_.animationSpeed = animationSpeed_;
					UpdateParameterBuffer();
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