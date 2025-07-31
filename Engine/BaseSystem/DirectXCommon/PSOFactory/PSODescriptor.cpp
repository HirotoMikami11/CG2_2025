#include "PSODescriptor.h"



///*-----------------------------------------------------------------------*///
//																			//
///							よく使うものを実装								   ///
//																			//
///*-----------------------------------------------------------------------*///

PSODescriptor PSODescriptor::Create3D() {
	PSODescriptor desc;

	// 3Dオブジェクト用のデフォルト設定
	desc.SetVertexShader(L"resources/Shader/Object3d/Object3d.VS.hlsl", L"main")
		.SetPixelShader(L"resources/Shader/Object3d/Object3d.PS.hlsl", L"main")
		.SetBlendMode(BlendMode::AlphaBlend)
		.SetCullMode(CullMode::Back)
		.EnableDepth(true)
		.EnableDepthWrite(true)
		.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	// 標準的な3D頂点レイアウト
	desc.AddInputElement({ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT })
		.AddInputElement({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT })
		.AddInputElement({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT });

	return desc;
}

PSODescriptor PSODescriptor::CreateSprite() {
	PSODescriptor desc;

	// スプライト用のデフォルト設定（深度テストなし）
	desc.SetVertexShader(L"resources/Shader/Sprite/Sprite.VS.hlsl", L"main")
		.SetPixelShader(L"resources/Shader/Sprite/Sprite.PS.hlsl", L"main")
		.SetBlendMode(BlendMode::AlphaBlend)
		.SetCullMode(CullMode::Back)
		.EnableDepth(false)
		.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	// スプライト用頂点レイアウト（3Dと同じ）
	desc.AddInputElement({ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT })
		.AddInputElement({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT })
		.AddInputElement({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT });

	return desc;
}

PSODescriptor PSODescriptor::CreateLine() {
	PSODescriptor desc;

	// 線分用のデフォルト設定
	desc.SetVertexShader(L"resources/Shader/Line/Line.VS.hlsl", L"main")
		.SetPixelShader(L"resources/Shader/Line/Line.PS.hlsl", L"main")
		.SetBlendMode(BlendMode::AlphaBlend)
		.SetCullMode(CullMode::None)  // 線分は両面描画
		.EnableDepth(true)
		.EnableDepthWrite(true)
		.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);

	// 線分用頂点レイアウト（色情報を含む）
	desc.AddInputElement({ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT })
		.AddInputElement({ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT })
		.AddInputElement({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT })
		.AddInputElement({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT });

	return desc;
}

PSODescriptor PSODescriptor::CreatePostEffect() {
	PSODescriptor desc;

	// ポストエフェクト用のデフォルト設定（フルスクリーン三角形用）
	desc.SetVertexShader(L"resources/Shader/FullscreenTriangle/FullscreenTriangle.VS.hlsl", L"main")
		.SetPixelShader(L"resources/Shader/FullscreenTriangle/FullscreenTriangle.PS.hlsl", L"main")
		.SetBlendMode(BlendMode::None)
		.SetCullMode(CullMode::Back)
		.EnableDepth(false)
		.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	// ポストエフェクト用の簡素な頂点レイアウト
	desc.AddInputElement({ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT })
		.AddInputElement({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT });

	return desc;
}

PSODescriptor PSODescriptor::CreatePostEffectWithDepth() {
	PSODescriptor desc;

	// 深度テクスチャを使うポストエフェクト用設定
	desc.SetVertexShader(L"resources/Shader/FullscreenTriangle/FullscreenTriangle.VS.hlsl", L"main")
		.SetPixelShader(L"", L"main")  // 個別に設定する必要がある
		.SetBlendMode(BlendMode::None)
		.SetCullMode(CullMode::Back)
		.EnableDepth(false)  // ポストエフェクトでは深度テストなし
		.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	// ポストエフェクト用頂点レイアウト（位置とUV座標のみ）
	desc.AddInputElement({ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT })
		.AddInputElement({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT });

	return desc;
}

PSODescriptor PSODescriptor::CreatePostEffectColorOnly() {
	PSODescriptor desc;

	// カラーテクスチャのみのポストエフェクト用設定
	desc.SetVertexShader(L"resources/Shader/FullscreenTriangle/FullscreenTriangle.VS.hlsl", L"main")
		.SetPixelShader(L"", L"main")  // 個別に設定する必要がある
		.SetBlendMode(BlendMode::None)
		.SetCullMode(CullMode::Back)
		.EnableDepth(false)
		.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	// ポストエフェクト用頂点レイアウト
	desc.AddInputElement({ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT })
		.AddInputElement({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT });

	return desc;
}
///*-----------------------------------------------------------------------*///
//																			//
///								各メソッドの実装							   ///
//																			//
///*-----------------------------------------------------------------------*///


PSODescriptor& PSODescriptor::SetVertexShader(const std::wstring& path, const std::wstring& entryPoint) {
	vertexShader_.filePath = path;
	vertexShader_.entryPoint = entryPoint;
	vertexShader_.target = L"vs_6_0";  // vs_6_0
	return *this;
}

PSODescriptor& PSODescriptor::SetPixelShader(const std::wstring& path, const std::wstring& entryPoint) {
	pixelShader_.filePath = path;
	pixelShader_.entryPoint = entryPoint;
	pixelShader_.target = L"ps_6_0";  // ps_6_0
	return *this;
}

