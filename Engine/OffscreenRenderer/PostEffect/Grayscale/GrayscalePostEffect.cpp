#include "GrayscalePostEffect.h"

void GrayscalePostEffect::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;

	// PSOを作成
	CreatePSO();

	// パラメータバッファを作成
	CreateParameterBuffer();

	isInitialized_ = true;

	Logger::Log(Logger::GetStream(), "GrayscalePostEffect initialized successfully (Sprite version)!\n");
}

void GrayscalePostEffect::Finalize() {
	// パラメータデータのマッピング解除
	if (mappedParameters_) {
		parameterBuffer_->Unmap(0, nullptr);
		mappedParameters_ = nullptr;
	}

	isInitialized_ = false;
	Logger::Log(Logger::GetStream(), "GrayscalePostEffect finalized (Sprite version).\n");
}

void GrayscalePostEffect::Update(float deltaTime) {
	if (!isEnabled_ || !isInitialized_) {
		return;
	}

	// 時間を更新
	parameters_.time += deltaTime * animationSpeed_;

	// パラメータバッファを更新
	UpdateParameterBuffer();
}

void GrayscalePostEffect::Apply(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV, D3D12_CPU_DESCRIPTOR_HANDLE outputRTV, Sprite* renderSprite) {
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

	// Spriteを使用してカスタムPSOで描画
	renderSprite->DrawWithCustomPSO(
		rootSignature_.Get(),
		pipelineState_.Get(),
		inputSRV,
		parameterBuffer_->GetGPUVirtualAddress()
	);
}

void GrayscalePostEffect::CreatePSO() {
	// グレースケールエフェクト用のルートシグネチャ作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// DescriptorRange（テクスチャ用）
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// RootParameter（3つ：Parameters、Transform、Texture）
	D3D12_ROOT_PARAMETER rootParameters[3] = {};

	// GrayscaleParameters (b0)
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].Descriptor.ShaderRegister = 0;

	// Transform (b0) - 頂点シェーダー用
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].Descriptor.ShaderRegister = 0;

	// Texture (t0)
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);

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

	// InputLayout設定（normalを削除）
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

	// グレースケール用シェーダーをコンパイル
	vertexShaderBlob_ = CompileShader(L"resources/Shader/Grayscale/Grayscale.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob_ != nullptr);

	pixelShaderBlob_ = CompileShader(L"resources/Shader/Grayscale/Grayscale.PS.hlsl", L"ps_6_0");
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

	Logger::Log(Logger::GetStream(), "Complete create Grayscale PSO!!\n");
}

void GrayscalePostEffect::CreateParameterBuffer() {
	// 構造体サイズをデバッグ出力
	size_t structSize = sizeof(GrayscaleParameters);
	Logger::Log(Logger::GetStream(), std::format("GrayscaleParameters size: {} bytes\n", structSize));

	// 256バイト境界に揃える（DirectX12の要件）
	size_t alignedSize = (structSize + 255) & ~255;
	Logger::Log(Logger::GetStream(), std::format("Aligned buffer size: {} bytes\n", alignedSize));

	// パラメータバッファ作成
	parameterBuffer_ = CreateBufferResource(dxCommon_->GetDeviceComPtr(), alignedSize);
	parameterBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedParameters_));

	// 初期データを設定
	UpdateParameterBuffer();

	Logger::Log(Logger::GetStream(), "Complete create Grayscale parameter buffer (Sprite version)!!\n");
}

Microsoft::WRL::ComPtr<IDxcBlob> GrayscalePostEffect::CompileShader(
	const std::wstring& filePath, const wchar_t* profile) {

	// DirectXCommonのCompileShader関数を使用
	return DirectXCommon::CompileShader(
		filePath,
		profile,
		dxCommon_->GetDxcUtils(),
		dxCommon_->GetDxcCompiler(),
		dxCommon_->GetIncludeHandler());
}

void GrayscalePostEffect::UpdateParameterBuffer() {
	if (mappedParameters_) {
		*mappedParameters_ = parameters_;
	}
}

void GrayscalePostEffect::ApplyPreset(EffectPreset preset) {
	switch (preset) {
	case EffectPreset::OFF:
		SetEnabled(false);
		break;

	case EffectPreset::SUBTLE:
		parameters_.grayIntensity = 0.3f;
		SetEnabled(true);
		break;

	case EffectPreset::MEDIUM:
		parameters_.grayIntensity = 0.7f;
		SetEnabled(true);
		break;

	case EffectPreset::INTENSE:
		parameters_.grayIntensity = 1.0f;
		SetEnabled(true);
		break;
	}

	UpdateParameterBuffer();
}

void GrayscalePostEffect::SetGrayIntensity(float intensity) {
	parameters_.grayIntensity = std::clamp(intensity, 0.0f, 1.0f);
	UpdateParameterBuffer();
}

void GrayscalePostEffect::ImGui() {
#ifdef _DEBUG
	if (ImGui::TreeNode(name_.c_str())) {
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
				float grayIntensity = parameters_.grayIntensity;
				if (ImGui::SliderFloat("Gray Intensity", &grayIntensity, 0.0f, 1.0f)) {
					SetGrayIntensity(grayIntensity);
				}

				if (ImGui::SliderFloat("Animation Speed", &animationSpeed_, 0.0f, 3.0f)) {
				}

				ImGui::TreePop();
			}

			// 情報表示
			ImGui::Separator();
			ImGui::Text("Current Time: %.2f", parameters_.time);
			ImGui::Text("Gray Intensity: %.2f", parameters_.grayIntensity);
			ImGui::Text("Animation Speed: %.2f", animationSpeed_);
		}

		ImGui::TreePop();
	}

#endif
}