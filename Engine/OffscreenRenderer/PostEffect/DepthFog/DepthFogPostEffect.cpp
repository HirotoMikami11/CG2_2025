#include "DepthFogPostEffect.h"
#include "Managers/ImGui/ImGuiManager.h" 

void DepthFogPostEffect::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;

	// PSOを作成
	CreatePSO();

	// パラメータバッファを作成
	CreateParameterBuffer();

	isInitialized_ = true;

	Logger::Log(Logger::GetStream(), "DepthFogPostEffect initialized successfully (OffscreenTriangle version)!\n");
}

void DepthFogPostEffect::Finalize() {
	// パラメータデータのマッピング解除
	if (mappedParameters_) {
		parameterBuffer_->Unmap(0, nullptr);
		mappedParameters_ = nullptr;
	}

	isInitialized_ = false;
	Logger::Log(Logger::GetStream(), "DepthFogPostEffect finalized (OffscreenTriangle version).\n");
}

void DepthFogPostEffect::Update(float deltaTime) {
	if (!isEnabled_ || !isInitialized_) {
		return;
	}

	// 時間を更新
	parameters_.time += deltaTime * animationSpeed_;

	// パラメータバッファを更新
	UpdateParameterBuffer();
}

void DepthFogPostEffect::Apply(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV, D3D12_CPU_DESCRIPTOR_HANDLE outputRTV, OffscreenTriangle* renderTriangle) {
	// 基底クラスのApplyは使用しない（深度テクスチャが必要なため）
	// 代わりに専用のApplyを使用することを推奨
	Logger::Log(Logger::GetStream(), "Warning: DepthFogPostEffect requires depth texture. Use Apply with depthSRV parameter.\n");
}

void DepthFogPostEffect::Apply(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV, D3D12_GPU_DESCRIPTOR_HANDLE depthSRV, D3D12_CPU_DESCRIPTOR_HANDLE outputRTV, OffscreenTriangle* renderTriangle) {
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

	// OffscreenTriangleの新しいDrawWithCustomPSOAndDepth関数を使用して描画
	renderTriangle->DrawWithCustomPSOAndDepth(
		rootSignature_.Get(),
		pipelineState_.Get(),
		inputSRV,		// カラーテクスチャ
		depthSRV,		// 深度テクスチャ
		parameterBuffer_->GetGPUVirtualAddress()	// パラメータバッファをマテリアルとして使用
	);
}

void DepthFogPostEffect::CreatePSO() {
	// RootSignatureを構築
	RootSignatureBuilder rsBuilder;
	rsBuilder.AddCBV(0, D3D12_SHADER_VISIBILITY_PIXEL)	// DepthFogParameters (b0)
		.AddSRV(0, 1, D3D12_SHADER_VISIBILITY_PIXEL)	// ColorTexture (t0)
		.AddSRV(1, 1, D3D12_SHADER_VISIBILITY_PIXEL)	// DepthTexture (t1)
		.AddStaticSampler(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

	// PSO設定を構築
	auto psoDesc = PSODescriptor::CreatePostEffectWithDepth()
		.SetPixelShader(L"resources/Shader/DepthFog/DepthFog.PS.hlsl");

	// PSOFactory経由で生成
	auto psoInfo = dxCommon_->GetPSOFactory()->CreatePSO(psoDesc, rsBuilder);
	if (!psoInfo.IsValid()) {
		Logger::Log(Logger::GetStream(), "DepthFogPostEffect: Failed to create PSO\n");
		assert(false);
	}

	rootSignature_ = psoInfo.rootSignature;
	pipelineState_ = psoInfo.pipelineState;

	Logger::Log(Logger::GetStream(), "Complete create DepthFog PSO (PSOFactory version)!!\n");
}


void DepthFogPostEffect::CreateParameterBuffer() {
	// 構造体サイズをデバッグ出力
	size_t structSize = sizeof(DepthFogParameters);
	Logger::Log(Logger::GetStream(), std::format("DepthFogParameters size: {} bytes\n", structSize));

	// 256バイト境界に揃える（DirectX12の要件）
	size_t alignedSize = (structSize + 255) & ~255;
	Logger::Log(Logger::GetStream(), std::format("Aligned buffer size: {} bytes\n", alignedSize));

	// パラメータバッファ作成
	parameterBuffer_ = CreateBufferResource(dxCommon_->GetDeviceComPtr(), alignedSize);
	parameterBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedParameters_));

	// 初期データを設定
	UpdateParameterBuffer();

	Logger::Log(Logger::GetStream(), "Complete create DepthFog parameter buffer (OffscreenTriangle version)!!\n");
}

Microsoft::WRL::ComPtr<IDxcBlob> DepthFogPostEffect::CompileShader(
	const std::wstring& filePath, const wchar_t* profile) {

	// DirectXCommonのCompileShader関数を使用
	return DirectXCommon::CompileShader(
		filePath,
		profile,
		dxCommon_->GetDxcUtils(),
		dxCommon_->GetDxcCompiler(),
		dxCommon_->GetIncludeHandler());
}

