#include"TriForce.h"

TriForce::TriForce(ID3D12Device* device)
{
	for (int i = 0; i < indexTriangularPrism; i++) {
		triangularPrism[i] = new TriangularPrism();
		triangularPrism[i]->Initialize(device);
		moveStart[i] = { 0.0f,0.0f,0.0f };
		rotateStart[i] = { 30.0f,30.0f,30.0f };
		rotateEnd[i] = { 0.0f,0.0f,0.0f };
	}
	moveEnd[0] = { 0.0f,0.55f,0.0f };
	moveEnd[1] = { -0.5f,-0.47f,0.0f };
	moveEnd[2] = { 0.5f,-0.47f,0.0f };
	t = 0;
}

TriForce::~TriForce()
{
	for (int i = 0; i < indexTriangularPrism; i++) {
		delete triangularPrism[i];
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
	triangularPrism[2]->SetPosition({ 10.0f, -7.0f, 15.0f});

	for (int i = 0; i < indexTriangularPrism; i++) {
		moveStart[i] = triangularPrism[i]->GetTransform().translate;
		rotateStart[i] = { 20.0f,10.0f,10.0f };
		rotateEnd[i] = { 0.0f,0.0f,0.0f };
	}
	moveEnd[0] = { 0.0f,0.55f,0.0f };
	moveEnd[1] = { -0.5f,-0.47f,0.0f };
	moveEnd[2] = { 0.5f,-0.47f,0.0f };
	t = 0;

}

void TriForce::Update(const Matrix4x4& viewProjectionMatrix)
{

	//イージング
	//MoveEasing();

	for (int i = 0; i < indexTriangularPrism; i++) {
		triangularPrism[i]->Update(viewProjectionMatrix);
	}

}

void TriForce::Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle)
{

	///三角柱それぞれを描画
	for (int i = 0; i < indexTriangularPrism; i++) {
		triangularPrism[i]->Draw(commandList, textureHandle);
	}

}

void TriForce::MoveEasing(int easing_num)
{

	t += (1.0f / (240.0f*1.5f));
	t = std::clamp(t, 0.0f, 1.0f);
	//イージング
	float easeT;
	switch (easing_num) {
	case 0:
		easeT = easeOutQuad(t);	///第一候補！！
		break;
	case 1:
		easeT = easeOutBack(t);
		break;
	case 2:
		easeT = easeInOutCubic(t);
		break;

	case 3:
		easeT = easeOutBounce(t);
		break;

	case 4:
		easeT = EaseOutSine(t);
		break;

	case 5:
		easeT = easeOutExpo(t);
		break;
	default:
		easeT = t;
		break;
	}


	for (int i = 0; i < indexTriangularPrism; i++) {
		triangularPrism[i]->SetPosition(Lerp(moveStart[i], moveEnd[i], easeT));
		triangularPrism[i]->SetRotation(Lerp(rotateStart[i], rotateEnd[i], easeT));
	}
}

