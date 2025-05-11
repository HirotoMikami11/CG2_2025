#include "SkyDustParticle.h"


SkyDustParticle::SkyDustParticle(ID3D12Device* device, Vector3Transform setTransform) {
	transform.translate = setTransform.translate;
	transform.scale = setTransform.scale;
	transform.rotate = setTransform.rotate;

	lifeTime = 1.0f;
	currentTime = 0.0f;
	alphaSpeed = 1.0f;
	rotationSpeed = RandomFloat(-3.0f, 3.0f);
	//EmitterのImguiで変更される
	velocity = Vector3(0.0f, -0.3f, 0.3f);
	alpha = 0.0f;

	pyramid = new TriangularPyramid();
	// ピラミッド初期化
	pyramid->Initialize(device);
}

SkyDustParticle::~SkyDustParticle()
{
	delete pyramid;
}

void SkyDustParticle::Reset(const Vector3Transform& newTransform) {
	transform = newTransform;
	currentTime = 0.0f;
	alphaSpeed = 1.0f;
	rotationSpeed = RandomFloat(-3.0f, 3.0f);
	// velocityはSetVelocityで設定するので、ここでは初期化しない
	alpha = 0.0f;
}

void SkyDustParticle::Update(float deltaTime) {
	currentTime += deltaTime;

	//指定した方向に移動するように変更
	transform.translate.x += velocity.x * deltaTime;
	transform.translate.y += velocity.y * deltaTime;
	transform.translate.z += velocity.z * deltaTime;


	transform.rotate.y += rotationSpeed * deltaTime;

	if (currentTime < lifeTime / 2.0f) {
		alpha = currentTime / (lifeTime / 2.0f);
	} else {
		alpha = 1.0f - (currentTime - lifeTime / 2.0f) / (lifeTime / 2.0f);
	}

	if (alpha < 0.0f) alpha = 0.0f;
}

void SkyDustParticle::Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle, const Matrix4x4& viewProjection) {
	pyramid->SetTransform(transform);
	Vector4 color = { 1.0f,1.0f,1.0f,alpha };
	pyramid->SetColor(color);
	pyramid->Update(viewProjection);
	pyramid->Draw(commandList, textureHandle);
}