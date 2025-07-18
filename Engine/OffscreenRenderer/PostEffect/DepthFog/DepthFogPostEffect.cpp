#include "DepthFogPostEffect.h"
#include "Managers/ImGui/ImGuiManager.h" 

void DepthFogPostEffect::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;

	// PSOを作成
	CreatePSO();

	// パラメータバッファを作成
	CreateParameterBuffer();

	isInitialized_ = true;

	Logger::Log(Logger::GetStream(), "DepthFogPostEffect initialized successfully (Sprite version)!\n");
}

void DepthFogPostEffect::Finalize() {
	// パラメータデータのマッピング解除
	if (mappedParameters_) {
		parameterBuffer_->Unmap(0, nullptr);
		mappedParameters_ = nullptr;
	}

	isInitialized_ = false;
	Logger::Log(Logger::GetStream(), "DepthFogPostEffect finalized (Sprite version).\n");
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

void DepthFogPostEffect::Apply(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV, D3D12_CPU_DESCRIPTOR_HANDLE outputRTV, Sprite* renderSprite) {
	// 基底クラスのApplyは使用しない（深度テクスチャが必要なため）
	// 代わりに専用のApplyを使用することを推奨
	Logger::Log(Logger::GetStream(), "Warning: DepthFogPostEffect requires depth texture. Use Apply with depthSRV parameter.\n");
}

void DepthFogPostEffect::Apply(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV, D3D12_GPU_DESCRIPTOR_HANDLE depthSRV, D3D12_CPU_DESCRIPTOR_HANDLE outputRTV, Sprite* renderSprite) {
	if (!isEnabled_ || !isInitialized_) {
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

	// Spriteの新しいDrawWithCustomPSOAndDepth関数を使用して描画
	renderSprite->DrawWithCustomPSOAndDepth(
		rootSignature_.Get(),
		pipelineState_.Get(),
		inputSRV,		// カラーテクスチャ
		depthSRV,		// 深度テクスチャ
		parameterBuffer_->GetGPUVirtualAddress()	// パラメータバッファをマテリアルとして使用
	);
}

void DepthFogPostEffect::CreatePSO() {
	// 深度フォグエフェクト用のルートシグネチャ作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// DescriptorRange（カラーテクスチャ用とデプステクスチャ用）
	D3D12_DESCRIPTOR_RANGE descriptorRanges[2] = {};

	// カラーテクスチャ用（t0）
	descriptorRanges[0].BaseShaderRegister = 0;
	descriptorRanges[0].NumDescriptors = 1;
	descriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// 深度テクスチャ用（t1）
	descriptorRanges[1].BaseShaderRegister = 1;
	descriptorRanges[1].NumDescriptors = 1;
	descriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// RootParameter（4つ：DepthFogParameters、Transform、ColorTexture、DepthTexture）
	D3D12_ROOT_PARAMETER rootParameters[4] = {};

	// DepthFogParameters (b0) - PixelShader用
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].Descriptor.ShaderRegister = 0;

	// Transform (b0) - VertexShader用
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].Descriptor.ShaderRegister = 0;

	// ColorTexture (t0)
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].DescriptorTable.pDescriptorRanges = &descriptorRanges[0];
	rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;

	// DepthTexture (t1)
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[3].DescriptorTable.pDescriptorRanges = &descriptorRanges[1];
	rootParameters[3].DescriptorTable.NumDescriptorRanges = 1;

	// Sampler
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	// シリアライズ
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob, errorBlob;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Logger::Log(Logger::GetStream(), reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));

	// InputLayout設定（normalは削除）
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// BlendState設定
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// RasterizerState設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// 深度フォグ用シェーダーをコンパイル
	vertexShaderBlob_ = CompileShader(L"resources/Shader/DepthFog/DepthFog.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob_ != nullptr);

	pixelShaderBlob_ = CompileShader(L"resources/Shader/DepthFog/DepthFog.PS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob_ != nullptr);

	// DepthStencilState設定（深度テストなし）
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = false;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	// PSO作成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPipelineStateDesc.VS = { vertexShaderBlob_->GetBufferPointer(), vertexShaderBlob_->GetBufferSize() };
	graphicsPipelineStateDesc.PS = { pixelShaderBlob_->GetBufferPointer(), pixelShaderBlob_->GetBufferSize() };
	graphicsPipelineStateDesc.BlendState = blendDesc;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// PSO生成
	hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&pipelineState_));
	assert(SUCCEEDED(hr));

	Logger::Log(Logger::GetStream(), "Complete create DepthFog PSO!!\n");
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

	Logger::Log(Logger::GetStream(), "Complete create DepthFog parameter buffer (Sprite version)!!\n");
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
		parameters_.fogColor = { 0.4f, 0.4f, 0.4f, 1.0f };
		parameters_.fogNear = 1.0f;		// 非線形変換対応：1.0f距離からフォグ開始
		parameters_.fogFar = 15.0f;		// 非線形変換対応：15.0f距離で完全にフォグ
		parameters_.fogDensity = 1.0f;
		SetEnabled(true);
		break;

	case EffectPreset::UNDERWATER:
		parameters_.fogColor = { 0.2f, 0.4f, 0.8f, 1.0f }; // 青い色
		parameters_.fogNear = 0.5f;		// 非線形変換対応：0.5f距離からフォグ開始
		parameters_.fogFar = 8.0f;		// 非線形変換対応：8.0f距離で完全にフォグ
		parameters_.fogDensity = 0.8f;
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
				if (ImGui::DragFloat("Fog Near", &fogNear, 0.1f, 0.1f, 200.0f)) {
					SetFogDistance(fogNear, parameters_.fogFar);
				}
				if (ImGui::DragFloat("Fog Far", &fogFar, 0.1f, 0.1f, 200.0f)) {
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