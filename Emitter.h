#pragma once
#include "Particle.h"
#include <vector>
#include <queue>
#include "TriangularPyramid.h"

class Emitter {
public:
	Emitter(ID3D12Device* device);
	~Emitter();


	void Initialize(); // パーティカルプール
	void Update(float deltaTime);
	void Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle, const Matrix4x4& viewProjection);

private:
	const size_t MAX_PARTICLES = 100; // 適切な数に調整

	std::vector<Particle*> activeParticles; // アクティブなパーティクル
	std::queue<Particle*> particlePool;    // 待機中のパーティクル
	float spawnTimer;
	float spawnInterval;
	Vector3Transform SetParticles;	//パーティクルにセットするTransform情報
	ID3D12Device* device;
};

