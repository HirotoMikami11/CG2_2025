#include "NoiseEffect.h"

void NoiseEffect::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;
	randomEngine_.seed(randomDevice_());

	// PSOを作成
	CreatePSO();

	// 定数バッファを作成
	CreateConstantBuffer();

	isInitialized_ = true;
#ifdef _DEBUG
	isEnabled_ = true;//デバッグのために有効
	//簡単な設定
	SetNoiseIntensity(0.1f);
	SetNoiseInterval(0.0f);
	SetcolorNoiseInternsity(0.4f);
	materialData_.blockIntensity = 0.2f;											//ブロックずらしの強さ
	materialData_.blockProbability = 0.05f;											//ブロックずらしの確率
	materialData_.reverseProbability = 0.01f;										//ブロック反転の確率
	materialData_.scanLineProbability = 0.03f;										//走査線の確率
	materialData_.animationSpeed = 0.15;

#endif // _DEBUG
	Logger::Log(Logger::GetStream(), "NoiseEffect initialized successfully!\n");
}

void NoiseEffect::Finalize() {
	// マテリアルデータのマッピング解除
	if (mappedMaterialData_) {
		materialBuffer_->Unmap(0, nullptr);
		mappedMaterialData_ = nullptr;
	}

	isInitialized_ = false;
	Logger::Log(Logger::GetStream(), "NoiseEffect finalized.\n");
}

void NoiseEffect::Update(float deltaTime) {
	if (!isEnabled_ || !isInitialized_) {
		return;
	}

	// 時間を更新
	materialData_.time += deltaTime * animationSpeed_;

	// 定数バッファを更新
	UpdateConstantBuffer();
}

void NoiseEffect::CreatePSO() {
	// ノイズエフェクト用のルートシグネチャ作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// DescriptorRange
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// RootParameter（3つ：Material、Transform、Texture）
	D3D12_ROOT_PARAMETER rootParameters[3] = {};

	// Material (b0) - ノイズマテリアル
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].Descriptor.ShaderRegister = 0;

	// Transform (b0)
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

	// InputLayout設定
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

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

	// ノイズ用シェーダーをコンパイル
	vertexShaderBlob_ = CompileShader(L"Noise.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob_ != nullptr);

	pixelShaderBlob_ = CompileShader(L"Noise.PS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob_ != nullptr);

	// DepthStencilState設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = false; // エフェクト描画では深度テストを無効
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

	Logger::Log(Logger::GetStream(), "Complete create noise PSO!!\n");
}
void NoiseEffect::CreateConstantBuffer() {
	// 構造体サイズをデバッグ出力
	size_t structSize = sizeof(NoiseMaterialData);
	Logger::Log(Logger::GetStream(), std::format("NoiseMaterialData size: {} bytes\n", structSize));

	// 各メンバーのオフセットを確認
	Logger::Log(Logger::GetStream(), std::format("color offset: {} bytes\n", offsetof(NoiseMaterialData, color)));
	Logger::Log(Logger::GetStream(), std::format("enableLighting offset: {} bytes\n", offsetof(NoiseMaterialData, enableLighting)));
	Logger::Log(Logger::GetStream(), std::format("useLambertianReflectance offset: {} bytes\n", offsetof(NoiseMaterialData, useLambertianReflectance)));
	Logger::Log(Logger::GetStream(), std::format("uvTransform offset: {} bytes\n", offsetof(NoiseMaterialData, uvTransform)));
	Logger::Log(Logger::GetStream(), std::format("time offset: {} bytes\n", offsetof(NoiseMaterialData, time)));
	Logger::Log(Logger::GetStream(), std::format("noiseIntensity offset: {} bytes\n", offsetof(NoiseMaterialData, noiseIntensity)));
	Logger::Log(Logger::GetStream(), std::format("noiseInterval offset: {} bytes\n", offsetof(NoiseMaterialData, noiseInterval)));
	Logger::Log(Logger::GetStream(), std::format("animationSpeed offset: {} bytes\n", offsetof(NoiseMaterialData, animationSpeed)));
	Logger::Log(Logger::GetStream(), std::format("blockIntensity offset: {} bytes\n", offsetof(NoiseMaterialData, blockIntensity)));
	Logger::Log(Logger::GetStream(), std::format("blockProbability offset: {} bytes\n", offsetof(NoiseMaterialData, blockProbability)));
	Logger::Log(Logger::GetStream(), std::format("reverseProbability offset: {} bytes\n", offsetof(NoiseMaterialData, reverseProbability)));
	Logger::Log(Logger::GetStream(), std::format("scanLineProbability offset: {} bytes\n", offsetof(NoiseMaterialData, scanLineProbability)));
	Logger::Log(Logger::GetStream(), std::format("blackIntensity offset: {} bytes\n", offsetof(NoiseMaterialData, blackIntensity)));
	Logger::Log(Logger::GetStream(), std::format("colorNoiseInternsity offset: {} bytes\n", offsetof(NoiseMaterialData, colorNoiseInternsity)));

	// 256バイト境界に揃える（DirectX12の要件）
	size_t alignedSize = (structSize + 255) & ~255;
	Logger::Log(Logger::GetStream(), std::format("Aligned buffer size: {} bytes\n", alignedSize));

	// マテリアルバッファ作成
	materialBuffer_ = CreateBufferResource(dxCommon_->GetDeviceComPtr(), alignedSize);
	materialBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedMaterialData_));

	// 安全な初期データを設定
	materialData_.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData_.enableLighting = 0;
	materialData_.useLambertianReflectance = 0;
	materialData_.uvTransform = MakeIdentity4x4();

	materialData_.time = 0.0f;
	materialData_.noiseIntensity = 0.2f;  // 初期状態では無効
	materialData_.noiseInterval = 0.0f;
	materialData_.animationSpeed = 1.0f;
	materialData_.blockIntensity = 0.2f;				//ブロックずらしの強さ
	materialData_.blockProbability = 0.05f;				//ブロックずらしの確率
	materialData_.reverseProbability = 0.01f;				//ブロック反転の確率
	materialData_.scanLineProbability = 0.03f;				//走査線の確率
	materialData_.blackIntensity = 0.8f;
	materialData_.colorNoiseInternsity = 0.1f;   // 非常に低い密度から開始
	materialData_.blockDivision = { 40.0f,20.0f };

	// 初期データを設定
	UpdateConstantBuffer();

	Logger::Log(Logger::GetStream(), "Complete create noise constant buffer!!\n");
}