void DepthFogPostEffect::UpdateParameterBuffer() {
	if (mappedParameters_) {
		*mappedParameters_ = parameters_;
	}
}

void DepthFogPostEffect::ApplyPreset(EffectPreset preset) {
	switch (preset) {
	case EffectPreset::NONE:
		SetEnabled(false);
		break;

	case EffectPreset::LIGHT:
		parameters_.fogColor = { 0.7f, 0.7f, 0.7f, 1.0f };
		parameters_.fogNear = 5.0f;		// 非線形変換対応：5.0f距離からフォグ開始
		parameters_.fogFar = 30.0f;		// 非線形変換対応：30.0f距離で完全にフォグ
		parameters_.fogDensity = 0.5f;
		SetEnabled(true);
		break;

	case EffectPreset::HEAVY:
		parameters_.fogColor = { 0.02f, 0.08f, 0.25f, 1.00f };
		parameters_.fogNear = 0.2f;
		parameters_.fogFar = 40.0f;
		parameters_.fogDensity = 5.0f;
		parameters_.time = 0.0f;
		SetEnabled(true);
		break;

	case EffectPreset::UNDERWATER:
		parameters_.fogColor = { 0.02f, 0.08f, 0.25f, 1.00f };
		parameters_.fogNear = 0.2f;
		parameters_.fogFar = 40.0f;
		parameters_.fogDensity = 1.0f;
		parameters_.time = 0.0f;
		SetEnabled(true);
		break;
	}

	UpdateParameterBuffer();
}

void DepthFogPostEffect::SetFogColor(const Vector4& color) {
	parameters_.fogColor = color;
	UpdateParameterBuffer();
}

void DepthFogPostEffect::SetFogDistance(float fogNear, float fogFar) {
	parameters_.fogNear = (std::max)(0.1f, fogNear);
	parameters_.fogFar = (std::max)(parameters_.fogNear + 0.1f, fogFar);
	UpdateParameterBuffer();
}

void DepthFogPostEffect::SetFogDensity(float density) {
	parameters_.fogDensity = std::clamp(density, 0.0f, 1.0f);
	UpdateParameterBuffer();
}

void DepthFogPostEffect::ImGui() {
#ifdef _DEBUG
	if (ImGui::TreeNode(name_.c_str())) {
		// エフェクトの状態表示
		ImGui::Text("Effect Status: %s", isEnabled_ ? "ENABLED" : "DISABLED");
		ImGui::Text("Initialized: %s", isInitialized_ ? "YES" : "NO");
		ImGui::Text("Render Method: OffscreenTriangle");

		if (isEnabled_) {
			// プリセット選択
			if (ImGui::TreeNode("Presets")) {
				if (ImGui::Button("None")) ApplyPreset(EffectPreset::NONE);
				ImGui::SameLine();
				if (ImGui::Button("Light")) ApplyPreset(EffectPreset::LIGHT);
				ImGui::SameLine();
				if (ImGui::Button("Heavy")) ApplyPreset(EffectPreset::HEAVY);
				ImGui::SameLine();
				if (ImGui::Button("Underwater")) ApplyPreset(EffectPreset::UNDERWATER);

				ImGui::TreePop();
			}

			// 個別パラメータ調整
			if (ImGui::TreeNode("Manual Settings")) {
				Vector4 fogColor = parameters_.fogColor;
				if (ImGui::ColorEdit4("Fog Color", reinterpret_cast<float*>(&fogColor.x))) {
					SetFogColor(fogColor);
				}

				float fogNear = parameters_.fogNear;
				float fogFar = parameters_.fogFar;
				if (ImGui::DragFloat("Fog Near", &fogNear, 0.1f, 0.1f, 1000.0f)) {
					SetFogDistance(fogNear, parameters_.fogFar);
				}
				if (ImGui::DragFloat("Fog Far", &fogFar, 0.1f, 0.1f, 1000.0f)) {
					SetFogDistance(parameters_.fogNear, fogFar);
				}

				float fogDensity = parameters_.fogDensity;
				if (ImGui::SliderFloat("Fog Density", &fogDensity, 0.0f, 1.0f)) {
					SetFogDensity(fogDensity);
				}

				if (ImGui::SliderFloat("Animation Speed", &animationSpeed_, 0.0f, 3.0f)) {
				}

				ImGui::TreePop();
			}

			// 情報表示
			ImGui::Separator();
			ImGui::Text("Current Time: %.2f", parameters_.time);
			ImGui::Text("Fog Near: %.2f", parameters_.fogNear);
			ImGui::Text("Fog Far: %.2f", parameters_.fogFar);
			ImGui::Text("Fog Density: %.2f", parameters_.fogDensity);
			ImGui::Text("Animation Speed: %.2f", animationSpeed_);
		}

		ImGui::TreePop();
	}
#endif
}