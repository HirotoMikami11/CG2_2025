#pragma once
#include "TriangularPyramid.h"

class Particle {
public:
	Particle(ID3D12Device* device,Vector3Transform setTransform);  // デバイスを受け取るように
	~Particle(); 

	// コピー禁止
	Particle(const Particle&) = delete;
	Particle& operator=(const Particle&) = delete;

	//リセット関数(パーティカルプール用)
	void Reset(const Vector3Transform& newTransform);

	void Update(float deltaTime);

	bool IsAlive() const {
		return currentTime < lifeTime;
	}

	const Vector3Transform& GetTransform() const {
		return transform;
	}

	float GetAlpha() const {
		return alpha;
	}

	void SetVelocity(const Vector3& newVelocity) {
		velocity = newVelocity;
	}
	void SetColor(const Vector4& Color) {
		color = Color;
	}
	void Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle, const Matrix4x4& viewProjection);

private:
	Vector3Transform transform;
	float lifeTime;
	float currentTime;
	float alphaSpeed;
	float rotationSpeed;
	Vector3 velocity; // 移動速度ベクトル(speedを変更)
	float alpha;
	Vector4 color ;
	TriangularPyramid* pyramid;  // 各パーティクルに固有のピラミッド
};