PSODescriptor& PSODescriptor::SetBlendMode(BlendMode mode) {
	blendMode_ = mode;
	return *this;
}

PSODescriptor& PSODescriptor::SetCullMode(CullMode mode) {
	cullMode_ = mode;
	return *this;
}

PSODescriptor& PSODescriptor::EnableDepth(bool enable, D3D12_COMPARISON_FUNC func) {
	depthEnable_ = enable;
	depthFunc_ = func;
	return *this;
}

PSODescriptor& PSODescriptor::EnableDepthWrite(bool enable) {
	depthWriteEnable_ = enable;
	return *this;
}

PSODescriptor& PSODescriptor::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE type) {
	topologyType_ = type;
	return *this;
}

PSODescriptor& PSODescriptor::AddInputElement(const InputElement& element) {
	inputElements_.push_back(element);
	return *this;
}

PSODescriptor& PSODescriptor::ClearInputElements() {
	inputElements_.clear();
	return *this;
}

PSODescriptor& PSODescriptor::SetRenderTargetFormat(DXGI_FORMAT format) {
	renderTargetFormat_ = format;
	return *this;
}

PSODescriptor& PSODescriptor::SetDepthStencilFormat(DXGI_FORMAT format) {
	depthStencilFormat_ = format;
	return *this;
}

///*-----------------------------------------------------------------------*///
//																			//
///						D3D12構造体生成メソッドの実装		　　　　　		   ///
//																			//
///*-----------------------------------------------------------------------*///

D3D12_BLEND_DESC PSODescriptor::GetBlendDesc() const {
	D3D12_BLEND_DESC blendDesc{};

	// デフォルト設定
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;

	// ブレンドモードに応じた設定
	switch (blendMode_) {
	case BlendMode::None:
		// ブレンドなし
		blendDesc.RenderTarget[0].BlendEnable = FALSE;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		break;

	case BlendMode::AlphaBlend:
		// アルファブレンド（最も一般的）
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		break;

	case BlendMode::Add:
		// 加算合成
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		break;

	case BlendMode::Subtract:
		// 減算合成
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		break;

	case BlendMode::Multiply:
		// 乗算合成
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_DEST_COLOR;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		break;
	}

	return blendDesc;
}

D3D12_RASTERIZER_DESC PSODescriptor::GetRasterizerDesc() const {
	D3D12_RASTERIZER_DESC rasterizerDesc{};

	// カリングモードの設定
	switch (cullMode_) {
	case CullMode::None:
		rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
		break;
	case CullMode::Back:
		rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
		break;
	case CullMode::Front:
		rasterizerDesc.CullMode = D3D12_CULL_MODE_FRONT;
		break;
	}

	// その他の設定
	rasterizerDesc.FillMode = fillMode_;											// ポリゴンの塗りつぶしモード
	rasterizerDesc.FrontCounterClockwise = FALSE;									// 時計回りを表面とする
	rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;	// スロープスケールされた深度バイアスのデフォルト値
	rasterizerDesc.DepthClipEnable = TRUE;											// 深度クリップを有効にする(これがないとカメラのfarは死んだも同然)
	rasterizerDesc.MultisampleEnable = FALSE;										//アンチエイリアシング無効
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rasterizerDesc;
}

D3D12_DEPTH_STENCIL_DESC PSODescriptor::GetDepthStencilDesc() const {
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};

	// 深度テストの設定
	depthStencilDesc.DepthEnable = depthEnable_;
	depthStencilDesc.DepthWriteMask = depthWriteEnable_ ?
		D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = depthFunc_;

	// ステンシルテストの設定（今回は使用しない）
	depthStencilDesc.StencilEnable = FALSE;
	depthStencilDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	depthStencilDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

	// 前面と背面のステンシル設定（デフォルト値）
	const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp = {
		D3D12_STENCIL_OP_KEEP,
		D3D12_STENCIL_OP_KEEP,
		D3D12_STENCIL_OP_KEEP,
		D3D12_COMPARISON_FUNC_ALWAYS
	};
	depthStencilDesc.FrontFace = defaultStencilOp;
	depthStencilDesc.BackFace = defaultStencilOp;

	return depthStencilDesc;
}

std::vector<D3D12_INPUT_ELEMENT_DESC> PSODescriptor::CreateInputElementDescs() const {
	std::vector<D3D12_INPUT_ELEMENT_DESC> descs;
	descs.reserve(inputElements_.size());

	// 文字列を保持するためのvectorをクリアして再構築
	semanticNames_.clear();
	semanticNames_.reserve(inputElements_.size());

	// InputElementからD3D12_INPUT_ELEMENT_DESCに変換
	for (const auto& element : inputElements_) {
		semanticNames_.push_back(element.semanticName);

		D3D12_INPUT_ELEMENT_DESC desc{};
		desc.SemanticName = semanticNames_.back().c_str();  // 保持した文字列のポインタを使用
		desc.SemanticIndex = element.semanticIndex;
		desc.Format = element.format;
		desc.InputSlot = element.inputSlot;
		desc.AlignedByteOffset = element.alignedByteOffset;
		desc.InputSlotClass = element.inputSlotClass;
		desc.InstanceDataStepRate = element.instanceDataStepRate;

		descs.push_back(desc);
	}

	return descs;
}