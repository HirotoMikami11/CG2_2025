#include "LayeredPostProcessChain.h"
#include "Managers/ImGui/ImGuiManager.h"
#include <algorithm>

void LayeredPostProcessChain::Initialize(DirectXCommon* dxCommon, uint32_t width, uint32_t height) {
	dxCommon_ = dxCommon;
	width_ = width;
	height_ = height;

	// 中間バッファ作成
	CreateIntermediateBuffers();
	CreateIntermediateSRVs();
	CreateIntermediateRTVs();

	// エフェクト描画用Sprite初期化
	effectSprite_ = std::make_unique<Sprite>();
	Vector2 center = { static_cast<float>(width_) * 0.5f, static_cast<float>(height_) * 0.5f };
	Vector2 size = { static_cast<float>(width_), static_cast<float>(height_) };
	effectSprite_->Initialize(dxCommon_, "", center, size);

	isInitialized_ = true;
	Logger::Log(Logger::GetStream(), "LayeredPostProcessChain initialized with dynamic layer management!\n");
}

void LayeredPostProcessChain::Finalize() {
	// エフェクトの終了処理
	for (auto& effectInfo : effects_) {
		if (effectInfo.effect) {
			effectInfo.effect->Finalize();
		}
	}
	effects_.clear();

	// Spriteの削除
	effectSprite_.reset();

	// ディスクリプタの解放
	auto descriptorManager = dxCommon_->GetDescriptorManager();
	if (descriptorManager) {
		for (int i = 0; i < kIntermediateBufferCount; ++i) {
			if (intermediateSRVHandles_[i].isValid) {
				descriptorManager->ReleaseSRV(intermediateSRVHandles_[i].index);
			}
			if (intermediateRTVHandles_[i].isValid) {
				descriptorManager->ReleaseRTV(intermediateRTVHandles_[i].index);
			}
		}
	}

	isInitialized_ = false;
	Logger::Log(Logger::GetStream(), "LayeredPostProcessChain finalized.\n");
}

void LayeredPostProcessChain::Update(float deltaTime) {
	if (!isInitialized_) {
		return;
	}

	// 各エフェクトの更新
	for (auto& effectInfo : effects_) {
		if (effectInfo.effect && effectInfo.enabled) {
			effectInfo.effect->Update(deltaTime);
		}
	}

	// エフェクト描画用Spriteの更新
	if (effectSprite_) {
		Matrix4x4 spriteViewProjection = MakeViewProjectionMatrixSprite();
		effectSprite_->Update(spriteViewProjection);
	}
}

D3D12_GPU_DESCRIPTOR_HANDLE LayeredPostProcessChain::ApplyLayeredEffects(
	D3D12_GPU_DESCRIPTOR_HANDLE world3DSRV,
	D3D12_GPU_DESCRIPTOR_HANDLE uiSpriteSRV,
	D3D12_GPU_DESCRIPTOR_HANDLE depthSRV) {

	if (!isInitialized_) {
		return world3DSRV;
	}

	// 1. WORLD_3Dレイヤーのエフェクトを3Dテクスチャに適用
	D3D12_GPU_DESCRIPTOR_HANDLE processed3D = ApplyLayerEffects(PostProcessLayer::WORLD_3D, world3DSRV, depthSRV);

	// 2. UI_SPRITEレイヤーのエフェクトをUIテクスチャに適用（深度なし）
	D3D12_GPU_DESCRIPTOR_HANDLE processedUI = ApplyLayerEffects(PostProcessLayer::UI_SPRITE, uiSpriteSRV);

	// 3. 3DとUIを合成
	D3D12_GPU_DESCRIPTOR_HANDLE composited = CompositeTextures(processed3D, processedUI);

	// 4. GLOBALレイヤーのエフェクトを合成結果に適用
	D3D12_GPU_DESCRIPTOR_HANDLE final = ApplyLayerEffects(PostProcessLayer::GLOBAL, composited, depthSRV);

	return final;
}

