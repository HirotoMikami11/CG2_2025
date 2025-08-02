#include "TitlePlayer.h"

void TitlePlayer::Initialize()
{
	// システム参照の取得
	directXCommon_ = Engine::GetInstance()->GetDirectXCommon();

	//初期座標
	Vector3 TitlePlayerPos = { 0.0f, -1.0f,6.5f };

	//初期化、座標設定
	Object_ = std::make_unique<Model3D>();
	Object_->Initialize(directXCommon_, "player");
	Object_->SetPosition(TitlePlayerPos);
	//回転方向を右方向
	Object_->SetRotation({ 0.0f,std::numbers::pi_v<float>,0.0f });

}

void TitlePlayer::Update(const Matrix4x4& viewProjectionMatrix)
{

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


