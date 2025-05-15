#include "BreakScreenEffect.h"
#include "DirectXCommon.h"

void BreakScreenEffect::Initialize(ID3D12Device* device)
{
	// 初期のランダム中心点を生成
	GenerateRandomCenter();

	// 三角形の頂点データを作成
	CreateTriangleVertices();

	// バッファリソースを作成
	CreateBuffers(device);

	// ポストエフェクト専用のPSOを作成
	CreatePostEffectPSO(device);

	// 初期化時は非アクティブ
	isActive = false;
	isCompleted = false;
	isMovingOut = false;
	isReturning = false;
	totalAnimationTime = 0.0f;
	phaseTime = 0.0f;
	breakScreenProgress = 0.0f;
}

void BreakScreenEffect::Update()
{
	if (!isActive) return;

	const float deltaTime = 1.0f / 60.0f;
	totalAnimationTime += deltaTime;
	phaseTime += deltaTime;

	// フェーズ1: 割れエフェクト (0 ~ shatterDuration)
	if (!isMovingOut && !isReturning) {
		// 割れエフェクトの進行度を計算 (0.0f ~ 1.0f)
		breakScreenProgress = std::clamp(phaseTime / shatterDuration, 0.0f, 1.0f);

		// 割れエフェクトが完了したら画面外への移動を開始
		if (breakScreenProgress >= 1.0f) {
			isMovingOut = true;
			phaseTime = 0.0f; // 次のフェーズ用にリセット
		}
	}

	// フェーズ2: 画面外への移動 (shatterDuration ~ shatterDuration + moveOutDuration)
	else if (isMovingOut && !isReturning) {
		// 画面外への移動が完了したら戻りを開始
		if (phaseTime >= moveOutDuration) {
			isMovingOut = false;
			isReturning = true;
			phaseTime = 0.0f; // 次のフェーズ用にリセット
		}
	}

	// フェーズ3: 元の位置への戻り (shatterDuration + moveOutDuration ~ total)
	else if (isReturning) {
		// 戻りが完了したらエフェクトを終了
		if (phaseTime >= returnDuration) {
			isReturning = false;
			isCompleted = true;
			///ここでイージングを停止させると、付随したパーティクルの演出も停止するため動かしたままにする（要修正）
			isActive = false; // 非アクティブにする
		}
	}

	// マテリアルデータの更新

	if (materialData) {
		materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
		materialData->enableLighting = false;
		materialData->useLambertianReflectance = false;
		materialData->uvTransform = MakeIdentity4x4();

	}
}
void BreakScreenEffect::Draw(ID3D12GraphicsCommandList* commandList,
	D3D12_GPU_DESCRIPTOR_HANDLE offScreenSRVHandle)
{
	///エフェクトが開始されていなければ、以下の処理はスキップ
	if (!isActive) return;

	// ポストエフェクト専用のルートシグネチャとPSOを設定
	commandList->SetGraphicsRootSignature(postEffectRootSignature.Get());
	commandList->SetPipelineState(postEffectPSO.Get());

	// テクスチャを設定
	commandList->SetGraphicsRootDescriptorTable(2, offScreenSRVHandle);
	commandList->SetGraphicsRootConstantBufferView(3, directionalLightBuffer->GetGPUVirtualAddress());

	// 頂点バッファを設定
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 8つの三角形を描画
	for (int i = 0; i < kTriangleCount; i++) {

		materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
		// マテリアルを設定
		commandList->SetGraphicsRootConstantBufferView(0, materialBuffer->GetGPUVirtualAddress());

		// 各三角形ごとに変形行列を計算
		Vector3Transform transform{};
		transform.scale = { 1.0f, 1.0f, 1.0f };
		transform.rotate = { 0.0f, 0.0f, 0.0f };
		transform.translate = { 0.0f, 0.0f, 0.0f };

		// 現在の移動距離を計算
		float currentMoveDistance = 0.0f;
		float easedProgress = 0.0f;

		if (isMovingOut) {
			// 画面外への移動
			easedProgress = easeInCirc(phaseTime / moveOutDuration);
			currentMoveDistance = breakScreenProgress * 0.5f + easedProgress * maxMoveDistance;
		} else if (isReturning) {
			//easeOutCubicで動きに波をつける
			easedProgress = easeOutCubic(phaseTime / returnDuration);
			currentMoveDistance = (breakScreenProgress * 0.5f + maxMoveDistance) * (1.0f - easedProgress);
		} else if (breakScreenProgress > 0.0f) {
			// 通常の割れエフェクト
			currentMoveDistance = breakScreenProgress * 0.5f;
		}

		// 三角形ごとの移動方向と回転を適用
		if (currentMoveDistance > 0.0f) {
			Vector3 direction = triangleDirections[i];
			float rotationAmount = currentMoveDistance * 0.6f;

			transform.translate = {
				direction.x * currentMoveDistance,
				direction.y * currentMoveDistance,
				0.0f
			};

			// 回転も適用
			transform.rotate = {
				0.0f, 0.0f, rotationAmount * (i % 2 == 0 ? 1.0f : -1.0f)
			};
		}

		// 変形行列を更新
		transformDatas[i]->World = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
		transformDatas[i]->WVP = transformDatas[i]->World;

		// トランスフォームバッファを設定
		commandList->SetGraphicsRootConstantBufferView(1, transformBuffers[i]->GetGPUVirtualAddress());

		// 三角形を描画
		commandList->DrawInstanced(3, 1, i * 3, 0);
	}
}
void BreakScreenEffect::Reset()
{
	// 新しいランダム中心点を生成
	GenerateRandomCenter();

	// 三角形の頂点データを再作成
	CreateTriangleVertices();

	// 頂点バッファを更新
	VertexData* vertexData = nullptr;
	vertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, triangleVertices.data(), sizeof(VertexData) * triangleVertices.size());
	vertexBuffer->Unmap(0, nullptr);

	// 状態をリセット
	breakScreenProgress = 0.0f;
	isCompleted = false;
	isMovingOut = false;
	isReturning = false;
	totalAnimationTime = 0.0f;
	phaseTime = 0.0f;
	isActive = false;
}
void BreakScreenEffect::GenerateRandomCenter()
{
	randomCenter.x = RandomFloat(-0.5f, 0.5f);
	randomCenter.y = RandomFloat(-0.5f, 0.5f);

	// ランダム中心点に基づいて各三角形の方向を計算
	Vector3 centerNDC = { randomCenter.x, randomCenter.y, 0.0f };

	// 8方向の基本ベクトル
	Vector3 baseDirections[8] = {
		{-1.0f, 1.0f, 0.0f},   // 上左
		{1.0f, 1.0f, 0.0f},    // 上右
		{1.0f, 0.5f, 0.0f},    // 右上
		{1.0f, -0.5f, 0.0f},   // 右下
		{1.0f, -1.0f, 0.0f},   // 下右
		{-1.0f, -1.0f, 0.0f},  // 下左
		{-1.0f, -0.5f, 0.0f},  // 左下
		{-1.0f, 0.5f, 0.0f}    // 左上
	};

	// 中心点からの相対的な方向を計算
	for (int i = 0; i < 8; i++) {
		Vector3 normalizedDirection = baseDirections[i];
		// 中心点に基づいて方向を調整
		normalizedDirection.x += randomCenter.x * 0.5f;
		normalizedDirection.y += randomCenter.y * 0.5f;
		triangleDirections[i] = Vector3Normalize(normalizedDirection);
	}
}

