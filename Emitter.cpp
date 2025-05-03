#include "Emitter.h"


Emitter::Emitter(ID3D12Device* device) : device(device), spawnTimer(0.0f), spawnInterval(0.05f) {}

Emitter::~Emitter() {
    ///残っているパーティクルを全部解放
	for (Particle* p : particles) {
		delete p;
	}
	particles.clear();
}

void Emitter::Update(float deltaTime) {
    // 生成
    spawnTimer += deltaTime;
    while (spawnTimer >= spawnInterval) {
        spawnTimer -= spawnInterval;
        particles.emplace_back(new Particle(device));
    }

    // 更新
    for (auto it = particles.begin(); it != particles.end(); ) {
        //生きているパーティクルは更新
        (*it)->Update(deltaTime);
        
        //死んだら消す
        if (!(*it)->IsAlive()) {
            delete* it;
            it = particles.erase(it);
        } else {
            ++it;
        }
    }
}

void Emitter::Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle, const Matrix4x4& viewProjection) {
	for (auto& particle : particles) {
		particle->Draw(commandList, textureHandle, viewProjection);
	}
}