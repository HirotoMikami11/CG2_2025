#pragma once

#include"TriangularPrism.h"
#include <algorithm>  // std::clamp
#include"Easing.h"

class TriForce {
public:
	// コンストラクタ、デストラクタ
	TriForce(ID3D12Device* device);
	~TriForce();

	// 初期化
	void Initialize();


	//イージングする動作の関数
	void MoveEasing(int easing_num);
	// 更新
	void Update(const Matrix4x4& viewProjectionMatrix);


	// 描画
	void Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle);


	Vector4& GetColor() {
		return triangularPrism[0]->GetColor();
	}

	const Vector3Transform& GetTransform() const { return triangularPrism[0]->GetTransform(); }

private:
	///トライフォースになる三角柱
	TriangularPrism* triangularPrism[3];
	///インデックス
	const int indexTriangularPrism = 3;

	///イージング用の変数
	Vector3 moveStart[3];
	Vector3 moveEnd[3];
	Vector3 rotateStart[3];
	Vector3 rotateEnd[3];
	float t;


	
};