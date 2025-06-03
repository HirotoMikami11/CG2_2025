#include "OffscreenRenderer.h"

void OffscreenRenderer::Initialize(DirectXCommon* dxCommon, uint32_t width, uint32_t height) {
	dxCommon_ = dxCommon;
	width_ = width;
	height_ = height;

	// リソース作成
	CreateRenderTargetTexture();
	CreateDepthStencilTexture();
	CreateRTV();
	CreateDSV();
	CreateSRV();
	CreatePSO();
	CreateVertexBuffer();

	// ビューポート設定
	viewport_.Width = static_cast<float>(width_);
	viewport_.Height = static_cast<float>(height_);
	viewport_.TopLeftX = 0.0f;
	viewport_.TopLeftY = 0.0f;
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;

	// シザー矩形設定
	scissorRect_.left = 0;
	scissorRect_.right = width_;
	scissorRect_.top = 0;
	scissorRect_.bottom = height_;


	// グリッチエフェクトを初期化
	glitchEffect_ = std::make_unique<GlitchEffect>();
	glitchEffect_->Initialize(dxCommon);

	// ノイズエフェクトを初期化
	noiseEffect_ = std::make_unique<NoiseEffect>();
	noiseEffect_->Initialize(dxCommon);
	//初期化完了のログを出す
	Logger::Log(Logger::GetStream(), "Complete OffscreenRenderer initialized !!\n");
}

void OffscreenRenderer::Finalize() {
	// グリッチエフェクトの終了処理
	if (glitchEffect_) {
		glitchEffect_->Finalize();
		glitchEffect_.reset();
	}

	if (noiseEffect_) {
		noiseEffect_->Finalize();
		noiseEffect_.reset();
	}

	// 頂点データのマッピング解除
	if (vertexData_) {
		vertexBuffer_->Unmap(0, nullptr);
		vertexData_ = nullptr;
	}

	// マテリアルデータのマッピング解除
	if (materialData_) {
		materialBuffer_->Unmap(0, nullptr);
		materialData_ = nullptr;
	}

	// Transformデータのマッピング解除
	if (transformData_) {
		transformBuffer_->Unmap(0, nullptr);
		transformData_ = nullptr;
	}

	// DescriptorHeapManagerからディスクリプタを解放
	auto descriptorManager = dxCommon_->GetDescriptorManager();
	if (descriptorManager) {
		if (rtvHandle_.isValid) {
			descriptorManager->ReleaseRTV(rtvHandle_.index);
		}
		if (dsvHandle_.isValid) {
			descriptorManager->ReleaseDSV(dsvHandle_.index);
		}
		if (srvHandle_.isValid) {
			descriptorManager->ReleaseSRV(srvHandle_.index);
		}
	}

	Logger::Log(Logger::GetStream(), "OffscreenRenderer finalized.\n");
}

void OffscreenRenderer::Update(float deltaTime) {
	// グリッチエフェクトの更新
	if (glitchEffect_) {
		glitchEffect_->Update(deltaTime);
	}
	if (noiseEffect_) {
		noiseEffect_->Update(deltaTime);
	}

}

void OffscreenRenderer::PreDraw() {
	auto commandList = dxCommon_->GetCommandList();

	// レンダーターゲットをレンダーターゲット状態に遷移
	barrier_.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier_.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier_.Transition.pResource = renderTargetTexture_.Get();
	barrier_.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier_.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandList->ResourceBarrier(1, &barrier_);

	// レンダーターゲットとデプスステンシルを設定
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHandle_.cpuHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHandle_.cpuHandle;
	commandList->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);

	// クリア
	float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// ビューポートとシザー矩形を設定
	commandList->RSSetViewports(1, &viewport_);
	commandList->RSSetScissorRects(1, &scissorRect_);

	// 描画用のデスクリプタヒープを設定
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeaps[] = { dxCommon_->GetDescriptorManager()->GetSRVHeapComPtr() };
	commandList->SetDescriptorHeaps(1, descriptorHeaps->GetAddressOf());

	// 通常の描画設定を適用（既存のPSOを使用）
	commandList->SetGraphicsRootSignature(dxCommon_->GetRootSignature());
	commandList->SetPipelineState(dxCommon_->GetPipelineState());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void OffscreenRenderer::PostDraw() {
	auto commandList = dxCommon_->GetCommandList();

	// レンダーターゲットをシェーダーリソース状態に遷移
	barrier_.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier_.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	commandList->ResourceBarrier(1, &barrier_);
}

