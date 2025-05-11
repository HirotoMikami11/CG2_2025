#include "SkyDustEmitter.h"

SkyDustEmitter::SkyDustEmitter(ID3D12Device* device) : device(device), spawnTimer(0.0f), spawnInterval(0.025f) {
	translateMin = Vector3(-5.0f, 1.8f, -5.0f);
	translateMax = Vector3(5.0f, 2.0f, 5.0f);
	rotateMin = Vector3(0.0f, 0.0f, 0.0f);
	rotateMax = Vector3(0.0f, 0.0f, 3.14f);
	scaleMin = Vector3(0.02f, 0.02f, 0.02f);
	scaleMax = Vector3(0.03f, 0.03f, 0.03f);

	minSpeed = 0.2f;
	maxSpeed = 1.0f;
	moveDirection = Vector3(-1.0f, 0.0f, 1.0f); // 下方向とZ軸正方向に移動
	directionVariance = 0.2f;
}

void SkyDustEmitter::Initialize() {
	// パーティクルプールを事前に作成
	Vector3Transform initialTransform = {}; // 初期値
	for (size_t i = 0; i < MAX_PARTICLES; i++) {
		SkyDustParticle* particle = new SkyDustParticle(device, initialTransform);
		particlePool.push(particle);
	}
}

SkyDustEmitter::~SkyDustEmitter() {
	// アクティブなパーティクルを解放
	for (SkyDustParticle* particle : activeParticles) {
		delete particle;
	}
	activeParticles.clear();

	// プール内のパーティクルを解放
	while (!particlePool.empty()) {
		SkyDustParticle* particle = particlePool.front();
		particlePool.pop();
		delete particle;
	}
}

