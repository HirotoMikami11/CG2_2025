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

	// オフスクリーン描画用Spriteを初期化
	InitializeOffscreenSprite();

	// ポストプロセスチェーンを初期化
	postProcessChain_ = std::make_unique<PostProcessChain>();
	postProcessChain_->Initialize(dxCommon_, width_, height_);


	///ここにエフェクトを追加していく
	// グリッチエフェクトを追加
	RGBShiftEffect_ = postProcessChain_->AddEffect<RGBShiftPostEffect>();
	// グレースケールエフェクトを追加
	grayscaleEffect_ = postProcessChain_->AddEffect<GrayscalePostEffect>();



	// 初期化完了のログを出す
	Logger::Log(Logger::GetStream(), "Complete OffscreenRenderer initialized (PostProcess Chain)!!\n");
}

void OffscreenRenderer::Finalize() {
	// ポストプロセスチェーンの終了処理
	if (postProcessChain_) {
		postProcessChain_->Finalize();
		postProcessChain_.reset();
	}
	RGBShiftEffect_ = nullptr;
	grayscaleEffect_ = nullptr;
	// オフスクリーンSprite削除
	offscreenSprite_.reset();

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

	Logger::Log(Logger::GetStream(), "OffscreenRenderer finalized (PostProcess Chain).\n");
}

void OffscreenRenderer::Update(float deltaTime) {
	// ポストプロセスチェーンの更新
	if (postProcessChain_) {
		postProcessChain_->Update(deltaTime);
	}

	// オフスクリーンSpriteの更新
	if (offscreenSprite_) {
		// スプライト用のビュープロジェクション行列で更新
		Matrix4x4 spriteViewProjection = MakeViewProjectionMatrixSprite();
		offscreenSprite_->Update(spriteViewProjection);
	}
}void OffscreenRenderer::PreDraw() {
	auto commandList = dxCommon_->GetCommandList();

	// バリア構造体を毎回新しく作成（メンバ変数の使い回しを避ける）
	D3D12_RESOURCE_BARRIER preDrawBarrier{};
	preDrawBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	preDrawBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	preDrawBarrier.Transition.pResource = renderTargetTexture_.Get();
	preDrawBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	preDrawBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	preDrawBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	// バリア実行
	commandList->ResourceBarrier(1, &preDrawBarrier);

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

	// バリア構造体を毎回新しく作成（メンバ変数の使い回しを避ける）
	D3D12_RESOURCE_BARRIER postDrawBarrier{};
	postDrawBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	postDrawBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	postDrawBarrier.Transition.pResource = renderTargetTexture_.Get();
	postDrawBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	postDrawBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	postDrawBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	// バリア実行
	commandList->ResourceBarrier(1, &postDrawBarrier);
}

void OffscreenRenderer::DrawOffscreenTexture(float x, float y, float width, float height) {
	if (!offscreenSprite_) {
		return;
	}

	auto commandList = dxCommon_->GetCommandList();

	// 引数からSpriteの位置とサイズを設定
	Vector2 position = { x + width * 0.5f, y + height * 0.5f };  // 中心座標
	Vector2 size = { width, height };

	offscreenSprite_->SetPosition(position);
	offscreenSprite_->SetSize(size);

	// ポストプロセスチェーンを適用
	D3D12_GPU_DESCRIPTOR_HANDLE finalTexture = srvHandle_.gpuHandle;
	if (postProcessChain_) {
		finalTexture = postProcessChain_->ApplyEffects(srvHandle_.gpuHandle);
	}

	// PostProcessChain実行後に描画状態を完全にリセット
	// バックバッファのレンダーターゲットを再設定
	UINT backBufferIndex = dxCommon_->GetSwapChain()->GetCurrentBackBufferIndex();
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = dxCommon_->GetRTVHandle(backBufferIndex);
	commandList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

	// ディスクリプタヒープを再設定
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeaps[] = {
		dxCommon_->GetDescriptorManager()->GetSRVHeapComPtr()
	};
	commandList->SetDescriptorHeaps(1, descriptorHeaps->GetAddressOf());

	// 最終結果を描画
	offscreenSprite_->DrawWithCustomPSO(
		dxCommon_->GetSpriteRootSignature(),
		dxCommon_->GetSpritePipelineState(),
		finalTexture
	);

	// スプライト用PSOから通常描画用PSOに戻す
	commandList->SetGraphicsRootSignature(dxCommon_->GetRootSignature());
	commandList->SetPipelineState(dxCommon_->GetPipelineState());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


void OffscreenRenderer::InitializeOffscreenSprite() {
	// オフスクリーン描画用Spriteを作成
	offscreenSprite_ = std::make_unique<Sprite>();

	// フルスクリーンサイズで初期化
	Vector2 center = { static_cast<float>(width_) * 0.5f, static_cast<float>(height_) * 0.5f };
	Vector2 size = { static_cast<float>(width_), static_cast<float>(height_) };

	// 空のテクスチャ名で初期化（DrawWithCustomPSOでテクスチャハンドルを直接指定するため）
	offscreenSprite_->Initialize(dxCommon_, "", center, size);

	Logger::Log(Logger::GetStream(), "Complete initialize offscreen sprite!!\n");
}
void OffscreenRenderer::CreateRenderTargetTexture() {
	// リソース設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(width_);
	resourceDesc.Height = UINT(height_);
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

	// 初期状態をPIXEL_SHADER_RESOURCEに変更（バリアとの整合性のため）
	HRESULT hr = dxCommon_->GetDevice()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&clearValue,
		IID_PPV_ARGS(&renderTargetTexture_));

	assert(SUCCEEDED(hr));
	Logger::Log(Logger::GetStream(), "Complete create offscreen render target texture (PIXEL_SHADER_RESOURCE initial state)!!\n");
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

void OffscreenRenderer::ImGui() {

#ifdef _DEBUG
	if (ImGui::CollapsingHeader("Offscreen Renderer (PostProcess Chain)")) {
		// オフスクリーンのサイズ
		ImGui::Text("Render Target Size: %dx%d", width_, height_);
		ImGui::Text("Is Valid: %s", IsValid() ? "Yes" : "No");

		// レンダーターゲットのハンドル情報
		if (srvHandle_.isValid) {
			ImGui::Text("SRV Index: %d", srvHandle_.index);
		}

		// オフスクリーンSprite情報
		if (offscreenSprite_) {
			ImGui::Separator();
			ImGui::Text("Offscreen Sprite Info:");
			Vector2 pos = offscreenSprite_->GetPosition();
			Vector2 size = offscreenSprite_->GetSize();
			ImGui::Text("Position: (%.1f, %.1f)", pos.x, pos.y);
			ImGui::Text("Size: (%.1f, %.1f)", size.x, size.y);
			ImGui::Text("Texture: %s", offscreenSprite_->GetTextureName().c_str());
			ImGui::Text("Visible: %s", offscreenSprite_->IsVisible() ? "Yes" : "No");
			ImGui::Text("Active: %s", offscreenSprite_->IsActive() ? "Yes" : "No");
		}

		// ポストプロセスチェーンのImGui
		if (postProcessChain_) {
			ImGui::Separator();
			postProcessChain_->ImGui();
		}
	}
#endif

}