D3D12_GPU_DESCRIPTOR_HANDLE LayeredPostProcessChain::ApplyEffectsWithDepth(
	D3D12_GPU_DESCRIPTOR_HANDLE inputSRV,
	D3D12_GPU_DESCRIPTOR_HANDLE depthSRV) {

	if (!isInitialized_) {
		return inputSRV;
	}

	// 簡易版：全レイヤーのエフェクトを順番に適用
	D3D12_GPU_DESCRIPTOR_HANDLE currentInput = inputSRV;

	// WORLD_3D → UI_SPRITE → GLOBAL の順序で適用
	for (int layerIndex = 0; layerIndex < 3; ++layerIndex) {
		PostProcessLayer layer = static_cast<PostProcessLayer>(layerIndex);
		currentInput = ApplyLayerEffects(layer, currentInput, depthSRV);
	}

	return currentInput;
}

D3D12_GPU_DESCRIPTOR_HANDLE LayeredPostProcessChain::ApplyLayerEffects(
	PostProcessLayer layer,
	D3D12_GPU_DESCRIPTOR_HANDLE inputSRV,
	D3D12_GPU_DESCRIPTOR_HANDLE depthSRV) {

	auto commandList = dxCommon_->GetCommandList();
	D3D12_GPU_DESCRIPTOR_HANDLE currentInput = inputSRV;
	int bufferIndex = 0;

	// 指定レイヤーの有効なエフェクトを取得
	std::vector<LayeredEffectInfo*> layerEffects;
	for (auto& effectInfo : effects_) {
		if (effectInfo.layer == layer && effectInfo.enabled && effectInfo.effect) {
			layerEffects.push_back(&effectInfo);
		}
	}

	// 優先度でソート
	std::sort(layerEffects.begin(), layerEffects.end(),
		[](const LayeredEffectInfo* a, const LayeredEffectInfo* b) {
			return a->priority < b->priority;
		});

	// エフェクトが何もない場合は入力をそのまま返す
	if (layerEffects.empty()) {
		return inputSRV;
	}

	// 各エフェクトを順番に適用
	for (const auto& effectInfo : layerEffects) {
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

		// エフェクトを適用
		// UI_SPRITEレイヤーでは深度ベースエフェクトを自動スキップ
		if (layer == PostProcessLayer::UI_SPRITE && effectInfo->effect->RequiresDepthTexture()) {
			Logger::Log(Logger::GetStream(), std::format("Skipping depth-based effect '{}' in UI_SPRITE layer.\n",
				effectInfo->effect->GetName()));

			// 深度ベースエフェクトはスキップ（入力をそのまま出力にコピー）
			// TODO: 実際にはコピー処理が必要だが、簡略化のため次のバッファに進む
		} else {
			// エフェクトを適用
			if (effectInfo->effect->RequiresDepthTexture() && depthSRV.ptr != 0) {
				// 深度版のApplyを使用
				effectInfo->effect->Apply(currentInput, depthSRV, outputRTV, effectSprite_.get());
			} else {
				// 通常版のApplyを使用
				effectInfo->effect->Apply(currentInput, outputRTV, effectSprite_.get());
			}
		}

		// 出力バッファをシェーダーリソース状態に戻す
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		commandList->ResourceBarrier(1, &barrier);

		// 次の入力として今回の出力を設定
		currentInput = intermediateSRVHandles_[bufferIndex].gpuHandle;
		bufferIndex = (bufferIndex + 1) % kIntermediateBufferCount;
	}

	return currentInput;
}

D3D12_GPU_DESCRIPTOR_HANDLE LayeredPostProcessChain::CompositeTextures(
	D3D12_GPU_DESCRIPTOR_HANDLE baseSRV,
	D3D12_GPU_DESCRIPTOR_HANDLE overlaySRV) {

	// 簡易実装：現在はベーステクスチャをそのまま返す
	// TODO: 実際の合成処理を実装（アルファブレンドなど）
	return baseSRV;
}

void LayeredPostProcessChain::CreateIntermediateBuffers() {
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

	// 複数の中間バッファを作成
	for (int i = 0; i < kIntermediateBufferCount; ++i) {
		HRESULT hr = dxCommon_->GetDevice()->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			&clearValue,
			IID_PPV_ARGS(&intermediateBuffers_[i]));

		assert(SUCCEEDED(hr));
	}

	Logger::Log(Logger::GetStream(), "Created layered intermediate buffers for LayeredPostProcessChain.\n");
}

