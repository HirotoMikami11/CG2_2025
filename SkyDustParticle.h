#pragma once
#include "TriangularPyramid.h"

class SkyDustParticle {
public:
	SkyDustParticle(ID3D12Device* device, Vector3Transform setTransform);  // デバイスを受け取るように
	~SkyDustParticle();

	// コピー禁止
	SkyDustParticle(const SkyDustParticle&) = delete;
	SkyDustParticle& operator=(const SkyDustParticle&) = delete;


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
	void Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle, const Matrix4x4& viewProjection);

private:
	Vector3Transform transform;
	float lifeTime;
	float currentTime;
	float alphaSpeed;
	float rotationSpeed;
	float speed;
	float alpha;

	TriangularPyramid* pyramid;  // 各パーティクルに固有のピラミッド
};
