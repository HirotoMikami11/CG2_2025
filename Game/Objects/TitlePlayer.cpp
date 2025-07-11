#include "TitlePlayer.h"

void TitlePlayer::Initialize()
{
	// システム参照の取得
	directXCommon_ = Engine::GetInstance()->GetDirectXCommon();

	//初期座標
	Vector3 TitlePlayerPos = { 0.0, -1.0, -1.0 };

	//初期化、座標設定
	Object_ = std::make_unique<Model3D>();
	Object_->Initialize(directXCommon_,"player");
	Object_->SetPosition(TitlePlayerPos);
	//回転方向を右方向
	Object_->SetRotation({ 0.0f,std::numbers::pi_v<float>,0.0f });
	//回転測度の設定
	rotate_ = { 0,0.005f,0 };
}

void TitlePlayer::Update(const Matrix4x4& viewProjectionMatrix)
{
	Object_->AddRotation(rotate_);
	Object_->Update(viewProjectionMatrix);
}


void TitlePlayer::Draw(const Light& directionalLight)
{
	Object_->Draw(directionalLight);
}

void TitlePlayer::ImGui()
{
#ifdef _DEBUG

	ImGui::Text("TitlePlayer");
	Object_->ImGui();
	ImGui::Spacing();

#endif
}


