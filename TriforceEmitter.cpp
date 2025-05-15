#include "TriforceEmitter.h"

TriforceEmitter::TriforceEmitter(ID3D12Device* device)
    : device(device), spawnTimer(0.0f), spawnInterval(0.8f) {
}

TriforceEmitter::~TriforceEmitter() {
    // アクティブなパーティクルを解放
    for (TriforceParticle* particle : activeParticles) {
        delete particle;
    }
    activeParticles.clear();

    // プール内のパーティクルを解放
    while (!particlePool.empty()) {
        TriforceParticle* particle = particlePool.front();
        particlePool.pop();
        delete particle;
    }
}

void TriforceEmitter::Initialize() {
    // パーティクルプールを事前に作成
    Vector3Transform initialTransform = {};
    for (size_t i = 0; i < MAX_PARTICLES; i++) {
        TriforceParticle* particle = new TriforceParticle(device, initialTransform);
        particlePool.push(particle);
    }
}

void TriforceEmitter::Update(float deltaTime, const Vector3Transform& vector3Transform, float easeT, const Matrix4x4& viewProjection) {
    // パーティクル生成
    if (easeT != 1.0f) {
        // イージングが完了するまで残像を生成する
        spawnTimer += deltaTime;
        while (spawnTimer >= spawnInterval && !particlePool.empty()) {
            spawnTimer -= spawnInterval;

            // プールからパーティクルを取得
            TriforceParticle* particle = particlePool.front();
            particlePool.pop();

            // パーティクルをリセット
            triforceTransform = vector3Transform;
            particle->Reset(triforceTransform);

            // アクティブリストに追加
            activeParticles.push_back(particle);

            // 周回用にフラグを戻す
            hasSpawnedOnEaseEnd = false;
        }
    } else if (!hasSpawnedOnEaseEnd && !particlePool.empty()) {
        // イージングが終了したときにのみ行う処理
        TriforceParticle* particle = particlePool.front();
        particlePool.pop();

        triforceTransform = vector3Transform;
        particle->Reset(triforceTransform);

        activeParticles.push_back(particle);
        specialParticle = particle; // 最後に生成されたものを追跡
        hasSpawnedOnEaseEnd = true; // 1回きりにする
    }

    // パーティクル更新
    for (auto it = activeParticles.begin(); it != activeParticles.end(); ) {
        // 生きているパーティクルは更新
        if (*it == specialParticle) {
            (*it)->UpdateEaseEnd(deltaTime, viewProjection); // 特別な更新
        } else {
            (*it)->Update(deltaTime, viewProjection); // 通常の更新
        }

        // 死んだらプールに戻す
        if (!(*it)->IsAlive()) {
            // 削除対象が specialParticle ならリセット
            if (*it == specialParticle) {
                specialParticle = nullptr;
            }

            // プールに戻す
            particlePool.push(*it);
            // アクティブリストから削除
            it = activeParticles.erase(it);
        } else {
            ++it;
        }
    }
}

void TriforceEmitter::Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle) {
    for (auto& particle : activeParticles) {
        particle->Draw(commandList, textureHandle);
    }
}