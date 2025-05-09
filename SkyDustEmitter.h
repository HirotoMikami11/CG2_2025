#pragma once
#include "SkyDustParticle.h"
#include <vector>
#include "TriangularPyramid.h"

class SkyDustEmitter
{
public:
	SkyDustEmitter(ID3D12Device* device);
	~SkyDustEmitter();

	void Update(float deltaTime);
	void Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle, const Matrix4x4& viewProjection);

private:
	std::vector<SkyDustParticle*> particles;
	float spawnTimer;
	float spawnInterval;
	Vector3Transform SetParticles;	//パーティクルにセットするTransform情報
	ID3D12Device* device;



};

