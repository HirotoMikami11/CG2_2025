#pragma once
#include "TriforceParticle.h"
#include <vector>
#include <queue>

/// <summary>
/// トライフォースの残像を生成するエミッター（パーティクルプール対応）
/// </summary>
class TriforceEmitter
{
public:
	TriforceEmitter(ID3D12Device* device);
	~TriforceEmitter();

	void Initialize(); // パーティクルプール初期化
	void Update(float deltaTime, const Vector3Transform& vector3Transform, float easeT, const Matrix4x4& viewProjection);
	void Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle);

private:
	const size_t MAX_PARTICLES = 9; // プールの最大サイズ

	std::vector<TriforceParticle*> activeParticles;	// アクティブなパーティクル
	std::queue<TriforceParticle*> particlePool;		// 待機中のパーティクル

	float spawnTimer;
	float spawnInterval;
	Vector3Transform triforceTransform;
	ID3D12Device* device;
	bool hasSpawnedOnEaseEnd = false;
	TriforceParticle* specialParticle = nullptr;
};