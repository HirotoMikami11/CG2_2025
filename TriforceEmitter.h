#pragma once
#include "TriforceParticle.h"
#include <vector>

/// <summary>
/// トライフォースの残像を生成するエミッター
/// </summary>
class TriforceEmitter
{
public:
	TriforceEmitter(ID3D12Device* device);
	~TriforceEmitter();

	void Update(float deltaTime,const Vector3Transform& vector3Transform, float easeT, const Matrix4x4& viewProjection);
	void Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle);

private:
	std::vector<TriforceParticle*> particles;
	float spawnTimer;
	float spawnInterval;
	///トライフォースの座標を持っておく
	Vector3Transform triforceTransform;
	ID3D12Device* device;
	bool hasSpawnedOnEaseEnd = false;
	TriforceParticle* specialParticle = nullptr;
};

