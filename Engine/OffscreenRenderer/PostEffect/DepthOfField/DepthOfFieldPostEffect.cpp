#include "DepthOfFieldPostEffect.h"
#include "Managers/ImGui/ImGuiManager.h" 

void DepthOfFieldPostEffect::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;

	// PSOを作成
	CreatePSO();

	// パラメータバッファを作成
	CreateParameterBuffer();

	isInitialized_ = true;

	Logger::Log(Logger::GetStream(), "DepthOfFieldPostEffect initialized successfully (OffscreenTriangle version)!\n");
}

void DepthOfFieldPostEffect::Finalize() {
	// パラメータデータのマッピング解除
	if (mappedParameters_) {
		parameterBuffer_->Unmap(0, nullptr);
		mappedParameters_ = nullptr;
	}

	isInitialized_ = false;
	Logger::Log(Logger::GetStream(), "DepthOfFieldPostEffect finalized (OffscreenTriangle version).\n");
}

void DepthOfFieldPostEffect::Update(float deltaTime) {
	if (!isEnabled_ || !isInitialized_) {
		return;
	}

	// 時間を更新
	parameters_.time += deltaTime * animationSpeed_;

	// パラメータバッファを更新
	UpdateParameterBuffer();
}

void DepthOfFieldPostEffect::Apply(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV, D3D12_CPU_DESCRIPTOR_HANDLE outputRTV, OffscreenTriangle* renderTriangle) {
	// 基底クラスのApplyは使用しない（深度テクスチャが必要なため）
	// 代わりに専用のApplyを使用することを推奨
	Logger::Log(Logger::GetStream(), "Warning: DepthOfFieldPostEffect requires depth texture. Use Apply with depthSRV parameter.\n");
}

void DepthOfFieldPostEffect::Apply(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV, D3D12_GPU_DESCRIPTOR_HANDLE depthSRV, D3D12_CPU_DESCRIPTOR_HANDLE outputRTV, OffscreenTriangle* renderTriangle) {
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

	renderTriangle->DrawWithCustomPSOAndDepth(
		rootSignature_.Get(),
		pipelineState_.Get(),
		inputSRV,		// カラーテクスチャ
		depthSRV,		// 深度テクスチャ
		parameterBuffer_->GetGPUVirtualAddress()	// パラメータバッファをマテリアルとして使用
	);
}
void DepthOfFieldPostEffect::CreatePSO() {
	// RootSignatureを構築
	RootSignatureBuilder rsBuilder;
	rsBuilder.AddCBV(0, D3D12_SHADER_VISIBILITY_PIXEL)		// DepthOfFieldParameters (b0)
		.AddSRV(0, 1, D3D12_SHADER_VISIBILITY_PIXEL)		// ColorTexture (t0)
		.AddSRV(1, 1, D3D12_SHADER_VISIBILITY_PIXEL)		// DepthTexture (t1)
		.AddStaticSampler(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

	// PSO設定を構築
	auto psoDesc = PSODescriptor::CreatePostEffectWithDepth()
		.SetPixelShader(L"resources/Shader/DepthOfField/DepthOfField.PS.hlsl");

	// PSOFactory経由で生成
	auto psoInfo = dxCommon_->GetPSOFactory()->CreatePSO(psoDesc, rsBuilder);
	if (!psoInfo.IsValid()) {
		Logger::Log(Logger::GetStream(), "DepthOfFieldPostEffect: Failed to create PSO\n");
		assert(false);
	}

	rootSignature_ = psoInfo.rootSignature;
	pipelineState_ = psoInfo.pipelineState;

	Logger::Log(Logger::GetStream(), "Complete create DepthOfField PSO (PSOFactory version)!!\n");
}


void DepthOfFieldPostEffect::CreateParameterBuffer() {
	// 構造体サイズをデバッグ出力
	size_t structSize = sizeof(DepthOfFieldParameters);
	Logger::Log(Logger::GetStream(), std::format("DepthOfFieldParameters size: {} bytes\n", structSize));

	// 256バイト境界に揃える（DirectX12の要件）
	size_t alignedSize = (structSize + 255) & ~255;
	Logger::Log(Logger::GetStream(), std::format("Aligned buffer size: {} bytes\n", alignedSize));

	// パラメータバッファ作成
	parameterBuffer_ = CreateBufferResource(dxCommon_->GetDeviceComPtr(), alignedSize);
	parameterBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedParameters_));

	// 初期データを設定
	UpdateParameterBuffer();

	Logger::Log(Logger::GetStream(), "Complete create DepthOfField parameter buffer (OffscreenTriangle version)!!\n");
}