void OffscreenRenderer::DrawOffscreenTexture(float x, float y, float width, float height) {
	auto commandList = dxCommon_->GetCommandList();

	// 引数を使用（警告対策）
	x, y;

	// Transform更新（スプライト用のビュープロジェクション使用）
	transformData_->World = MakeIdentity4x4();
	transformData_->WVP = MakeViewProjectionMatrixSprite();



	if (noiseEffect_ && noiseEffect_->IsEnabled()) {
		// =================
		// ノイズエフェクト描画
		// =================
		commandList->SetGraphicsRootSignature(noiseEffect_->GetRootSignature());
		commandList->SetPipelineState(noiseEffect_->GetPipelineState());
		commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
		commandList->SetGraphicsRootConstantBufferView(0, noiseEffect_->GetMaterialBuffer()->GetGPUVirtualAddress());
		commandList->SetGraphicsRootConstantBufferView(1, transformBuffer_->GetGPUVirtualAddress());
		commandList->SetGraphicsRootDescriptorTable(2, srvHandle_.gpuHandle);
	} else if (glitchEffect_ && glitchEffect_->IsEnabled()) {// グリッチエフェクトが有効かどうかで描画方法を分岐
		// =================
		// グリッチエフェクト描画
		// =================

		// グリッチエフェクト用のPSOを設定
		commandList->SetGraphicsRootSignature(glitchEffect_->GetRootSignature());
		commandList->SetPipelineState(glitchEffect_->GetPipelineState());

		// 頂点バッファを設定
		commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);

		// CBVを設定（グリッチマテリアル、Transform、テクスチャ）
		commandList->SetGraphicsRootConstantBufferView(0, glitchEffect_->GetMaterialBuffer()->GetGPUVirtualAddress());
		commandList->SetGraphicsRootConstantBufferView(1, transformBuffer_->GetGPUVirtualAddress());
		commandList->SetGraphicsRootDescriptorTable(2, srvHandle_.gpuHandle);
	} else {
		// =================
		// 通常描画
		// =================

		// マテリアルデータ更新
		materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
		materialData_->enableLighting = false;
		materialData_->useLambertianReflectance = false;
		materialData_->uvTransform = MakeIdentity4x4();

		// 通常のオフスクリーン描画用のPSOを設定
		commandList->SetGraphicsRootSignature(rootSignature_.Get());
		commandList->SetPipelineState(pipelineState_.Get());

		// 頂点バッファを設定
		commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);

		// CBVを設定（通常のマテリアル、Transform、テクスチャ）
		commandList->SetGraphicsRootConstantBufferView(0, materialBuffer_->GetGPUVirtualAddress());
		commandList->SetGraphicsRootConstantBufferView(1, transformBuffer_->GetGPUVirtualAddress());
		commandList->SetGraphicsRootDescriptorTable(2, srvHandle_.gpuHandle);
	}

	// 描画実行
	commandList->DrawInstanced(6, 1, 0, 0);

	// 通常の描画設定に戻す
	commandList->SetGraphicsRootSignature(dxCommon_->GetRootSignature());
	commandList->SetPipelineState(dxCommon_->GetPipelineState());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void OffscreenRenderer::CreateRenderTargetTexture() {
	// リソース設定
	///オフスクリーン描画先となるテクスチャを作成
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(width_);			//横幅
	resourceDesc.Height = UINT(height_);			//高さ
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	// ヒープ設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;


	// オフスクリーンレンダリング用のクリアカラーを設定
	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	clearValue.Color[0] = 0.1f;
	clearValue.Color[1] = 0.25f;
	clearValue.Color[2] = 0.5f;
	clearValue.Color[3] = 1.0f;

	// リソース作成
	HRESULT hr = dxCommon_->GetDevice()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&clearValue,
		IID_PPV_ARGS(&renderTargetTexture_));

	assert(SUCCEEDED(hr));

	// レンダーターゲットビューを作成するためのハンドルを取得
	Logger::Log(Logger::GetStream(), "Complete create offscreen render target texture!!\n");
}

