#pragma once

#include "../../../../../Engine/Engine.h"
#include "../../../../../GameObject.h"

class Player {

public:

	void Initialize();

	void Update(const Matrix4x4& viewProjectionMatrix);

	void Draw(const Light& directionalLight);

	void ImGui();

private:

	//プレイヤー
	std::unique_ptr<Model3D> Object_ = nullptr;


	// システム参照
	DirectXCommon* directXCommon_;

};