void LayeredPostProcessChain::CreateIntermediateSRVs() {
	auto descriptorManager = dxCommon_->GetDescriptorManager();

	for (int i = 0; i < kIntermediateBufferCount; ++i) {
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

	Logger::Log(Logger::GetStream(), "Created layered intermediate SRVs for LayeredPostProcessChain.\n");
}

void LayeredPostProcessChain::CreateIntermediateRTVs() {
	auto descriptorManager = dxCommon_->GetDescriptorManager();

	for (int i = 0; i < kIntermediateBufferCount; ++i) {
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

	Logger::Log(Logger::GetStream(), "Created layered intermediate RTVs for LayeredPostProcessChain.\n");
}

void LayeredPostProcessChain::SortEffectsInLayer(PostProcessLayer layer) {
	// 指定レイヤーのエフェクトのみをソート
	// 現在は優先度順で自動ソートされるため、特別な処理は不要
	// 必要に応じて実装
}

void LayeredPostProcessChain::MoveEffectToLayer(size_t effectIndex, PostProcessLayer newLayer) {
	if (effectIndex >= effects_.size()) {
		return;
	}

	effects_[effectIndex].layer = newLayer;
	Logger::Log(Logger::GetStream(), std::format("Moved effect '{}' to layer '{}'\n",
		effects_[effectIndex].effect->GetName(), GetLayerName(newLayer)));
}

void LayeredPostProcessChain::MoveEffectInLayer(size_t effectIndex, int direction) {
	if (effectIndex >= effects_.size()) {
		return;
	}

	LayeredEffectInfo& effect = effects_[effectIndex];
	PostProcessLayer layer = effect.layer;

	// 同じレイヤーのエフェクトを探す
	std::vector<size_t> sameLayerIndices;
	for (size_t i = 0; i < effects_.size(); ++i) {
		if (effects_[i].layer == layer) {
			sameLayerIndices.push_back(i);
		}
	}

	// 現在のエフェクトのレイヤー内位置を特定
	auto it = std::find(sameLayerIndices.begin(), sameLayerIndices.end(), effectIndex);
	if (it == sameLayerIndices.end()) {
		return;
	}

	size_t currentPos = std::distance(sameLayerIndices.begin(), it);
	size_t newPos = currentPos;

	if (direction > 0 && currentPos < sameLayerIndices.size() - 1) {
		// 下に移動
		newPos = currentPos + 1;
	} else if (direction < 0 && currentPos > 0) {
		// 上に移動
		newPos = currentPos - 1;
	} else {
		return; // 移動できない
	}

	// 優先度を交換
	size_t targetIndex = sameLayerIndices[newPos];
	int tempPriority = effects_[effectIndex].priority;
	effects_[effectIndex].priority = effects_[targetIndex].priority;
	effects_[targetIndex].priority = tempPriority;

	Logger::Log(Logger::GetStream(), std::format("Moved effect '{}' {} in layer '{}'\n",
		effect.effect->GetName(), (direction > 0) ? "down" : "up", GetLayerName(layer)));
}

size_t LayeredPostProcessChain::GetActiveEffectCount(PostProcessLayer layer) const {
	size_t count = 0;
	for (const auto& effectInfo : effects_) {
		if (effectInfo.layer == layer && effectInfo.enabled && effectInfo.effect) {
			count++;
		}
	}
	return count;
}

size_t LayeredPostProcessChain::GetDepthRequiredEffectCount(PostProcessLayer layer) const {
	size_t count = 0;
	for (const auto& effectInfo : effects_) {
		if (effectInfo.layer == layer && effectInfo.enabled &&
			effectInfo.effect && effectInfo.effect->RequiresDepthTexture()) {
			count++;
		}
	}
	return count;
}

const char* LayeredPostProcessChain::GetLayerName(PostProcessLayer layer) {
	switch (layer) {
	case PostProcessLayer::WORLD_3D: return "3D World";
	case PostProcessLayer::UI_SPRITE: return "UI Sprite";
	case PostProcessLayer::GLOBAL: return "Global";
	default: return "Unknown";
	}
}

void LayeredPostProcessChain::ImGui() {
#ifdef _DEBUG
	if (ImGui::CollapsingHeader("Layered Post Process Chain (Dynamic System)")) {
		ImGui::Text("Effects Count: %zu", effects_.size());
		ImGui::Text("Chain Size: %dx%d", width_, height_);

		// レイヤー別統計表示
		ImGui::Separator();
		ImGui::Text("Layer Statistics:");
		for (int i = 0; i < 3; ++i) {
			PostProcessLayer layer = static_cast<PostProcessLayer>(i);
			size_t activeCount = GetActiveEffectCount(layer);
			size_t depthCount = GetDepthRequiredEffectCount(layer);
			ImGui::Text("  %s: %zu active (%zu depth-based)", GetLayerName(layer), activeCount, depthCount);
		}

		ImGui::Separator();

		// レイヤータブ
		if (ImGui::BeginTabBar("LayerTabs")) {
			for (int layerIndex = 0; layerIndex < 3; ++layerIndex) {
				PostProcessLayer layer = static_cast<PostProcessLayer>(layerIndex);
				const char* layerName = GetLayerName(layer);

				if (ImGui::BeginTabItem(layerName)) {
					selectedLayer_ = layerIndex;
					DrawLayerImGui(layer);
					ImGui::EndTabItem();
				}
			}
			ImGui::EndTabBar();
		}

		// 全体制御
		ImGui::Separator();
		if (ImGui::CollapsingHeader("Global Controls")) {
			ImGui::Text("Quick Actions:");
			if (ImGui::Button("Enable All Effects")) {
				for (auto& effectInfo : effects_) {
					effectInfo.enabled = true;
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Disable All Effects")) {
				for (auto& effectInfo : effects_) {
					effectInfo.enabled = false;
				}
			}
		}
	}
#endif
}

void LayeredPostProcessChain::DrawLayerImGui(PostProcessLayer layer) {
#ifdef _DEBUG
	const char* layerName = GetLayerName(layer);

	// レイヤー説明
	switch (layer) {
	case PostProcessLayer::WORLD_3D:
		ImGui::TextWrapped("3D World Layer: Effects applied to 3D objects. Depth-based effects (fog, DOF) work here.");
		break;
	case PostProcessLayer::UI_SPRITE:
		ImGui::TextWrapped("UI Sprite Layer: Effects applied to UI elements. Depth-based effects are automatically skipped.");
		break;
	case PostProcessLayer::GLOBAL:
		ImGui::TextWrapped("Global Layer: Effects applied to the final composed image (3D + UI).");
		break;
	}

	ImGui::Separator();

	// レイヤー内のエフェクト一覧
	bool hasEffectsInLayer = false;
	for (size_t i = 0; i < effects_.size(); ++i) {
		if (effects_[i].layer != layer) {
			continue;
		}

		hasEffectsInLayer = true;
		ImGui::PushID(static_cast<int>(i));

		// エフェクト情報表示
		std::string effectInfo = std::format("[{}] {}", effects_[i].priority, effects_[i].effect->GetName());
		if (effects_[i].effect->RequiresDepthTexture()) {
			effectInfo += " [DEPTH]";
			if (layer == PostProcessLayer::UI_SPRITE) {
				effectInfo += " [SKIPPED]";
			}
		}
		ImGui::Text("%s", effectInfo.c_str());

		// 有効/無効チェックボックス
		ImGui::SameLine();
		if (ImGui::Checkbox("##enabled", &effects_[i].enabled)) {
			Logger::Log(Logger::GetStream(), std::format("Effect '{}' {} in layer '{}'\n",
				effects_[i].effect->GetName(), effects_[i].enabled ? "enabled" : "disabled", layerName));
		}

		// 順序変更ボタン
		ImGui::SameLine();
		if (ImGui::ArrowButton("##up", ImGuiDir_Up)) {
			MoveEffectInLayer(i, -1);
		}
		ImGui::SameLine();
		if (ImGui::ArrowButton("##down", ImGuiDir_Down)) {
			MoveEffectInLayer(i, 1);
		}

		// レイヤー移動ボタン
		ImGui::SameLine();
		if (ImGui::BeginCombo("##moveLayer", "Move To...")) {
			for (int targetLayer = 0; targetLayer < 3; ++targetLayer) {
				if (targetLayer == static_cast<int>(layer)) continue;

				PostProcessLayer target = static_cast<PostProcessLayer>(targetLayer);
				if (ImGui::Selectable(GetLayerName(target))) {
					MoveEffectToLayer(i, target);
				}
			}
			ImGui::EndCombo();
		}

		// エフェクト固有のImGui
		effects_[i].effect->ImGui();

		ImGui::PopID();
		ImGui::Separator();
	}

	if (!hasEffectsInLayer) {
		ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No effects in this layer.");
	}
#endif
}