void OffscreenRenderer::CreateDepthStencilTexture() {
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width_;
	resourceDesc.Height = height_;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	HRESULT hr = dxCommon_->GetDevice()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(&depthStencilTexture_));

	assert(SUCCEEDED(hr));
	Logger::Log(Logger::GetStream(), "Complete create offscreen depth stencil texture!!\n");
}

void OffscreenRenderer::CreateRTV() {
	auto descriptorManager = dxCommon_->GetDescriptorManager();
	if (!descriptorManager) {
		Logger::Log(Logger::GetStream(), "DescriptorManager is null\n");
		return;
	}

	// RTVを割り当て
	rtvHandle_ = descriptorManager->AllocateRTV();
	if (!rtvHandle_.isValid) {
		Logger::Log(Logger::GetStream(), "Failed to allocate RTV for offscreen renderer\n");
		return;
	}

	// RTV作成
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	dxCommon_->GetDevice()->CreateRenderTargetView(
		renderTargetTexture_.Get(),
		&rtvDesc,
		rtvHandle_.cpuHandle);

	Logger::Log(Logger::GetStream(), std::format("Complete create offscreen RTV (Index: {})!!\n", rtvHandle_.index));
}

void OffscreenRenderer::CreateDSV() {
	auto descriptorManager = dxCommon_->GetDescriptorManager();
	if (!descriptorManager) {
		Logger::Log(Logger::GetStream(), "DescriptorManager is null\n");
		return;
	}

	// DSVを割り当て
	dsvHandle_ = descriptorManager->AllocateDSV();
	if (!dsvHandle_.isValid) {
		Logger::Log(Logger::GetStream(), "Failed to allocate DSV for offscreen renderer\n");
		return;
	}

	// DSV作成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	dxCommon_->GetDevice()->CreateDepthStencilView(
		depthStencilTexture_.Get(),
		&dsvDesc,
		dsvHandle_.cpuHandle);

	Logger::Log(Logger::GetStream(), std::format("Complete create offscreen DSV (Index: {})!!\n", dsvHandle_.index));
}

void OffscreenRenderer::CreateSRV() {
	auto descriptorManager = dxCommon_->GetDescriptorManager();
	if (!descriptorManager) {
		Logger::Log(Logger::GetStream(), "DescriptorManager is null\n");
		return;
	}

	// SRVを割り当て
	srvHandle_ = descriptorManager->AllocateSRV();
	if (!srvHandle_.isValid) {
		Logger::Log(Logger::GetStream(), "Failed to allocate SRV for offscreen renderer\n");
		return;
	}

	// SRV作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	dxCommon_->GetDevice()->CreateShaderResourceView(
		renderTargetTexture_.Get(),
		&srvDesc,
		srvHandle_.cpuHandle);

	Logger::Log(Logger::GetStream(), std::format("Complete create offscreen SRV (Index: {})!!\n", srvHandle_.index));
}

void OffscreenRenderer::CreatePSO() {

	//																			//
	//								RootSignature作成							//
	//																			//

	// オフスクリーン描画用の簡素化されたRootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// DescriptorRange
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// RootParameter（3つのみ：Material、Transform、Texture）
	D3D12_ROOT_PARAMETER rootParameters[3] = {};
	// Material (b0)
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

	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;	//0~1の範囲リピートする
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;	//
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;	//

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


	//																			//
	//							InputLayoutの設定								//
	//																			//

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


	//																			//
	//							BlendStateの設定									//
	//																			//

	// BlendState設定
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//																			//
	//						RasterizerStateの設定								//
	//																			//

	// RasterizerState設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// 新しいフルスクリーン用シェーダーをコンパイル
	vertexShaderBlob_ = CompileShader(L"Fullscreen.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob_ != nullptr);

	pixelShaderBlob_ = CompileShader(L"Fullscreen.PS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob_ != nullptr);

	//																			//
	//						DepthStencilStateの設定								//
	//																			//

	// DepthStencilState設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = false; // オフスクリーン描画では深度テストを無効
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;



	//																			//
	//								PSOの設定									//
	//																			//

	// PSO作成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPipelineStateDesc.VS = { vertexShaderBlob_->GetBufferPointer(), vertexShaderBlob_->GetBufferSize() };
	graphicsPipelineStateDesc.PS = { pixelShaderBlob_->GetBufferPointer(), pixelShaderBlob_->GetBufferSize() };
	graphicsPipelineStateDesc.BlendState = blendDesc;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
	
	//書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	
	//どのように画面に色を打ち込むかの設定
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	
	// 深度ステンシルの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;


	// 実際にPSO生成
	hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&pipelineState_));
	assert(SUCCEEDED(hr));

	// PSOの生成が成功したことをログに出力
	Logger::Log(Logger::GetStream(), "Complete create offscreen PSO!!\n");
}

