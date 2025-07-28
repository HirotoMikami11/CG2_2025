#include "PostProcessChain.h"
#include "Managers/ImGui/ImGuiManager.h"

void PostProcessChain::Initialize(DirectXCommon* dxCommon, uint32_t width, uint32_t height) {
	dxCommon_ = dxCommon;
	width_ = width;
	height_ = height;

	// 中間バッファ作成
	CreateIntermediateBuffers();
	CreateIntermediateSRVs();
	CreateIntermediateRTVs();

	// エフェクト描画用OffscreenTriangle初期化（Sprite置き換え）
	offscreenTriangle_ = std::make_unique<OffscreenTriangle>();
	offscreenTriangle_->Initialize(dxCommon_);

	isInitialized_ = true;
	Logger::Log(Logger::GetStream(), "PostProcessChain initialized with OffscreenTriangle!\n");
}

void PostProcessChain::Finalize() {
	// エフェクトの終了処理
	for (auto& effect : effects_) {
		if (effect) {
			effect->Finalize();
		}
	}
	effects_.clear();

	// OffscreenTriangleの削除（Sprite置き換え）
	if (offscreenTriangle_) {
		offscreenTriangle_->Finalize();
		offscreenTriangle_.reset();
	}

	// ディスクリプタの解放
	auto descriptorManager = dxCommon_->GetDescriptorManager();
	if (descriptorManager) {
		for (int i = 0; i < 2; ++i) {
			if (intermediateSRVHandles_[i].isValid) {
				descriptorManager->ReleaseSRV(intermediateSRVHandles_[i].index);
			}
			if (intermediateRTVHandles_[i].isValid) {
				descriptorManager->ReleaseRTV(intermediateRTVHandles_[i].index);
			}
		}
	}

	isInitialized_ = false;
	Logger::Log(Logger::GetStream(), "PostProcessChain finalized.\n");
}

void PostProcessChain::Update(float deltaTime) {
	if (!isInitialized_) {
		return;
	}

	// 各エフェクトの更新
	for (auto& effect : effects_) {
		if (effect && effect->IsEnabled()) {
			effect->Update(deltaTime);
		}
	}

	// OffscreenTriangleの更新は特に不要（状態を持たないため）
}

D3D12_GPU_DESCRIPTOR_HANDLE PostProcessChain::ApplyEffects(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV) {
	if (!isInitialized_ || effects_.empty() || !offscreenTriangle_) {
		return inputSRV;
	}

	// 有効なエフェクトがない場合は入力をそのまま返す
	bool hasActiveEffect = false;
	for (const auto& effect : effects_) {
		if (effect && effect->IsEnabled()) {
			hasActiveEffect = true;
			break;
		}
	}

	if (!hasActiveEffect) {
		return inputSRV;
	}

	auto commandList = dxCommon_->GetCommandList();
	D3D12_GPU_DESCRIPTOR_HANDLE currentInput = inputSRV;
	int bufferIndex = 0;

	// 各エフェクトを順番に適用
	for (const auto& effect : effects_) {
		if (!effect || !effect->IsEnabled()) {
			continue;
		}

		// 深度テクスチャが必要なエフェクトの場合は自動的にスキップ
		if (effect->RequiresDepthTexture()) {
			Logger::Log(Logger::GetStream(), std::format("Warning: {} requires depth texture. Use ApplyEffectsWithDepth instead.\n", effect->GetName()));
			continue;
		}

		// 出力バッファをレンダーターゲット状態に遷移
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = intermediateBuffers_[bufferIndex].Get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		commandList->ResourceBarrier(1, &barrier);

		// 出力先を決定
		D3D12_CPU_DESCRIPTOR_HANDLE outputRTV = intermediateRTVHandles_[bufferIndex].cpuHandle;

		// エフェクトを適用（通常版Apply、OffscreenTriangle使用）
		effect->Apply(currentInput, outputRTV, offscreenTriangle_.get());

		// 出力バッファをシェーダーリソース状態に戻す
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		commandList->ResourceBarrier(1, &barrier);

		// 次の入力として今回の出力を設定
		currentInput = intermediateSRVHandles_[bufferIndex].gpuHandle;
		bufferIndex = (bufferIndex + 1) % 2;
	}

	return currentInput;
}

