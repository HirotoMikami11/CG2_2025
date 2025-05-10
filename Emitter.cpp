#include "Emitter.h"

///初期化リストで初期化（うまくいかなかったら戻す）
Emitter::Emitter(ID3D12Device* device) : device(device), spawnTimer(0.0f), spawnInterval(0.025f) {}

Emitter::~Emitter() {
	// アクティブなパーティクルを解放
	for (Particle* particle : activeParticles) {
		delete particle;
	}
	activeParticles.clear();

	// プール内のパーティクルを解放
	while (!particlePool.empty()) {
		Particle* particle = particlePool.front();
		particlePool.pop();
		delete particle;
	}
}

void Emitter::Initialize() {
	// パーティクルプールを事前に作成
	for (size_t i = 0; i < MAX_PARTICLES; i++) {
		Vector3Transform initialTransform = {}; // 初期値は空
		Particle* particle = new Particle(device, initialTransform);
		particlePool.push(particle);
	}
}

void Emitter::Update(float deltaTime) {
	// 生成
	spawnTimer += deltaTime;


	while (spawnTimer >= spawnInterval && !particlePool.empty()) {
		spawnTimer -= spawnInterval;

		// プールからパーティクルを取得
		Particle* particle = particlePool.front();
		particlePool.pop();

		// パーティクルを初期化
		SetParticles.scale = { RandomFloat(0.01f, 0.08f), RandomFloat(0.01f, 0.08f), RandomFloat(0.01f, 0.08f) };
		SetParticles.rotate = { 0.0f, 0.0f, RandomFloat(0.0f, 3.14f) };
		SetParticles.translate = { RandomFloat(-5.0f, 5.0f), -2.0f, RandomFloat(-5.0f, 5.0f) };

		// パーティクルをリセットしてアクティブリストに追加
		particle->Reset(SetParticles);
		activeParticles.push_back(particle);
	}

	// 更新
	for (auto it = activeParticles.begin(); it != activeParticles.end(); ) {
		// 生きているパーティクルは更新
		(*it)->Update(deltaTime);

		// 死んだらプールに戻す
		if (!(*it)->IsAlive()) {
			particlePool.push(*it); // プールに戻す
			it = activeParticles.erase(it);
		} else {
			++it;
		}
	}
}

void Emitter::Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle, const Matrix4x4& viewProjection) {
	for (auto& particle : activeParticles) {
		particle->Draw(commandList, textureHandle, viewProjection);
	}
}