#include "Player.h"

void Player::Initialize()
{
	// システム参照の取得
	directXCommon_ = Engine::GetInstance()->GetDirectXCommon();

	//初期座標
	Vector3 PlayerPos = { 0.0, -1.0, -1.0 };

	//初期化、座標設定
	Object_ = std::make_unique<Model3D>();
	Object_->Initialize(directXCommon_, "resources/Model/Player", "player.obj");
	Object_->SetPosition(PlayerPos);


}

void Player::Update(const Matrix4x4& viewProjectionMatrix)
{
	Object_->Update(viewProjectionMatrix);
}


void Player::Draw(const Light& directionalLight)
{
	Object_->Draw(directionalLight);
}

void Player::ImGui()
{
#ifdef _DEBUG

	ImGui::Text("Player");
	Object_->ImGui();
	ImGui::Spacing();

#endif
}