D3D12_GPU_DESCRIPTOR_HANDLE PostProcessChain::ApplyEffectsWithDepth(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV, D3D12_GPU_DESCRIPTOR_HANDLE depthSRV) {
	if (!isInitialized_ || effects_.empty() || !offscreenTriangle_) {
		return inputSRV;
	}

	// 有効なエフェクトがない場合は入力をそのまま返す
	bool hasActiveEffect = false;
	for (const auto& effect : effects_) {
		if (effect && effect->IsEnabled()) {
			hasActiveEffect = true;
			break;
		}
	}

	if (!hasActiveEffect) {
		return inputSRV;
	}

	auto commandList = dxCommon_->GetCommandList();
	D3D12_GPU_DESCRIPTOR_HANDLE currentInput = inputSRV;
	int bufferIndex = 0;

	// 各エフェクトを順番に適用
	for (const auto& effect : effects_) {
		if (!effect || !effect->IsEnabled()) {
			continue;
		}

		// 出力バッファをレンダーターゲット状態に遷移
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = intermediateBuffers_[bufferIndex].Get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		commandList->ResourceBarrier(1, &barrier);

		// 出力先を決定
		D3D12_CPU_DESCRIPTOR_HANDLE outputRTV = intermediateRTVHandles_[bufferIndex].cpuHandle;

		// 深度テクスチャが必要なエフェクトかどうかで自動判定
		if (effect->RequiresDepthTexture()) {
			// 深度版のApplyを使用（OffscreenTriangle使用）
			effect->Apply(currentInput, depthSRV, outputRTV, offscreenTriangle_.get());
		} else {
			// 通常版のApplyを使用（OffscreenTriangle使用）
			effect->Apply(currentInput, outputRTV, offscreenTriangle_.get());
		}

		// 出力バッファをシェーダーリソース状態に戻す
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		commandList->ResourceBarrier(1, &barrier);

		// 次の入力として今回の出力を設定
		currentInput = intermediateSRVHandles_[bufferIndex].gpuHandle;
		bufferIndex = (bufferIndex + 1) % 2;
	}

	return currentInput;
}

void PostProcessChain::CreateIntermediateBuffers() {
	// 中間バッファのリソース設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width_;
	resourceDesc.Height = height_;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	// ヒープ設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	// クリアカラー設定
	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	clearValue.Color[0] = 0.0f;
	clearValue.Color[1] = 0.0f;
	clearValue.Color[2] = 0.0f;
	clearValue.Color[3] = 1.0f;

	// 2つの中間バッファを作成
	for (int i = 0; i < 2; ++i) {
		HRESULT hr = dxCommon_->GetDevice()->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			&clearValue,
			IID_PPV_ARGS(&intermediateBuffers_[i]));

		assert(SUCCEEDED(hr));
	}

	Logger::Log(Logger::GetStream(), "Created intermediate buffers for PostProcessChain.\n");
}

void PostProcessChain::CreateIntermediateSRVs() {
	auto descriptorManager = dxCommon_->GetDescriptorManager();

	for (int i = 0; i < 2; ++i) {
		// SRVを割り当て
		intermediateSRVHandles_[i] = descriptorManager->AllocateSRV();
		assert(intermediateSRVHandles_[i].isValid);

		// SRV作成
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		dxCommon_->GetDevice()->CreateShaderResourceView(
			intermediateBuffers_[i].Get(),
			&srvDesc,
			intermediateSRVHandles_[i].cpuHandle);
	}

	Logger::Log(Logger::GetStream(), "Created intermediate SRVs for PostProcessChain.\n");
}

