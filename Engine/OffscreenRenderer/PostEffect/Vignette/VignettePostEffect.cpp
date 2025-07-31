#include "VignettePostEffect.h"
#include "Managers/ImGui/ImGuiManager.h" 

void VignettePostEffect::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;

	// PSOを作成
	CreatePSO();

	// パラメータバッファを作成
	CreateParameterBuffer();

	isInitialized_ = true;

	Logger::Log(Logger::GetStream(), "VignettePostEffect initialized successfully (OffscreenTriangle version)!\n");
}

void VignettePostEffect::Finalize() {
	// パラメータデータのマッピング解除
	if (mappedParameters_) {
		parameterBuffer_->Unmap(0, nullptr);
		mappedParameters_ = nullptr;
	}

	isInitialized_ = false;
	Logger::Log(Logger::GetStream(), "VignettePostEffect finalized (OffscreenTriangle version).\n");
}

void VignettePostEffect::Update(float deltaTime) {
	if (!isEnabled_ || !isInitialized_) {
		return;
	}


	parameters_.time += deltaTime * animationSpeed_;


	// パラメータバッファを更新
	UpdateParameterBuffer();
}

void VignettePostEffect::Apply(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV, D3D12_CPU_DESCRIPTOR_HANDLE outputRTV, OffscreenTriangle* renderTriangle) {
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
void VignettePostEffect::CreatePSO() {
	// RootSignatureを構築
	RootSignatureBuilder rsBuilder;
	rsBuilder.AddCBV(0, D3D12_SHADER_VISIBILITY_PIXEL)				// VignetteParameters (b0)
		.AddSRV(0, 1, D3D12_SHADER_VISIBILITY_PIXEL)				// Texture (t0)
		.AddStaticSampler(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

	// PSO設定を構築
	auto psoDesc = PSODescriptor::CreatePostEffectColorOnly()
		.SetPixelShader(L"resources/Shader/Vignette/Vignette.PS.hlsl");

	// PSOFactory経由で生成
	auto psoInfo = dxCommon_->GetPSOFactory()->CreatePSO(psoDesc, rsBuilder);
	if (!psoInfo.IsValid()) {
		Logger::Log(Logger::GetStream(), "VignettePostEffect: Failed to create PSO\n");
		assert(false);
	}

	rootSignature_ = psoInfo.rootSignature;
	pipelineState_ = psoInfo.pipelineState;

	Logger::Log(Logger::GetStream(), "Complete create Vignette PSO (PSOFactory version)!!\n");
}

void VignettePostEffect::CreateParameterBuffer() {
	// 構造体サイズをデバッグ出力
	size_t structSize = sizeof(VignetteParameters);
	Logger::Log(Logger::GetStream(), std::format("VignetteParameters size: {} bytes\n", structSize));

	// 256バイト境界に揃える（DirectX12の要件）
	size_t alignedSize = (structSize + 255) & ~255;
	Logger::Log(Logger::GetStream(), std::format("Aligned buffer size: {} bytes\n", alignedSize));

	// パラメータバッファ作成
	parameterBuffer_ = CreateBufferResource(dxCommon_->GetDeviceComPtr(), alignedSize);
	parameterBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedParameters_));

	// 初期データを設定
	UpdateParameterBuffer();

	Logger::Log(Logger::GetStream(), "Complete create Vignette parameter buffer (OffscreenTriangle version)!!\n");
}

Microsoft::WRL::ComPtr<IDxcBlob> VignettePostEffect::CompileShader(
	const std::wstring& filePath, const wchar_t* profile) {

	// DirectXCommonのCompileShader関数を使用
	return DirectXCommon::CompileShader(
		filePath,
		profile,
		dxCommon_->GetDxcUtils(),
		dxCommon_->GetDxcCompiler(),
		dxCommon_->GetIncludeHandler());
}

void VignettePostEffect::UpdateParameterBuffer() {
	if (mappedParameters_) {
		*mappedParameters_ = parameters_;
	}
}

