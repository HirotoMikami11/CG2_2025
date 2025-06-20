#include "TitlePlayer.h"

void TitlePlayer::Initialize()
{
	// システム参照の取得
	directXCommon_ = Engine::GetInstance()->GetDirectXCommon();

	//初期座標
	Vector3 titlePlayerPos = { 0.0, -1.0, -1.0 };

	//初期化、座標設定
	Object_ = std::make_unique<Model3D>();
	Object_->Initialize(directXCommon_, "resources/Player", "player.obj");
	Object_->SetPosition(titlePlayerPos);

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
	ImGui::Text("TitlePlayer");
	Object_->ImGui();
	ImGui::Spacing();

}