void NoiseEffect::ApplyPreset(NoisePreset preset) {
	switch (preset) {
	case NoisePreset::OFF:
		SetEnabled(false);
		break;

	case NoisePreset::SUBTLE:
		SetEnabled(true);
		materialData_.time = 1000.0f;
		///全体に影響を与えるパラメータ
		materialData_.noiseIntensity = 0.28f;
		materialData_.noiseInterval = 0.735f;
		materialData_.animationSpeed = 0.33f;
		///色収差
		materialData_.colorNoiseInternsity = 0.5f;
		///グレースケール
		materialData_.blackIntensity = 0.5f;
		///ブロックずらし
		materialData_.blockIntensity = 0.2f;
		materialData_.blockProbability = 0.065f;
		materialData_.reverseProbability = 0.032f;
		materialData_.blockDivision = { 7.5f,25.0f };
		///走査線
		materialData_.scanLineProbability = 0.025f;
		break;

	case NoisePreset::MEDIUM:
		SetEnabled(true);
		materialData_.time = 0.0f;
		///全体に影響を与えるパラメータ
		materialData_.noiseIntensity = 0.28f;
		materialData_.noiseInterval = 0.735f;
		materialData_.animationSpeed = 1.0f;
		///色収差
		materialData_.colorNoiseInternsity = 0.5f;
		///グレースケール
		materialData_.blackIntensity = 0.5f;
		///ブロックずらし
		materialData_.blockIntensity = 0.2f;
		materialData_.blockProbability = 0.065f;
		materialData_.reverseProbability = 0.032f;
		materialData_.blockDivision = { 7.5f,25.0f };
		///走査線
		materialData_.scanLineProbability = 0.025f;
		break;

	case NoisePreset::HEAVY:
		SetEnabled(true);
		materialData_.time = 0.0f;
		///全体に影響を与えるパラメータ
		materialData_.noiseIntensity = 0.28f;
		materialData_.noiseInterval = 0.735f;
		materialData_.animationSpeed = 1.0f;
		///色収差
		materialData_.colorNoiseInternsity = 0.5f;
		///グレースケール
		materialData_.blackIntensity = 0.5f;
		///ブロックずらし
		materialData_.blockIntensity = 0.2f;
		materialData_.blockProbability = 0.065f;
		materialData_.reverseProbability = 0.032f;
		materialData_.blockDivision = { 7.5f,25.0f };
		///走査線
		materialData_.scanLineProbability = 0.025f;
		break;

	case NoisePreset::DIGITAL_RAIN:
		SetEnabled(true);
		materialData_.time = 0.0f;
		///全体に影響を与えるパラメータ
		materialData_.noiseIntensity = 0.28f;
		materialData_.noiseInterval = 0.735f;
		materialData_.animationSpeed = 1.0f;
		///色収差
		materialData_.colorNoiseInternsity = 0.5f;
		///グレースケール
		materialData_.blackIntensity = 0.5f;
		///ブロックずらし
		materialData_.blockIntensity = 0.2f;
		materialData_.blockProbability = 0.065f;
		materialData_.reverseProbability = 0.032f;
		materialData_.blockDivision = { 7.5f,25.0f };
		///走査線
		materialData_.scanLineProbability = 0.025f;
		break;
	}
}

