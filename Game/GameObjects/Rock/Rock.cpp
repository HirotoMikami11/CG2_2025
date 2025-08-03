#include "Rock.h"
#include "Managers/ImGui/ImGuiManager.h"

Rock::Rock()
	: rockType_(RockType::Rock1) {
}

Rock::~Rock() {
}

void Rock::Initialize(DirectXCommon* dxCommon, RockType rockType,
	const Vector3& position, const Vector3& rotation, const Vector3& scale) {

	rockType_ = rockType;

	// Model3Dの初期化（岩タイプに応じたモデルを読み込み）
	std::string modelName = GetModelNameFromType(rockType_);
	Model3D::Initialize(dxCommon, modelName);

	// 名前を設定
	SetName("Rock_" + RockTypeToString(rockType_));

	// Transform設定
	SetPosition(position);
	SetRotation(rotation);
	SetScale(scale);

	// デフォルトでライティング有効
	SetLightingMode(LightingMode::HalfLambert);
}

void Rock::ImGui() {
#ifdef _DEBUG
	if (ImGui::TreeNode(GetName().c_str())) {
		// 岩タイプの選択
		const char* rockTypeNames[] = { "Rock1", "Rock2", "Rock3" };
		int currentType = static_cast<int>(rockType_);
		if (ImGui::Combo("Rock Type", &currentType, rockTypeNames, IM_ARRAYSIZE(rockTypeNames))) {
			SetRockType(static_cast<RockType>(currentType));
		}

		// 基底クラスのImGuiを呼び出し
		Model3D::ImGui();

		ImGui::TreePop();
	}
#endif
}

void Rock::SetRockType(RockType rockType) {
	if (rockType_ != rockType) {
		rockType_ = rockType;

		// モデルを変更
		std::string modelName = GetModelNameFromType(rockType_);
		// 新しいモデルで再初期化（位置などは保持）
		Vector3 currentPos = GetPosition();
		Vector3 currentRot = GetRotation();
		Vector3 currentScale = GetScale();

		Model3D::Initialize(directXCommon_, modelName);
		SetName("Rock_" + RockTypeToString(rockType_));

		// Transform値を復元
		SetPosition(currentPos);
		SetRotation(currentRot);
		SetScale(currentScale);
	}
}

std::string Rock::RockTypeToString(RockType rockType) {
	switch (rockType) {
	case RockType::Rock1: return "Rock1";
	case RockType::Rock2: return "Rock2";
	case RockType::Rock3: return "Rock3";
	default: return "Rock1";
	}
}

RockType Rock::StringToRockType(const std::string& str) {
	if (str == "Rock1") return RockType::Rock1;
	if (str == "Rock2") return RockType::Rock2;
	if (str == "Rock3") return RockType::Rock3;
	return RockType::Rock1; // デフォルト
}

std::string Rock::GetModelNameFromType(RockType rockType) {
	switch (rockType) {
	case RockType::Rock1: return "rock1";
	case RockType::Rock2: return "rock2";
	case RockType::Rock3: return "rock3";
	default: return "rock1";
	}
}