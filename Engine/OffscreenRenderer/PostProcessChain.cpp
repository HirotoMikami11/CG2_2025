#include "PostProcessChain.h"
#include "Managers/ImGui/ImGuiManager.h" 

void PostProcessChain::Initialize(DirectXCommon* dxCommon, uint32_t width, uint32_t height) {
	dxCommon_ = dxCommon;
	width_ = width;
	height_ = height;

	// ping-pong用のレンダーターゲットを作成
	CreatePingPongTargets();

	// フルスクリーン描画用スプライトを作成
	CreateFullscreenSprite();

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

	Logger::Log(Logger::GetStream(), "PostProcessChain initialized successfully (Final Fix)!\n");
}

void PostProcessChain::Finalize() {
	// エフェクトの終了処理
	for (auto& effect : effects_) {
		effect->Finalize();
	}
	effects_.clear();

	// フルスクリーンスプライトの削除
	fullscreenSprite_.reset();

	// DescriptorHeapManagerからディスクリプタを解放
	auto descriptorManager = dxCommon_->GetDescriptorManager();
	if (descriptorManager) {
		for (int i = 0; i < 2; ++i) {
			if (pingPongRTVs_[i].isValid) {
				descriptorManager->ReleaseRTV(pingPongRTVs_[i].index);
			}
			if (pingPongSRVs_[i].isValid) {
				descriptorManager->ReleaseSRV(pingPongSRVs_[i].index);
			}
		}
	}

	Logger::Log(Logger::GetStream(), "PostProcessChain finalized (Final Fix).\n");
}

void PostProcessChain::Update(float deltaTime) {
	// エフェクトの更新
	for (auto& effect : effects_) {
		effect->Update(deltaTime);
	}

	// フルスクリーンスプライトの更新
	if (fullscreenSprite_) {
		Matrix4x4 spriteViewProjection = MakeViewProjectionMatrixSprite();
		fullscreenSprite_->Update(spriteViewProjection);
	}
}
D3D12_GPU_DESCRIPTOR_HANDLE PostProcessChain::ApplyEffects(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV) {
	auto commandList = dxCommon_->GetCommandList();

	// 有効なエフェクトのみを収集
	std::vector<PostEffect*> activeEffects;
	for (auto& effect : effects_) {
		if (effect->IsEnabled()) {
			activeEffects.push_back(effect.get());
		}
	}

	// エフェクトが一つもない場合は入力をそのまま返す
	if (activeEffects.empty()) {
		return inputSRV;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE currentInput = inputSRV;

	// 各エフェクトを順次適用
	for (size_t i = 0; i < activeEffects.size(); ++i) {
		int targetIndex = i % 2;

		// 毎回ping-pongテクスチャをRENDER_TARGET状態に遷移
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = pingPongTextures_[targetIndex].Get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		// バリア実行
		commandList->ResourceBarrier(1, &barrier);

		// エフェクトを適用
		activeEffects[i]->Apply(currentInput, pingPongRTVs_[targetIndex].cpuHandle, fullscreenSprite_.get());

		// レンダーターゲットをシェーダーリソース状態に遷移
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		commandList->ResourceBarrier(1, &barrier);

		// 次のエフェクトの入力として設定
		currentInput = pingPongSRVs_[targetIndex].gpuHandle;
	}

	return currentInput;
}

void PostProcessChain::CreatePingPongTargets() {
	auto descriptorManager = dxCommon_->GetDescriptorManager();

	for (int i = 0; i < 2; ++i) {
		// テクスチャリソース作成
		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Width = width_;
		resourceDesc.Height = height_;
		resourceDesc.MipLevels = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		D3D12_HEAP_PROPERTIES heapProperties{};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

		D3D12_CLEAR_VALUE clearValue{};
		clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		clearValue.Color[0] = 0.0f;
		clearValue.Color[1] = 0.0f;
		clearValue.Color[2] = 0.0f;
		clearValue.Color[3] = 1.0f;

		// 初期状態をPIXEL_SHADER_RESOURCEに変更（バリアとの整合性のため）
		HRESULT hr = dxCommon_->GetDevice()->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			&clearValue,
			IID_PPV_ARGS(&pingPongTextures_[i]));
		assert(SUCCEEDED(hr));

		// RTV作成
		pingPongRTVs_[i] = descriptorManager->AllocateRTV();
		if (!pingPongRTVs_[i].isValid) {
			Logger::Log(Logger::GetStream(), std::format("Failed to allocate RTV for ping-pong texture {}\n", i));
			assert(false);
			return;
		}

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		dxCommon_->GetDevice()->CreateRenderTargetView(
			pingPongTextures_[i].Get(), &rtvDesc, pingPongRTVs_[i].cpuHandle);

		// SRV作成
		pingPongSRVs_[i] = descriptorManager->AllocateSRV();
		if (!pingPongSRVs_[i].isValid) {
			Logger::Log(Logger::GetStream(), std::format("Failed to allocate SRV for ping-pong texture {}\n", i));
			assert(false);
			return;
		}

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		dxCommon_->GetDevice()->CreateShaderResourceView(
			pingPongTextures_[i].Get(), &srvDesc, pingPongSRVs_[i].cpuHandle);

		Logger::Log(Logger::GetStream(), std::format("Created ping-pong texture {} successfully!\n", i));
	}

	Logger::Log(Logger::GetStream(), "PingPong render targets created successfully (PIXEL_SHADER_RESOURCE initial state)!\n");
}

void PostProcessChain::CreateFullscreenSprite() {
	// フルスクリーン描画用スプライトを作成
	fullscreenSprite_ = std::make_unique<Sprite>();

	// 画面全体をカバーするサイズと位置で初期化
	Vector2 center = { static_cast<float>(width_) * 0.5f, static_cast<float>(height_) * 0.5f };
	Vector2 size = { static_cast<float>(width_), static_cast<float>(height_) };

	// 空のテクスチャ名で初期化（エフェクト適用時にテクスチャハンドルを直接指定するため）
	fullscreenSprite_->Initialize(dxCommon_, "", center, size);

	Logger::Log(Logger::GetStream(), "Fullscreen sprite created successfully!\n");
}

void PostProcessChain::ImGui() {
#ifdef _DEBUG
	if (ImGui::CollapsingHeader("Post Process Chain (Final Fix)")) {
		ImGui::Text("Effects Count: %zu", effects_.size());
		ImGui::Text("Chain Size: %dx%d", width_, height_);

		// 有効なエフェクト数をカウント
		int activeCount = 0;
		for (auto& effect : effects_) {
			if (effect->IsEnabled()) activeCount++;
		}
		ImGui::Text("Active Effects: %d", activeCount);

		ImGui::Separator();

		// 各エフェクトのImGui
		for (size_t i = 0; i < effects_.size(); ++i) {
			ImGui::PushID(static_cast<int>(i));

			// エフェクト名とインデックス
			ImGui::Text("[%zu] %s", i, effects_[i]->GetName().c_str());
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

		// フルスクリーンスプライト情報
		if (fullscreenSprite_) {
			ImGui::Separator();
			ImGui::Text("Fullscreen Sprite Info:");
			Vector2 pos = fullscreenSprite_->GetPosition();
			Vector2 size = fullscreenSprite_->GetSize();
			ImGui::Text("Position: (%.1f, %.1f)", pos.x, pos.y);
			ImGui::Text("Size: (%.1f, %.1f)", size.x, size.y);
		}
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