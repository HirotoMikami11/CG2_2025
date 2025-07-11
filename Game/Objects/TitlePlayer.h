#pragma once

#include "Engine.h"
#include "Objects/GameObject/GameObject.h"

class TitlePlayer {

public:

	TitlePlayer() = default;
	~TitlePlayer() = default;


	void Initialize();

	void Update(const Matrix4x4& viewProjectionMatrix);

	void Draw(const Light& directionalLight);

	void ImGui();

private:

	//タイトル用のプレイヤー
	std::unique_ptr<Model3D> Object_ = nullptr;


	//回転速度
	Vector3 rotate_;



	// システム参照
	DirectXCommon* directXCommon_;

};