void VignettePostEffect::ApplyPreset(EffectPreset preset) {
	switch (preset) {
	case EffectPreset::OFF:
		SetEnabled(false);
		break;

	case EffectPreset::SUBTLE:
		parameters_.vignetteStrength = 0.3f;
		parameters_.vignetteRadius = 0.7f;
		parameters_.vignetteSoftness = 0.4f;
		SetEnabled(true);
		break;

	case EffectPreset::MEDIUM:
		parameters_.vignetteStrength = 0.6f;
		parameters_.vignetteRadius = 0.5f;
		parameters_.vignetteSoftness = 0.3f;
		SetEnabled(true);
		break;

	case EffectPreset::INTENSE:
		parameters_.vignetteStrength = 0.9f;
		parameters_.vignetteRadius = 0.3f;
		parameters_.vignetteSoftness = 0.2f;
		SetEnabled(true);
		break;

	case EffectPreset::CINEMATIC:
		parameters_.vignetteStrength = 0.7f;
		parameters_.vignetteRadius = 0.4f;
		parameters_.vignetteSoftness = 0.5f;
		SetEnabled(true);
		break;
	}

	UpdateParameterBuffer();
}

void VignettePostEffect::SetVignetteStrength(float strength) {
	parameters_.vignetteStrength = std::clamp(strength, 0.0f, 1.0f);
	UpdateParameterBuffer();
}

void VignettePostEffect::SetVignetteRadius(float radius) {
	parameters_.vignetteRadius = std::clamp(radius, 0.0f, 1.0f);
	UpdateParameterBuffer();
}

void VignettePostEffect::SetVignetteSoftness(float softness) {
	parameters_.vignetteSoftness = std::clamp(softness, 0.0f, 1.0f);
	UpdateParameterBuffer();
}

void VignettePostEffect::SetVignetteColor(const Vector4& color) {
	parameters_.vignetteColor = color;
	UpdateParameterBuffer();
}

void VignettePostEffect::ImGui() {
#ifdef _DEBUG
	if (ImGui::TreeNode(name_.c_str())) {
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
				ImGui::SameLine();
				if (ImGui::Button("Cinematic")) ApplyPreset(EffectPreset::CINEMATIC);

				ImGui::TreePop();
			}

			// 個別パラメータ調整
			if (ImGui::TreeNode("Manual Settings")) {
				float vignetteStrength = parameters_.vignetteStrength;
				if (ImGui::SliderFloat("Vignette Strength", &vignetteStrength, 0.0f, 1.0f)) {
					SetVignetteStrength(vignetteStrength);
				}

				float vignetteRadius = parameters_.vignetteRadius;
				if (ImGui::SliderFloat("Vignette Radius", &vignetteRadius, 0.0f, 1.0f)) {
					SetVignetteRadius(vignetteRadius);
				}

				float vignetteSoftness = parameters_.vignetteSoftness;
				if (ImGui::SliderFloat("Vignette Softness", &vignetteSoftness, 0.0f, 1.0f)) {
					SetVignetteSoftness(vignetteSoftness);
				}

				// 色選択
				float vignetteColor[4] = {
					parameters_.vignetteColor.x,
					parameters_.vignetteColor.y,
					parameters_.vignetteColor.z,
					parameters_.vignetteColor.w
				};
				if (ImGui::ColorEdit4("Vignette Color", vignetteColor)) {
					SetVignetteColor({ vignetteColor[0], vignetteColor[1], vignetteColor[2], vignetteColor[3] });
				}

				ImGui::TreePop();
			}

			// 情報表示
			ImGui::Separator();
			ImGui::Text("Current Time: %.2f", parameters_.time);
			ImGui::Text("Vignette Strength: %.2f", parameters_.vignetteStrength);
			ImGui::Text("Vignette Radius: %.2f", parameters_.vignetteRadius);
			ImGui::Text("Vignette Softness: %.2f", parameters_.vignetteSoftness);
			ImGui::Text("Animation Speed: %.2f", animationSpeed_);
		}

		ImGui::TreePop();
	}
#endif
}