void PostProcessChain::CreateIntermediateRTVs() {
	auto descriptorManager = dxCommon_->GetDescriptorManager();

	for (int i = 0; i < 2; ++i) {
		// RTVを割り当て
		intermediateRTVHandles_[i] = descriptorManager->AllocateRTV();
		assert(intermediateRTVHandles_[i].isValid);

		// RTV作成
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		dxCommon_->GetDevice()->CreateRenderTargetView(
			intermediateBuffers_[i].Get(),
			&rtvDesc,
			intermediateRTVHandles_[i].cpuHandle);
	}

	Logger::Log(Logger::GetStream(), "Created intermediate RTVs for PostProcessChain.\n");
}

size_t PostProcessChain::GetActiveEffectCount() const {
	size_t count = 0;
	for (const auto& effect : effects_) {
		if (effect && effect->IsEnabled()) {
			count++;
		}
	}
	return count;
}

size_t PostProcessChain::GetDepthRequiredEffectCount() const {
	size_t count = 0;
	for (const auto& effect : effects_) {
		if (effect && effect->IsEnabled() && effect->RequiresDepthTexture()) {
			count++;
		}
	}
	return count;
}

void PostProcessChain::ImGui() {
#ifdef _DEBUG
	if (ImGui::TreeNode("Post Process Chain")) {
		ImGui::Text("Effects Count: %zu", effects_.size());
		ImGui::Text("Chain Size: %dx%d", width_, height_);

		// 有効なエフェクト数をカウント
		size_t activeCount = GetActiveEffectCount();
		size_t depthRequiredCount = GetDepthRequiredEffectCount();
		ImGui::Text("Active Effects: %zu", activeCount);
		ImGui::Text("Depth Required Effects: %zu", depthRequiredCount);

		// OffscreenTriangle情報
		ImGui::Separator();
		ImGui::Text("Render Method: OffscreenTriangle");
		ImGui::Text("Triangle Valid: %s", offscreenTriangle_ && offscreenTriangle_->IsValid() ? "YES" : "NO");



		ImGui::Separator();

		// 各エフェクトのImGui
		for (size_t i = 0; i < effects_.size(); ++i) {
			ImGui::PushID(static_cast<int>(i));

			// エフェクト名とインデックス、深度要求の表示
			std::string effectInfo = std::format("[{}] {}", i, effects_[i]->GetName());
			if (effects_[i]->RequiresDepthTexture()) {
				effectInfo += " [DEPTH]";
			}
			ImGui::Text("%s", effectInfo.c_str());
			ImGui::SameLine();

			// 有効/無効チェックボックス
			bool enabled = effects_[i]->IsEnabled();
			if (ImGui::Checkbox("##enabled", &enabled)) {
				effects_[i]->SetEnabled(enabled);
			}

			// 順序変更ボタン
			if (i > 0) {
				ImGui::SameLine();
				if (ImGui::ArrowButton("##up", ImGuiDir_Up)) {
					MoveEffect(i, i - 1);
				}
			}
			if (i < effects_.size() - 1) {
				ImGui::SameLine();
				if (ImGui::ArrowButton("##down", ImGuiDir_Down)) {
					MoveEffect(i, i + 1);
				}
			}

			// エフェクト固有のImGui
			effects_[i]->ImGui();

			ImGui::PopID();
			ImGui::Separator();
		}
		ImGui::TreePop();
	}
#endif
}

void PostProcessChain::MoveEffect(size_t from, size_t to) {
	if (from >= effects_.size() || to >= effects_.size() || from == to) {
		return;
	}

	if (from < to) {
		std::rotate(effects_.begin() + from, effects_.begin() + from + 1, effects_.begin() + to + 1);
	} else {
		std::rotate(effects_.begin() + to, effects_.begin() + from, effects_.begin() + from + 1);
	}
}