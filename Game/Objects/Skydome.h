#pragma once

#include "Engine.h"
#include "Objects/GameObject/GameObject.h"

/// <summary>
/// 天球クラス
/// </summary>
class Skydome {

public:

	void Initialize();

	void Update(const Matrix4x4& viewProjectionMatrix);

	void Draw(const Light& directionalLight);

	void ImGui();

private:

	//天球
	std::unique_ptr<Model3D> Object_ = nullptr;




	// システム参照
	DirectXCommon* directXCommon_;

};