void SkyDustEmitter::Update(float deltaTime) {
	
	ImGui();

	// 生成
	spawnTimer += deltaTime;
	while (spawnTimer >= spawnInterval && !particlePool.empty()) {
		spawnTimer -= spawnInterval;

		// パーティクルの設定を準備
		SetParticles.scale = Vector3(
			RandomFloat(scaleMin.x, scaleMax.x),
			RandomFloat(scaleMin.y, scaleMax.y),
			RandomFloat(scaleMin.z, scaleMax.z)
		);

		SetParticles.rotate = Vector3(
			RandomFloat(rotateMin.x, rotateMax.x),
			RandomFloat(rotateMin.y, rotateMax.y),
			RandomFloat(rotateMin.z, rotateMax.z)
		);

		SetParticles.translate = Vector3(
			RandomFloat(translateMin.x, translateMax.x),
			RandomFloat(translateMin.y, translateMax.y),
			RandomFloat(translateMin.z, translateMax.z)
		);

		// プールからパーティクルを取得
		SkyDustParticle* particle = particlePool.front();
		particlePool.pop();

		// パーティクルをリセット
		particle->Reset(SetParticles);

		// ランダムな速度の値を設定
		float speed = RandomFloat(minSpeed, maxSpeed);

		// 方向にばらつきを加える
		Vector3 direction = moveDirection;
		direction.x += RandomFloat(-directionVariance, directionVariance);
		direction.y += RandomFloat(-directionVariance, directionVariance);
		direction.z += RandomFloat(-directionVariance, directionVariance);

		// 方向を正規化
		float length = std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
		if (length > 0.001f) { // 0除算を防ぐ
			direction.x /= length;
			direction.y /= length;
			direction.z /= length;
		}

		// 速度を方向にかける
		Vector3 finalVelocity;
		finalVelocity.x = direction.x * speed;
		finalVelocity.y = direction.y * speed;
		finalVelocity.z = direction.z * speed;

		// パーティクルに速度を設定
		particle->SetVelocity(finalVelocity);

		// アクティブリストに追加
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

void SkyDustEmitter::Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle, const Matrix4x4& viewProjection) {
	for (auto& particle : activeParticles) {
		particle->Draw(commandList, textureHandle, viewProjection);
	}
}

void SkyDustEmitter::ImGui() {

	if (useImGui) {
		ImGui::Begin("skyDustEmitter");

		// 各Vector3変数の編集UI
		ImGui::Text("Translation Range:");
		bool translateMinChanged = ImGui::DragFloat3("Min Translation", &translateMin.x, 0.01f, -5.0f, 5.0f);
		bool translateMaxChanged = ImGui::DragFloat3("Max Translation", &translateMax.x, 0.01f, -5.0f, 5.0f);

		// 最小値・最大値の順序を保証
		if (translateMinChanged) {
			// 最小値が最大値を超えないようにする
			translateMin.x = (translateMin.x > translateMax.x) ? translateMax.x : translateMin.x;
			translateMin.y = (translateMin.y > translateMax.y) ? translateMax.y : translateMin.y;
			translateMin.z = (translateMin.z > translateMax.z) ? translateMax.z : translateMin.z;
		}
		if (translateMaxChanged) {
			// 最大値が最小値を下回らないようにする
			translateMax.x = (translateMax.x < translateMin.x) ? translateMin.x : translateMax.x;
			translateMax.y = (translateMax.y < translateMin.y) ? translateMin.y : translateMax.y;
			translateMax.z = (translateMax.z < translateMin.z) ? translateMin.z : translateMax.z;
		}

		ImGui::Separator();

		ImGui::Text("Rotation Range (radians):");
		bool rotateMinChanged = ImGui::DragFloat3("Min Rotation", &rotateMin.x, 0.01f, 0.0f, 6.28f);
		bool rotateMaxChanged = ImGui::DragFloat3("Max Rotation", &rotateMax.x, 0.01f, 0.0f, 6.28f);

		// 最小値・最大値の順序を保証
		if (rotateMinChanged) {
			rotateMin.x = (rotateMin.x > rotateMax.x) ? rotateMax.x : rotateMin.x;
			rotateMin.y = (rotateMin.y > rotateMax.y) ? rotateMax.y : rotateMin.y;
			rotateMin.z = (rotateMin.z > rotateMax.z) ? rotateMax.z : rotateMin.z;
		}
		if (rotateMaxChanged) {
			rotateMax.x = (rotateMax.x < rotateMin.x) ? rotateMin.x : rotateMax.x;
			rotateMax.y = (rotateMax.y < rotateMin.y) ? rotateMin.y : rotateMax.y;
			rotateMax.z = (rotateMax.z < rotateMin.z) ? rotateMin.z : rotateMax.z;
		}

		ImGui::Separator();

		ImGui::Text("Scale Range:");
		bool scaleMinChanged = ImGui::DragFloat3("Min Scale", &scaleMin.x, 0.001f, 0.001f, 1.0f);
		bool scaleMaxChanged = ImGui::DragFloat3("Max Scale", &scaleMax.x, 0.001f, 0.001f, 1.0f);

		// 最小値・最大値の順序を保証
		if (scaleMinChanged) {
			scaleMin.x = (scaleMin.x > scaleMax.x) ? scaleMax.x : scaleMin.x;
			scaleMin.y = (scaleMin.y > scaleMax.y) ? scaleMax.y : scaleMin.y;
			scaleMin.z = (scaleMin.z > scaleMax.z) ? scaleMax.z : scaleMin.z;
		}
		if (scaleMaxChanged) {
			scaleMax.x = (scaleMax.x < scaleMin.x) ? scaleMin.x : scaleMax.x;
			scaleMax.y = (scaleMax.y < scaleMin.y) ? scaleMin.y : scaleMax.y;
			scaleMax.z = (scaleMax.z < scaleMin.z) ? scaleMin.z : scaleMax.z;
		}
		ImGui::Separator();
		ImGui::Text("Movement Settings:");

		// 速度の範囲
		bool speedChanged = false;
		speedChanged |= ImGui::DragFloat("Min Speed", &minSpeed, 0.01f, 0.01f, maxSpeed);
		speedChanged |= ImGui::DragFloat("Max Speed", &maxSpeed, 0.01f, minSpeed, 2.0f);

		// 速度の範囲を確認
		if (speedChanged) {
			if (minSpeed > maxSpeed) minSpeed = maxSpeed;
			if (maxSpeed < minSpeed) maxSpeed = minSpeed;
		}

		// 移動方向
		ImGui::Text("Movement Direction:");
		ImGui::DragFloat3("Direction", &moveDirection.x, 0.01f, -1.0f, 1.0f);

		// 方向のノーマライズをユーザーに提供
		if (ImGui::Button("Normalize Direction")) {
			float length = std::sqrt(moveDirection.x * moveDirection.x +
				moveDirection.y * moveDirection.y +
				moveDirection.z * moveDirection.z);
			if (length > 0.0f) {
				moveDirection.x /= length;
				moveDirection.y /= length;
				moveDirection.z /= length;
			}
		}

		// パーティクル情報を表示
		ImGui::Separator();
		ImGui::Text("Particle Count: Active %zu / Pool %zu / Total %zu",
			activeParticles.size(), particlePool.size(), MAX_PARTICLES);

		ImGui::End();
	}
}
