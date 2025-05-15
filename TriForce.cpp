#include"TriForce.h"

TriForce::TriForce(ID3D12Device* device)
{
	for (int i = 0; i < indexTriangularPrism; i++) {
		triangularPrism[i] = new TriangularPrism();
		triangularPrism[i]->Initialize(device);
		moveStart[i] = { 0.0f,0.0f,0.0f };
		rotateStart[i] = { 30.0f,30.0f,30.0f };
		rotateEnd[i] = { 0.0f,0.0f,0.0f };

		triforceEmitter[i] = new TriforceEmitter(device);
	}
	moveEnd[0] = { 0.0f,0.55f,0.0f };
	moveEnd[1] = { -0.5f,-0.47f,0.0f };
	moveEnd[2] = { 0.5f,-0.47f,0.0f };
	t = 0;
	shouldStartEasing = false; // 初期状態ではイージングしない
		endEaseTimer = 0.0f;
}

TriForce::~TriForce()
{
	for (int i = 0; i < indexTriangularPrism; i++) {
		delete triangularPrism[i];
		delete triforceEmitter[i];
	}
}

void TriForce::Initialize()
{
	for (int i = 0; i < indexTriangularPrism; i++) {
		triangularPrism[i]->SetRotation({ 0.0f, 0.0f, 0.0f });
		triangularPrism[i]->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
	}

	//上
	triangularPrism[0]->SetPosition({ 0.0f, 7.0f, 15.0f });
	//左
	triangularPrism[1]->SetPosition({ -10.0f, -7.0f, 15.0f });
	//右
	triangularPrism[2]->SetPosition({ 10.0f, -7.0f, 15.0f });

	for (int i = 0; i < indexTriangularPrism; i++) {
		moveStart[i] = triangularPrism[i]->GetTransform().translate;
		rotateStart[i] = { 20.0f,10.0f,10.0f };
		rotateEnd[i] = { 0.0f,0.0f,0.0f };
	}
	moveEnd[0] = { 0.0f,0.55f,0.0f };
	moveEnd[1] = { -0.5f,-0.47f,0.0f };
	moveEnd[2] = { 0.5f,-0.47f,0.0f };
	t = 0;
	shouldStartEasing = false; // Initialize時はイージングしない
}

void TriForce::ResetProgress()
{
	// イージングの進行度を0にリセット
	t = 0.0f;

	// 必要に応じて他の状態もリセット
	endEaseTimer = 0.0f;

	// イージング開始フラグもリセット
	shouldStartEasing = false;

	// 三角柱を初期位置に戻す
	for (int i = 0; i < indexTriangularPrism; i++) {
		triangularPrism[i]->SetPosition(moveStart[i]);
		triangularPrism[i]->SetRotation(rotateStart[i]);
	}
}

void TriForce::Update(const Matrix4x4& viewProjectionMatrix)
{
	for (int i = 0; i < indexTriangularPrism; i++) {
		triangularPrism[i]->Update(viewProjectionMatrix);
	}
}

void TriForce::Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle)
{
	///三角柱それぞれを描画
	for (int i = 0; i < indexTriangularPrism; i++) {
		triangularPrism[i]->Draw(commandList, textureHandle);
		triforceEmitter[i]->Draw(commandList, textureHandle);
	}
}

void TriForce::MoveEasing(const Matrix4x4& viewProjection)
{
	// イージング開始フラグがfalseの場合は何もしない
	if (!shouldStartEasing) {
		return;
	}

	// イージング進行
	t += (1.0f / (360.0f));
	t = std::clamp(t, 0.0f, 1.0f);

	//イージング
	float easeT;

	easeT = easeOutCubic(t);///第一候補！

	for (int i = 0; i < indexTriangularPrism; i++) {
		triangularPrism[i]->SetPosition(Lerp(moveStart[i], moveEnd[i], easeT));
		triangularPrism[i]->SetRotation(Lerp(rotateStart[i], rotateEnd[i], easeT));
		///イージングが完了するまで残像を生成する
		triforceEmitter[i]->Update((1.0f / 60.0f), triangularPrism[i]->GetTransform(), easeT, viewProjection);
	}
}