#include "TriforceParticle.h"


TriforceParticle::TriforceParticle(ID3D12Device* device, Vector3Transform& vector3Transform) {
	transform = vector3Transform;
	lifeTime = 1.0f;
	currentTime = 0.0f;
	alpha = 1.0f;
	prism = new TriangularPrism();
	// ピラミッド初期化
	prism->Initialize(device);
	//トライフォースの座標をそのままもらう
	prism->SetTransform(transform);
}

TriforceParticle::~TriforceParticle()
{
	delete prism;
}

void TriforceParticle::Reset(const Vector3Transform& newTransform) {
	transform = newTransform;
	lifeTime = 1.0f;
	currentTime = 0.0f;
	alpha = 1.0f;
	// 新しい座標を設定
	prism->SetTransform(transform);
}

void TriforceParticle::Update(float deltaTime, const Matrix4x4& viewProjection) {
	currentTime += deltaTime;

	alpha = 1.0f - (currentTime / lifeTime);
	if (alpha < 0.0f) alpha = 0.0f;

	Vector4 color = { 1.0f,1.0f,1.0f,alpha };
	prism->SetColor(color);
	prism->Update(viewProjection);
}

void TriforceParticle::UpdateEaseEnd(float deltaTime, const Matrix4x4& viewProjection)
{
	currentTime += deltaTime;
	alpha = 1.0f - (currentTime / lifeTime);
	if (alpha < 0.0f) alpha = 0.0f;

	Vector4 color = { 1.0f,1.0f,0.0f,alpha };//完成した感を出すために黄色で着色
	prism->SetColor(color);

	prism->Update(viewProjection);

}

void TriforceParticle::Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle) {
	prism->Draw(commandList, textureHandle);
}