Microsoft::WRL::ComPtr<IDxcBlob> DepthOfFieldPostEffect::CompileShader(
	const std::wstring& filePath, const wchar_t* profile) {

	// DirectXCommonのCompileShader関数を使用
	return DirectXCommon::CompileShader(
		filePath,
		profile,
		dxCommon_->GetDxcUtils(),
		dxCommon_->GetDxcCompiler(),
		dxCommon_->GetIncludeHandler());
}

void DepthOfFieldPostEffect::UpdateParameterBuffer() {
	if (mappedParameters_) {
		*mappedParameters_ = parameters_;
		// focusParamsも更新
		parameters_.focusParams.x = parameters_.focusDistance;
		parameters_.focusParams.y = parameters_.focusRange;
		parameters_.focusParams.z = parameters_.blurStrength;
	}
}

void DepthOfFieldPostEffect::ApplyPreset(EffectPreset preset) {
	switch (preset) {
	case EffectPreset::NONE:
		SetEnabled(false);
		break;
	}

	UpdateParameterBuffer();
}

void DepthOfFieldPostEffect::SetFocusDistance(float distance) {
	parameters_.focusDistance = (std::max)(0.1f, distance);
	UpdateParameterBuffer();
}

void DepthOfFieldPostEffect::SetFocusRange(float range) {
	parameters_.focusRange = (std::max)(0.1f, range);
	UpdateParameterBuffer();
}

void DepthOfFieldPostEffect::SetBlurStrength(float strength) {
	parameters_.blurStrength = std::clamp(strength, 0.0f, 3.0f);
	UpdateParameterBuffer();
}

void DepthOfFieldPostEffect::ImGui() {

	if (ImGui::CollapsingHeader(name_.c_str())) {
		// エフェクトの状態表示
		ImGui::Text("Effect Status: %s", isEnabled_ ? "ENABLED" : "DISABLED");
		ImGui::Text("Initialized: %s", isInitialized_ ? "YES" : "NO");

		if (isEnabled_) {
			// プリセット選択
			if (ImGui::TreeNode("Presets")) {
				if (ImGui::Button("None")) ApplyPreset(EffectPreset::NONE);
				ImGui::TreePop();
			}

			// 個別パラメータ調整
			if (ImGui::TreeNode("Manual Settings")) {
				float focusDistance = parameters_.focusDistance;
				if (ImGui::DragFloat("Focus Distance", &focusDistance, 0.1f, 0.1f, 100.0f)) {
					SetFocusDistance(focusDistance);
				}

				float focusRange = parameters_.focusRange;
				if (ImGui::DragFloat("Focus Range", &focusRange, 0.1f, 0.1f, 50.0f)) {
					SetFocusRange(focusRange);
				}

				float blurStrength = parameters_.blurStrength;
				if (ImGui::SliderFloat("Blur Strength", &blurStrength, 0.0f, 3.0f)) {
					SetBlurStrength(blurStrength);
				}

				if (ImGui::SliderFloat("Animation Speed", &animationSpeed_, 0.0f, 3.0f)) {
				}
			}

			// 情報表示
			ImGui::Separator();
			ImGui::Text("Current Time: %.2f", parameters_.time);
		}
	}

}