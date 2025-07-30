#include "Skydome.h"

Skydome::Skydome() {
}

Skydome::~Skydome() {
}

void Skydome::Initialize(DirectXCommon* dxCommon) {
	directXCommon_ = dxCommon;

	// ゲームオブジェクト（3Dモデル）の初期化
	gameObject_ = std::make_unique<Model3D>();
	gameObject_->Initialize(dxCommon, "skydome");
	gameObject_->SetName("Skydome");

	// 初期位置設定
	Vector3Transform defaultTransform{
		{1.0f, 1.0f, 1.0f},  // scale
		{0.0f, 0.0f, 0.0f},  // rotate
		{0.0f, 0.0f, 0.0f}   // translate
	};
	gameObject_->SetTransform(defaultTransform);
	gameObject_->GetMaterial().SetLightingMode(LightingMode::None);
}

void Skydome::Update(const Matrix4x4& viewProjectionMatrix) {
	// ゲームオブジェクトの更新
	gameObject_->Update(viewProjectionMatrix);
}

void Skydome::Draw(const Light& directionalLight) {
	// 天球の描画
	gameObject_->Draw(directionalLight);
}