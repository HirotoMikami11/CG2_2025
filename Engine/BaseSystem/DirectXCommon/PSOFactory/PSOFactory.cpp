#include "PSOFactory.h"
#include <format>
#include <cassert>
#include "BaseSystem/DirectXCommon/DirectXCommon.h"

void PSOFactory::Initialize(ID3D12Device* device,
	IDxcUtils* dxcUtils,
	IDxcCompiler3* dxcCompiler,
	IDxcIncludeHandler* includeHandler) {
	assert(device && "Device must not be null");
	assert(dxcUtils && "DxcUtils must not be null");
	assert(dxcCompiler && "DxcCompiler must not be null");
	assert(includeHandler && "IncludeHandler must not be null");

	device_ = device;
	dxcUtils_ = dxcUtils;
	dxcCompiler_ = dxcCompiler;
	includeHandler_ = includeHandler;

	isInitialized_ = true;

	Logger::Log(Logger::GetStream(), "PSOFactory: Initialized successfully\n");
}

PSOFactory::PSOInfo PSOFactory::CreatePSO(const PSODescriptor& descriptor,
	RootSignatureBuilder& rootSignatureBuilder) {
	PSOInfo result;

	if (!isInitialized_) {
		Logger::Log(Logger::GetStream(), "PSOFactory: Error - Not initialized\n");
		return result;
	}

	// RootSignatureを作成
	result.rootSignature = rootSignatureBuilder.Build(device_);
	if (!result.rootSignature) {
		Logger::Log(Logger::GetStream(), "PSOFactory: Failed to create RootSignature\n");
		return result;
	}

	// PipelineStateを作成
	result.pipelineState = CreatePSO(descriptor, result.rootSignature.Get());
	if (!result.pipelineState) {
		Logger::Log(Logger::GetStream(), "PSOFactory: Failed to create PipelineState\n");
		result.rootSignature.Reset();  // 失敗時はRootSignatureも無効化
		return result;
	}

	return result;
}

Microsoft::WRL::ComPtr<ID3D12PipelineState> PSOFactory::CreatePSO(
	const PSODescriptor& descriptor,
	ID3D12RootSignature* existingRootSignature) {

	if (!isInitialized_ || !existingRootSignature) {
		Logger::Log(Logger::GetStream(), "PSOFactory: Error - Invalid parameters\n");
		return nullptr;
	}

	// シェーダーをコンパイル
	auto vertexShaderBlob = DirectXCommon::CompileShader(
		descriptor.GetVertexShader().filePath,
		descriptor.GetVertexShader().target.c_str(),
		dxcUtils_,
		dxcCompiler_,
		includeHandler_);

	auto pixelShaderBlob = DirectXCommon::CompileShader(
		descriptor.GetPixelShader().filePath,
		descriptor.GetPixelShader().target.c_str(),
		dxcUtils_,
		dxcCompiler_,
		includeHandler_);


	if (!vertexShaderBlob || !pixelShaderBlob) {
		Logger::Log(Logger::GetStream(), "PSOFactory: Failed to compile shaders\n");
		return nullptr;
	}

	// InputLayoutを取得
	auto inputElementDescs = descriptor.CreateInputElementDescs();
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs.empty() ? nullptr : inputElementDescs.data();
	inputLayoutDesc.NumElements = static_cast<UINT>(inputElementDescs.size());

	// PSO設定を構築
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};

	// RootSignature
	pipelineDesc.pRootSignature = existingRootSignature;

	// シェーダー
	pipelineDesc.VS = {
		vertexShaderBlob->GetBufferPointer(),
		vertexShaderBlob->GetBufferSize()
	};
	pipelineDesc.PS = {
		pixelShaderBlob->GetBufferPointer(),
		pixelShaderBlob->GetBufferSize()
	};

	// 各種設定
	pipelineDesc.InputLayout = inputLayoutDesc;
	pipelineDesc.BlendState = descriptor.GetBlendDesc();
	pipelineDesc.RasterizerState = descriptor.GetRasterizerDesc();
	pipelineDesc.DepthStencilState = descriptor.GetDepthStencilDesc();

	// レンダーターゲット
	pipelineDesc.NumRenderTargets = 1;
	pipelineDesc.RTVFormats[0] = descriptor.GetRenderTargetFormat();
	pipelineDesc.DSVFormat = descriptor.GetDepthStencilFormat();

	// プリミティブトポロジー
	pipelineDesc.PrimitiveTopologyType = descriptor.GetPrimitiveTopologyType();

	// サンプル設定
	pipelineDesc.SampleDesc.Count = 1;
	pipelineDesc.SampleDesc.Quality = 0;
	pipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// その他
	pipelineDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	pipelineDesc.NodeMask = 0;
	pipelineDesc.CachedPSO = { nullptr, 0 };
	pipelineDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	// PipelineStateを作成
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
	HRESULT hr = device_->CreateGraphicsPipelineState(
		&pipelineDesc,
		IID_PPV_ARGS(&pipelineState));

	if (FAILED(hr)) {
		Logger::Log(Logger::GetStream(),
			std::format("PSOFactory: Failed to create PipelineState (HRESULT: 0x{:08X})\n", hr));
		return nullptr;
	}

	Logger::Log(Logger::GetStream(), "PSOFactory: Successfully created PipelineState\n");
	return pipelineState;
}