void NoiseEffect::ImGui() {
	if (ImGui::CollapsingHeader("Noise Effect")) {
		// エフェクトの状態表示
		ImGui::Text("Effect Status: %s", isEnabled_ ? "ENABLED" : "DISABLED");
		ImGui::Text("Initialized: %s", isInitialized_ ? "YES" : "NO");

		// エフェクトのオン/オフ
		ImGui::Checkbox("Enable Noise Effect", &isEnabled_);


		// プリセットボタン
		ImGui::Text("Presets");
		if (ImGui::Button("Subtle Noise")) {
			ApplyPreset(NoisePreset::SUBTLE);
		}
		ImGui::SameLine();
		if (ImGui::Button("Medium Noise")) {
			ApplyPreset(NoisePreset::MEDIUM);
		}
		if (ImGui::Button("Heavy Noise")) {
			ApplyPreset(NoisePreset::HEAVY);
		}
		ImGui::SameLine();
		if (ImGui::Button("Digital Rain")) {
			ApplyPreset(NoisePreset::DIGITAL_RAIN);
		}
		ImGui::Separator();

		if (ImGui::Button("Reset")) {
			SetEnabled(false);
		}

		ImGui::Separator();
		if (isEnabled_) {
			// 個別パラメータ調整
			if (ImGui::TreeNode("Settings")) {

				ImGui::Text("master");

				float intensity = materialData_.noiseIntensity;
				if (ImGui::SliderFloat("Noise Intensity", &intensity, 0.0f, 1.0f)) {
					SetNoiseIntensity(intensity);
				}
				float noiseInterval = materialData_.noiseInterval;
				if (ImGui::SliderFloat("noise Interval", &noiseInterval, 0.0f, 1.0f)) {
					SetNoiseInterval(noiseInterval);
				}
				if (ImGui::SliderFloat("Animation Speed", &animationSpeed_, 0.0f, 5.0f)) {
					// アニメーション速度の変更は即座に反映
				}

				ImGui::Text("colorNoise");
				float colorNoiseInternsity = materialData_.colorNoiseInternsity;
				if (ImGui::SliderFloat("color NoiseInternsity", &colorNoiseInternsity, 0.0f, 1.0f)) {
					SetcolorNoiseInternsity(colorNoiseInternsity);
				}

				ImGui::Text("grayScale");
				float blackIntensity = materialData_.blackIntensity;
				if (ImGui::SliderFloat("Black Intensity", &blackIntensity, 0.0f, 1.0f)) {
					materialData_.blackIntensity = blackIntensity;
					UpdateConstantBuffer();
				}
				ImGui::Text("blockNoise");

				float blockIntensity = materialData_.blockIntensity;
				if (ImGui::SliderFloat("block Intensity", &blockIntensity, 0.0f, 1.0f)) {
					materialData_.blockIntensity = blockIntensity;
				}

				float blockProbability = materialData_.blockProbability;
				if (ImGui::SliderFloat("block Probability", &blockProbability, 0.0f, 1.0f)) {
					materialData_.blockProbability = blockProbability;
				}

				float reverseProbability = materialData_.reverseProbability;
				if (ImGui::SliderFloat("reverse Probability", &reverseProbability, 0.0f, 1.0f)) {
					materialData_.reverseProbability = reverseProbability;
				}
				Vector2 blockDivision = materialData_.blockDivision;
				if (ImGui::SliderFloat("Block DivisionX", &blockDivision.x, 1.0f, 50.0f)) { // 最大値を画面のxの半分にする
					SetBlockDivision(blockDivision);
				}
				if (ImGui::SliderFloat("Block DivisionY", &blockDivision.y, 1.0f, 30.0f)) { // 最大値を画面のyの半分にする
					SetBlockDivision(blockDivision);
				}
				ImGui::Text("scanLineNoise");

				float scanLineProbability = materialData_.scanLineProbability;
				if (ImGui::SliderFloat("scanLine Probability", &scanLineProbability, 0.0f, 1.0f)) {
					materialData_.scanLineProbability = scanLineProbability;
				}


				ImGui::TreePop();
			}

			// 情報表示
			ImGui::Separator();
			ImGui::Text("Current Time: %.2f", materialData_.time);
			ImGui::Text("Noise Intensity: %.2f", materialData_.noiseIntensity);
			ImGui::Text("noise Interval: %.2f", materialData_.noiseInterval);
			ImGui::Text("Color NoiseInternsity: %.2f", materialData_.colorNoiseInternsity);
			ImGui::Text("Block Division: x.%.2f,y.%.2", materialData_.blockDivision.x, materialData_.blockDivision.y);
			ImGui::Text("Animation Speed: %.2f", animationSpeed_);
		}
	}
}

Microsoft::WRL::ComPtr<IDxcBlob> NoiseEffect::CompileShader(
	const std::wstring& filePath,
	const wchar_t* profile) {

	// DirectXCommonの既存のCompileShader関数を使用
	return DirectXCommon::CompileShader(
		filePath,
		profile,
		dxCommon_->GetDxcUtils(),
		dxCommon_->GetDxcCompiler(),
		dxCommon_->GetIncludeHandler());
}

void NoiseEffect::UpdateConstantBuffer() {
	if (mappedMaterialData_) {
		*mappedMaterialData_ = materialData_;
	}
}