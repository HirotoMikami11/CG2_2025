#include "PSOFactory.h"
#include <format>
#include <cassert>

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
	//constのrootSignatureBuilderをコピーしようとするとエラー出るので仕方なくconst外した。
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

	// シェーダーをコンパイル（キャッシュ使用）
	auto vertexShaderBlob = CompileShader(descriptor.GetVertexShader());
	auto pixelShaderBlob = CompileShader(descriptor.GetPixelShader());

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

Microsoft::WRL::ComPtr<IDxcBlob> PSOFactory::CompileShader(const PSODescriptor::ShaderInfo& shaderInfo) {
	// キャッシュキーを生成
	std::wstring cacheKey = CreateShaderCacheKey(shaderInfo);

	// キャッシュをチェック
	{
		std::lock_guard<std::mutex> lock(cacheMutex_);
		auto it = shaderCache_.find(cacheKey);
		if (it != shaderCache_.end()) {
			Logger::Log(Logger::GetStream(),
				Logger::ConvertString(std::format(L"PSOFactory: Using cached shader - {}\n", cacheKey)));
			return it->second;
		}
	}

	// キャッシュにない場合はコンパイル
	Logger::Log(Logger::GetStream(),
		Logger::ConvertString(std::format(L"PSOFactory: Compiling shader - {}\n", shaderInfo.filePath)));

	// ファイルを読み込み
	Microsoft::WRL::ComPtr<IDxcBlobEncoding> shaderSource;
	HRESULT hr = dxcUtils_->LoadFile(shaderInfo.filePath.c_str(), nullptr, &shaderSource);
	if (FAILED(hr)) {
		Logger::Log(Logger::GetStream(),
			Logger::ConvertString(std::format(L"PSOFactory: Failed to load shader file - {}\n", shaderInfo.filePath)));
		return nullptr;
	}

	// コンパイル設定
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;

	// コンパイル引数
	LPCWSTR arguments[] = {
		shaderInfo.filePath.c_str(),     // ファイル名
		L"-E", shaderInfo.entryPoint.c_str(),  // エントリーポイント
		L"-T", shaderInfo.target.c_str(),      // ターゲット
		L"-Zi", L"-Qembed_debug",         // デバッグ情報
		L"-Od",                           // 最適化なし（デバッグ時）
		L"-Zpr",                          // 行優先行列
	};

	// コンパイル実行
	Microsoft::WRL::ComPtr<IDxcResult> shaderResult;
	hr = dxcCompiler_->Compile(
		&shaderSourceBuffer,
		arguments,
		_countof(arguments),
		includeHandler_,
		IID_PPV_ARGS(&shaderResult));

	if (FAILED(hr)) {
		Logger::Log(Logger::GetStream(), "PSOFactory: DXC compile call failed\n");
		return nullptr;
	}

	// コンパイル結果をチェック
	Microsoft::WRL::ComPtr<IDxcBlobUtf8> errors;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
	if (errors && errors->GetStringLength() > 0) {
		Logger::Log(Logger::GetStream(),
			std::format("PSOFactory: Shader compile error - {}\n", errors->GetStringPointer()));
		return nullptr;
	}

	// コンパイル済みシェーダーを取得
	Microsoft::WRL::ComPtr<IDxcBlob> compiledShader;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&compiledShader), nullptr);
	if (FAILED(hr)) {
		Logger::Log(Logger::GetStream(), "PSOFactory: Failed to get compiled shader\n");
		return nullptr;
	}

	// キャッシュに保存
	{
		std::lock_guard<std::mutex> lock(cacheMutex_);
		///ComPtrをstd::moveで移動してキャッシュに保存
		///じゃないとxmemoryでエラーが出る、そもそものキャッシュに保存をやめるべき？
		shaderCache_.emplace(cacheKey, std::move(compiledShader));
	}

	Logger::Log(Logger::GetStream(),
		Logger::ConvertString(std::format(L"PSOFactory: Successfully compiled shader - {}\n", shaderInfo.filePath)));

	return compiledShader;
}

std::wstring PSOFactory::CreateShaderCacheKey(const PSODescriptor::ShaderInfo& shaderInfo) const {
	// ファイルパス、エントリーポイント、ターゲットを組み合わせてキーを作成
	return std::format(L"{}|{}|{}",
		shaderInfo.filePath,
		shaderInfo.entryPoint,
		shaderInfo.target);
}

void PSOFactory::ClearShaderCache() {
	std::lock_guard<std::mutex> lock(cacheMutex_);
	size_t cacheSize = shaderCache_.size();
	shaderCache_.clear();

	Logger::Log(Logger::GetStream(),
		std::format("PSOFactory: Cleared shader cache ({} shaders)\n", cacheSize));
}