void BreakScreenEffect::CreateTriangleVertices()
{
	triangleVertices.clear();
	triangleVertices.reserve(24); // 8つの三角形 × 3頂点

	Vector3 normal = { 0.0f, 0.0f, 1.0f };

	// ランダム中心点を使用（NDC座標系）
	Vector4 center = { randomCenter.x, randomCenter.y, 0.0f, 1.0f };
	Vector2 uvCenter = { 0.5f + randomCenter.x * 0.5f, 0.5f - randomCenter.y * 0.5f };

	// 画面の四角の頂点（NDC座標系：-1〜1の範囲）
	Vector4 topLeft = { -1.0f, 1.0f, 0.0f, 1.0f };
	Vector4 topRight = { 1.0f, 1.0f, 0.0f, 1.0f };
	Vector4 bottomLeft = { -1.0f, -1.0f, 0.0f, 1.0f };
	Vector4 bottomRight = { 1.0f, -1.0f, 0.0f, 1.0f };
	Vector4 topCenter = { randomCenter.x, 1.0f, 0.0f, 1.0f };
	Vector4 bottomCenter = { randomCenter.x, -1.0f, 0.0f, 1.0f };
	Vector4 leftCenter = { -1.0f, randomCenter.y, 0.0f, 1.0f };
	Vector4 rightCenter = { 1.0f, randomCenter.y, 0.0f, 1.0f };

	// 対応するUV座標
	Vector2 uvTopLeft = { 0.0f, 0.0f };
	Vector2 uvTopRight = { 1.0f, 0.0f };
	Vector2 uvBottomLeft = { 0.0f, 1.0f };
	Vector2 uvBottomRight = { 1.0f, 1.0f };
	Vector2 uvTopCenter = { 0.5f + randomCenter.x * 0.5f, 0.0f };
	Vector2 uvBottomCenter = { 0.5f + randomCenter.x * 0.5f, 1.0f };
	Vector2 uvLeftCenter = { 0.0f, 0.5f - randomCenter.y * 0.5f };
	Vector2 uvRightCenter = { 1.0f, 0.5f - randomCenter.y * 0.5f };

	// 8つの三角形を時計回りで定義
	// 0: 上左の三角形
	triangleVertices.push_back({ topLeft, uvTopLeft, normal });
	triangleVertices.push_back({ topCenter, uvTopCenter, normal });
	triangleVertices.push_back({ center, uvCenter, normal });

	// 1: 上右の三角形
	triangleVertices.push_back({ topCenter, uvTopCenter, normal });
	triangleVertices.push_back({ topRight, uvTopRight, normal });
	triangleVertices.push_back({ center, uvCenter, normal });

	// 2: 右上の三角形
	triangleVertices.push_back({ topRight, uvTopRight, normal });
	triangleVertices.push_back({ rightCenter, uvRightCenter, normal });
	triangleVertices.push_back({ center, uvCenter, normal });

	// 3: 右下の三角形
	triangleVertices.push_back({ rightCenter, uvRightCenter, normal });
	triangleVertices.push_back({ bottomRight, uvBottomRight, normal });
	triangleVertices.push_back({ center, uvCenter, normal });

	// 4: 下右の三角形
	triangleVertices.push_back({ bottomRight, uvBottomRight, normal });
	triangleVertices.push_back({ bottomCenter, uvBottomCenter, normal });
	triangleVertices.push_back({ center, uvCenter, normal });

	// 5: 下左の三角形
	triangleVertices.push_back({ bottomCenter, uvBottomCenter, normal });
	triangleVertices.push_back({ bottomLeft, uvBottomLeft, normal });
	triangleVertices.push_back({ center, uvCenter, normal });

	// 6: 左下の三角形
	triangleVertices.push_back({ bottomLeft, uvBottomLeft, normal });
	triangleVertices.push_back({ leftCenter, uvLeftCenter, normal });
	triangleVertices.push_back({ center, uvCenter, normal });

	// 7: 左上の三角形
	triangleVertices.push_back({ leftCenter, uvLeftCenter, normal });
	triangleVertices.push_back({ topLeft, uvTopLeft, normal });
	triangleVertices.push_back({ center, uvCenter, normal });
}