void OffscreenRenderer::CreateVertexBuffer() {
	// 6頂点のクアッド用頂点バッファ作成
	vertexBuffer_ = CreateBufferResource(dxCommon_->GetDeviceComPtr(), sizeof(VertexData) * 6);

	// 頂点バッファビュー作成
	vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 6;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// 頂点データのマッピング
	vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

	// 初期の頂点データ設定（フルスクリーンクアッド）
	// 一つ目の三角形
	// 左下
	vertexData_[0].position = { 0.0f, static_cast<float>(GraphicsConfig::kClientHeight), 0.0f, 1.0f };
	vertexData_[0].texcoord = { 0.0f, 1.0f };
	vertexData_[0].normal = { 0.0f, 0.0f, -1.0f };
	// 左上
	vertexData_[1].position = { 0.0f, 0.0f, 0.0f, 1.0f };
	vertexData_[1].texcoord = { 0.0f, 0.0f };
	vertexData_[1].normal = { 0.0f, 0.0f, -1.0f };
	// 右下
	vertexData_[2].position = { static_cast<float>(GraphicsConfig::kClientWidth), static_cast<float>(GraphicsConfig::kClientHeight), 0.0f, 1.0f };
	vertexData_[2].texcoord = { 1.0f, 1.0f };
	vertexData_[2].normal = { 0.0f, 0.0f, -1.0f };
	// 二つ目の三角形
	// 左上
	vertexData_[3].position = { 0.0f, 0.0f, 0.0f, 1.0f };
	vertexData_[3].texcoord = { 0.0f, 0.0f };
	vertexData_[3].normal = { 0.0f, 0.0f, -1.0f };
	// 右上
	vertexData_[4].position = { static_cast<float>(GraphicsConfig::kClientWidth), 0.0f, 0.0f, 1.0f };
	vertexData_[4].texcoord = { 1.0f, 0.0f };
	vertexData_[4].normal = { 0.0f, 0.0f, -1.0f };
	// 右下
	vertexData_[5].position = { static_cast<float>(GraphicsConfig::kClientWidth), static_cast<float>(GraphicsConfig::kClientHeight), 0.0f, 1.0f };
	vertexData_[5].texcoord = { 1.0f, 1.0f };
	vertexData_[5].normal = { 0.0f, 0.0f, -1.0f };

	// マテリアルバッファ作成
	materialBuffer_ = CreateBufferResource(dxCommon_->GetDeviceComPtr(), sizeof(MaterialData));
	materialBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

	// 初期マテリアル設定
	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData_->enableLighting = false;
	materialData_->useLambertianReflectance = false;
	materialData_->uvTransform = MakeIdentity4x4();

	// Transform バッファ作成
	transformBuffer_ = CreateBufferResource(dxCommon_->GetDeviceComPtr(), sizeof(TransformationMatrix));
	transformBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&transformData_));

	// 初期Transform設定
	transformData_->World = MakeIdentity4x4();
	transformData_->WVP = MakeViewProjectionMatrixSprite();

	Logger::Log(Logger::GetStream(), "Complete create offscreen vertex buffer!!\n");
}

Microsoft::WRL::ComPtr<IDxcBlob> OffscreenRenderer::CompileShader(
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

void OffscreenRenderer::ImGui() {
	///閉じたり開いたりできる
	if (ImGui::CollapsingHeader("Offscreen Renderer")) {
		// オフスクリーンのサイズ
		ImGui::Text("Render Target Size: %dx%d", width_, height_);
	
		ImGui::Text("Is Valid: %s", IsValid() ? "Yes" : "No");
		// レンダーターゲットのハンドル情報
		if (srvHandle_.isValid) {
			ImGui::Text("SRV Index: %d", srvHandle_.index);
		}

		// グリッチエフェクトのImGui
		if (glitchEffect_) {
			glitchEffect_->ImGui();
		}
		if (noiseEffect_) {
			noiseEffect_->ImGui();
		}
	}
}