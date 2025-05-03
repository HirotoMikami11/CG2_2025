#include "Particle.h"


Particle::Particle(ID3D12Device* device) {
	transform.translate = { RandomFloat(-5.0f, 5.0f), -2.0f, RandomFloat(-5.0f, 5.0f) };
	transform.scale = { RandomFloat(0.01f, 0.1f), RandomFloat(0.01f, 0.1f),RandomFloat(0.01f, 0.1f) };
	transform.rotate = { 0.0f, 0.0f, RandomFloat(0.0f, 3.14f) };

	lifeTime = 6.0f;
	currentTime = 0.0f;
	alphaSpeed = 1.0f;
	rotationSpeed = RandomFloat(-3.0f, 3.0f);
	speed = RandomFloat(0.005f, 0.3f);
	alpha = 0.0f;

	pyramid = new TriangularPyramid();
	// ピラミッド初期化
	pyramid->Initialize(device);
}

Particle::~Particle()
{
	delete pyramid;
}

void Particle::Update(float deltaTime) {
	currentTime += deltaTime;
	transform.translate.y += speed * deltaTime;
	transform.rotate.y += rotationSpeed * deltaTime;

	if (currentTime < lifeTime / 2.0f) {
		alpha = currentTime / (lifeTime / 2.0f);
	} else {
		alpha = 1.0f - (currentTime - lifeTime / 2.0f) / (lifeTime / 2.0f);
	}

	if (alpha < 0.0f) alpha = 0.0f;
}

void Particle::Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle, const Matrix4x4& viewProjection) {
	pyramid->SetTransform(transform);
	Vector4 color = { 1.0f,1.0f,1.0f,alpha };
	pyramid->SetColor(color);
	pyramid->Update(viewProjection);
	pyramid->Draw(commandList, textureHandle);
}