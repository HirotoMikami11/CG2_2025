#include "TriforceEmitter.h"

///初期化リストで初期化（うまくいかなかったら戻す）
TriforceEmitter::TriforceEmitter(ID3D12Device* device) : device(device), spawnTimer(0.0f), spawnInterval(0.8f) {}


TriforceEmitter::~TriforceEmitter() {
	///残っているパーティクルを全部解放
	for (TriforceParticle* particle : particles) {
		delete particle;
	}
	particles.clear();
}


void TriforceEmitter::Update(float deltaTime, const Vector3Transform& vector3Transform, float easeT, const Matrix4x4& viewProjection) {
	// 生成

	///イージングが完了するまで残像を生成する
	if (easeT != 1.0f) {
		spawnTimer += deltaTime;
		while (spawnTimer >= spawnInterval) {
			spawnTimer -= spawnInterval;
			//座標を渡して生成
			triforceTransform = vector3Transform;
			particles.emplace_back(new TriforceParticle(device, triforceTransform));

			//周回用に最後のパーティクル用のフラグを基に戻す
			hasSpawnedOnEaseEnd = false;
		}
	} else if (!hasSpawnedOnEaseEnd) {//イージングが終了したときにのみ行う処理
		triforceTransform = vector3Transform;
		auto* newParticle = new TriforceParticle(device, triforceTransform);
		particles.emplace_back(newParticle);
		specialParticle = newParticle; // 最後に生成されたものを追跡
		hasSpawnedOnEaseEnd = true; // 1回きりにする
	}

	// 更新
	for (auto it = particles.begin(); it != particles.end(); ) {
		//生きているパーティクルは更新
		if (*it == specialParticle) {
			(*it)->UpdateEaseEnd(deltaTime, viewProjection); // 特別な更新
		} else {
			(*it)->Update(deltaTime, viewProjection); // 通常の更新
		}
		//死んだら消す
		if (!(*it)->IsAlive()) {
			// 削除対象が specialParticle ならリセット
			if (*it == specialParticle) specialParticle = nullptr;
			delete* it;
			it = particles.erase(it);
		} else {
			++it;
		}
	}
}

void TriforceEmitter::Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle) {
	for (auto& particle : particles) {
		particle->Draw(commandList, textureHandle);
	}
}