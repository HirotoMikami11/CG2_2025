#include "Skydome.h"

void Skydome::Initialize()
{
	// システム参照の取得
	directXCommon_ = Engine::GetInstance()->GetDirectXCommon();

	//初期座標
	Vector3 SkydomePos = { 0.0, 0.0, 0.0 };

	//初期化、座標設定
	Object_ = std::make_unique<Model3D>();
	Object_->Initialize(directXCommon_,"skydome");
	Object_->SetPosition(SkydomePos);

}

void Skydome::Update(const Matrix4x4& viewProjectionMatrix)
{
	Object_->Update(viewProjectionMatrix);
}


void Skydome::Draw(const Light& directionalLight)
{
	Object_->Draw(directionalLight);
}

void Skydome::ImGui()
{
#ifdef _DEBUG

	ImGui::Text("Skydome");
	Object_->ImGui();
	ImGui::Spacing();

#endif
}


