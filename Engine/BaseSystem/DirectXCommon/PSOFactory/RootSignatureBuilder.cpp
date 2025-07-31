#include "RootSignatureBuilder.h"
#include <format>

RootSignatureBuilder& RootSignatureBuilder::AddCBV(uint32_t shaderRegister,
	D3D12_SHADER_VISIBILITY visibility) {
	D3D12_ROOT_PARAMETER param{};
	param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	param.Descriptor.ShaderRegister = shaderRegister;
	param.Descriptor.RegisterSpace = 0;  // デフォルトのレジスタスペース
	param.ShaderVisibility = visibility;

	rootParameters_.push_back(param);

	Logger::Log(Logger::GetStream(),
		std::format("RootSignatureBuilder: Added CBV (b{}) at parameter index {}\n",
			shaderRegister, rootParameters_.size() - 1));

	return *this;
}

RootSignatureBuilder& RootSignatureBuilder::AddSRV(uint32_t baseShaderRegister,
	uint32_t count,
	D3D12_SHADER_VISIBILITY visibility) {
	// DescriptorRangeを作成
	auto rangeHolder = std::make_unique<DescriptorRangeHolder>();

	D3D12_DESCRIPTOR_RANGE range{};
	range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range.NumDescriptors = count;
	range.BaseShaderRegister = baseShaderRegister;
	range.RegisterSpace = 0;
	range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rangeHolder->ranges.push_back(range);

	// DescriptorTableを作成
	D3D12_ROOT_PARAMETER param{};
	param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	param.DescriptorTable.NumDescriptorRanges = static_cast<UINT>(rangeHolder->ranges.size());
	param.DescriptorTable.pDescriptorRanges = rangeHolder->ranges.data();
	param.ShaderVisibility = visibility;

	rootParameters_.push_back(param);
	descriptorRangeHolders_.push_back(std::move(rangeHolder));

	// ポインタを再設定（vectorの再配置に対応）
	UpdateDescriptorTablePointers();

	Logger::Log(Logger::GetStream(),
		std::format("RootSignatureBuilder: Added SRV Table (t{}, count:{}) at parameter index {}\n",
			baseShaderRegister, count, rootParameters_.size() - 1));

	return *this;
}

RootSignatureBuilder& RootSignatureBuilder::AddUAV(uint32_t baseShaderRegister,
	uint32_t count,
	D3D12_SHADER_VISIBILITY visibility) {
	// DescriptorRangeを作成
	auto rangeHolder = std::make_unique<DescriptorRangeHolder>();

	D3D12_DESCRIPTOR_RANGE range{};
	range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	range.NumDescriptors = count;
	range.BaseShaderRegister = baseShaderRegister;
	range.RegisterSpace = 0;
	range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rangeHolder->ranges.push_back(range);

	// DescriptorTableを作成
	D3D12_ROOT_PARAMETER param{};
	param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	param.DescriptorTable.NumDescriptorRanges = static_cast<UINT>(rangeHolder->ranges.size());
	param.DescriptorTable.pDescriptorRanges = rangeHolder->ranges.data();
	param.ShaderVisibility = visibility;

	rootParameters_.push_back(param);
	descriptorRangeHolders_.push_back(std::move(rangeHolder));

	// ポインタを再設定
	UpdateDescriptorTablePointers();

	Logger::Log(Logger::GetStream(),
		std::format("RootSignatureBuilder: Added UAV Table (u{}, count:{}) at parameter index {}\n",
			baseShaderRegister, count, rootParameters_.size() - 1));

	return *this;
}

RootSignatureBuilder& RootSignatureBuilder::AddStaticSampler(uint32_t shaderRegister,
	D3D12_FILTER filter,
	D3D12_TEXTURE_ADDRESS_MODE addressMode,
	D3D12_SHADER_VISIBILITY visibility) {
	D3D12_STATIC_SAMPLER_DESC sampler{};
	sampler.Filter = filter;
	sampler.AddressU = addressMode;
	sampler.AddressV = addressMode;
	sampler.AddressW = addressMode;
	sampler.MipLODBias = 0.0f;
	sampler.MaxAnisotropy = 0;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = shaderRegister;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = visibility;

	staticSamplers_.push_back(sampler);

	Logger::Log(Logger::GetStream(),
		std::format("RootSignatureBuilder: Added Static Sampler (s{})\n", shaderRegister));

	return *this;
}

RootSignatureBuilder& RootSignatureBuilder::SetFlags(D3D12_ROOT_SIGNATURE_FLAGS flags) {
	flags_ = flags;
	return *this;
}

Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignatureBuilder::Build(ID3D12Device* device) {
	if (!device) {
		Logger::Log(Logger::GetStream(), "RootSignatureBuilder: Error - device is null\n");
		return nullptr;
	}

	// RootSignature設定を作成
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.NumParameters = static_cast<UINT>(rootParameters_.size());
	rootSignatureDesc.pParameters = rootParameters_.empty() ? nullptr : rootParameters_.data();
	rootSignatureDesc.NumStaticSamplers = static_cast<UINT>(staticSamplers_.size());
	rootSignatureDesc.pStaticSamplers = staticSamplers_.empty() ? nullptr : staticSamplers_.data();
	rootSignatureDesc.Flags = flags_;

	// シリアライズ
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

	HRESULT hr = D3D12SerializeRootSignature(
		&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&signatureBlob,
		&errorBlob);

	if (FAILED(hr)) {
		if (errorBlob) {
			Logger::Log(Logger::GetStream(),
				std::format("RootSignatureBuilder: Serialization error - {}\n",
					reinterpret_cast<char*>(errorBlob->GetBufferPointer())));
		}
		return nullptr;
	}

	// RootSignatureを生成
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
	hr = device->CreateRootSignature(
		0,
		signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));

	if (FAILED(hr)) {
		Logger::Log(Logger::GetStream(), "RootSignatureBuilder: Failed to create RootSignature\n");
		return nullptr;
	}

	Logger::Log(Logger::GetStream(),
		std::format("RootSignatureBuilder: Successfully created RootSignature with {} parameters\n",
			rootParameters_.size()));

	return rootSignature;
}

void RootSignatureBuilder::UpdateDescriptorTablePointers() {
	// DescriptorTableのポインタを更新（vectorの再配置に対応）
	size_t descriptorTableIndex = 0;
	for (auto& param : rootParameters_) {
		if (param.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
			if (descriptorTableIndex < descriptorRangeHolders_.size()) {
				param.DescriptorTable.pDescriptorRanges =
					descriptorRangeHolders_[descriptorTableIndex]->ranges.data();
				descriptorTableIndex++;
			}
		}
	}
}