void BreakScreenEffect::CreateBuffers(ID3D12Device* device)
{
	// 頂点バッファの作成
	vertexBuffer = CreateBufferResource(device, sizeof(VertexData) * triangleVertices.size());

	// 頂点データを書き込み
	VertexData* vertexData = nullptr;
	vertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, triangleVertices.data(), sizeof(VertexData) * triangleVertices.size());
	vertexBuffer->Unmap(0, nullptr);

	// 頂点バッファビューの設定
	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * triangleVertices.size());
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// マテリアルバッファを作成

	materialBuffer = CreateBufferResource(device, sizeof(Material));
	materialBuffer->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData->enableLighting = false;
	materialData->useLambertianReflectance = false;
	materialData->uvTransform = MakeIdentity4x4();


	// 8つのトランスフォームバッファを作成（各三角形用）
	for (int i = 0; i < kTriangleCount; ++i) {
		transformBuffers[i] = CreateBufferResource(device, sizeof(TransformationMatrix));
		transformBuffers[i]->Map(0, nullptr, reinterpret_cast<void**>(&transformDatas[i]));
		transformDatas[i]->WVP = MakeIdentity4x4();
		transformDatas[i]->World = MakeIdentity4x4();
	}

	// DirectionalLightバッファの作成（ルートシグネチャに合わせて必須）
	directionalLightBuffer = CreateBufferResource(device, sizeof(DirectionalLight));
	directionalLightBuffer->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData->intensity = 1.0f;
}

/// ここで生成していいかわからないけどここ以外で生成する場所がわからなかった
void BreakScreenEffect::CreatePostEffectPSO(ID3D12Device* device)
{
	// 1. ルートシグネチャの作成（DirectXCommonと同じ）
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// DescriptorRangeを作る
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// RootParameterを作成
	D3D12_ROOT_PARAMETER rootParameters[4] = {};
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
	// DirectionalLight (b1)
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[3].Descriptor.ShaderRegister = 1;

	// Samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);
	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);

	// ルートシグネチャをシリアライズ
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		assert(false);
	}

	// ルートシグネチャを作成
	hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(), IID_PPV_ARGS(&postEffectRootSignature));
	assert(SUCCEEDED(hr));

	// 2. InputLayoutの設定
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

	// 3. BlendStateの設定（アルファブレンド無効）
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].BlendEnable = FALSE; // アルファブレンド無効

	// 4. RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE; // カリング無効
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// 5. DepthStencilStateの設定（深度テスト無効）
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = FALSE; // 深度テスト無効
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // 深度書き込み無効

	// 6. シェーダーをコンパイル（DirectXCommonと同じシェーダーを使用）
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler;

	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));

	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = DirectXCommon::CompileShader(
		L"Object3d.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = DirectXCommon::CompileShader(
		L"Object3d.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);

	// 7. PSOの作成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = postEffectRootSignature.Get();
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.BlendState = blendDesc;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc; // 深度テスト無効
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_UNKNOWN; // 深度バッファ使用しない

	hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&postEffectPSO));
	assert(SUCCEEDED(hr));
}