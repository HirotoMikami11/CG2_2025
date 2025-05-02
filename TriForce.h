#pragma once

#include"TriangularPrism.h"

class TriForce {
public:
	// コンストラクタ、デストラクタ
	TriForce();
	~TriForce();

	// 初期化
	void Initialize(ID3D12Device* device);

	// 更新
	void Update(const Matrix4x4& viewProjectionMatrix);


	// 描画
	void Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle);



	const Vector3Transform& GetTransform() const { return triangularPrism[0]->GetTransform(); }

private:
	///トライフォースになる三角柱
	TriangularPrism* triangularPrism[3];

};