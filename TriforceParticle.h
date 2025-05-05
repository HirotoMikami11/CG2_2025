#pragma once
#include"TriangularPrism.h"

class TriforceParticle
{
public:
	TriforceParticle(ID3D12Device* device, Vector3Transform& vector3Transform);  // デバイスを受け取るように
	~TriforceParticle();

	// コピー禁止
	TriforceParticle(const TriforceParticle&) = delete;
	TriforceParticle& operator=(const TriforceParticle&) = delete;

	void Update(float deltaTime, const Matrix4x4& viewProjection);
	//イージングが終わった後一個だけに適用するUpdate
	void UpdateEaseEnd(float deltaTime, const Matrix4x4& viewProjection);

	bool IsAlive() const {
		return currentTime < lifeTime;
	}

	const Vector3Transform& GetTransform() const {
		return transform;
	}

	float GetAlpha() const {
		return alpha;
	}
	void Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle);

private:
	Vector3Transform transform;
	float lifeTime;
	float currentTime;
	float alpha;

	TriangularPrism* prism;  // 各パーティクルにトライフォースと同じ形
};

