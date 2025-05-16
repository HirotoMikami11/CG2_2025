#include"TriForce.h"

TriForce::TriForce(ID3D12Device* device)
{
	for (int i = 0; i < indexTriangularPrism; i++) {
		triangularPrism[i] = new TriangularPrism();
		triangularPrism[i]->Initialize(device);

		triforceEmitter[i] = new TriforceEmitter(device);
		triforceEmitter[i]->Initialize();
	}

	//　最後の位置
	finalPosition[0] = { 0.0f, 0.55f, 2.0f };
	finalPosition[1] = { -0.5f, -0.47f, 2.0f };
	finalPosition[2] = { 0.5f, -0.47f, 2.0f };

	// 最後の位置
	finalRotate[0] = { 0.0f, 0.0f, 0.0f };
	finalRotate[1] = { 0.0f, 0.0f, 0.0f };
	finalRotate[2] = { 0.0f, 0.0f, 0.0f };

	//　初期化
	firstT = 0.0f;
	secondT = 0.0f;
	thirdT = 0.0f;
	shouldStartEasing = false;
	currentStage = EasingStage::NOT_STARTED;
	easeStartTimer = kStartTime;
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

	//画面上
	firstPosition[0] = { 0.0f, 30.0f, 50.0f };
	firstPosition[1] = { 0.0f, 30.0f, 50.0f };
	firstPosition[2] = { 0.0f, 30.0f, 50.0f };

	firstRotate[0] = { 10.0f, 10.0f, 10.0f };
	firstRotate[1] = { 10.0f, 10.0f, 10.0f };
	firstRotate[2] = { 10.0f, 10.0f, 10.0f };

	//画面中央
	secondPosition[0] = { 0.0f,0.0f,50.0f };
	secondPosition[1] = { 0.0f,0.0f,50.0f };
	secondPosition[2] = { 0.0f,0.0f,50.0f };

	secondRotate[0] = { 0.0f, float(M_PI) * 2.0f, float(M_PI) };
	secondRotate[1] = { 0.0f, float(M_PI) * 2.0f, float(M_PI) };
	secondRotate[2] = { 0.0f, float(M_PI) * 2.0f, float(M_PI) };

	//演出開始の位置　
	thirdPosition[0] = { 0.0f, 7.0f, 10.0f };
	thirdPosition[1] = { -10.0f, -7.0f, 10.0f };
	thirdPosition[2] = { 10.0f, -7.0f, 10.0f };

	thirdRotate[0] = { 20.0f, 10.0f, 10.0f };
	thirdRotate[1] = { 10.0f, 20.0f, 10.0f };
	thirdRotate[2] = { 10.0f, 10.0f, 20.0f };

	for (int i = 0; i < indexTriangularPrism; i++) {
		triangularPrism[i]->SetPosition(firstPosition[i]);
		triangularPrism[i]->SetRotation(firstRotate[i]);
	}


	firstT = 0.0f;
	secondT = 0.0f;
	thirdT = 0.0f;
	shouldStartEasing = false;
	currentStage = EasingStage::NOT_STARTED;
	easeStartTimer = kStartTime;
}

void TriForce::ResetProgress()
{
	// すべてのイージング進行度を0にリセット
	firstT = 0.0f;
	secondT = 0.0f;
	thirdT = 0.0f;
	easeStartTimer = kStartTime;

	// 状態をリセット
	shouldStartEasing = false;
	currentStage = EasingStage::NOT_STARTED;

	// 初期化
	for (int i = 0; i < indexTriangularPrism; i++) {
		triangularPrism[i]->SetPosition(firstPosition[i]);
		triangularPrism[i]->SetRotation(firstRotate[i]);
	}
}

void TriForce::Update(const Matrix4x4& viewProjectionMatrix)
{
	if (!shouldStartEasing) {
		easeStartTimer -= (1.0f / 60.0f);

		if (easeStartTimer <= 0.0f) {
			shouldStartEasing = true;
			currentStage = EasingStage::FIRST_STAGE; // 最初から開始
		}
	}

	// イージング処理を実行
	MoveEasing(viewProjectionMatrix);

	for (int i = 0; i < indexTriangularPrism; i++) {
		triangularPrism[i]->Update(viewProjectionMatrix);
	}
}

void TriForce::Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle)
{
	// 三角柱それぞれを描画
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

	switch (currentStage) {
	case EasingStage::FIRST_STAGE:
		///上から画面中央へ落下
		firstT += (1.0f / (60.0f * 15.0f));
		firstT = std::clamp(firstT, 0.0f, 1.0f);

		{
			float firstEaseT = easeOutQuint(firstT);

			for (int i = 0; i < indexTriangularPrism; i++) {
				triangularPrism[i]->SetPosition(Lerp(firstPosition[i], secondPosition[i], firstEaseT));
				triangularPrism[i]->SetRotation(Lerp(firstRotate[i], secondRotate[i], firstEaseT));
				triforceEmitter[i]->Update((1.0f / 60.0f), triangularPrism[i]->GetTransform(), firstEaseT, viewProjection);
			}

			//次の段階へ
			if (firstT >= 1.0f) {
				currentStage = EasingStage::SECOND_STAGE;
			}
		}
		break;

	case EasingStage::SECOND_STAGE:
		///中央から演出の位置へ
		secondT += (1.0f / (60.0f * 20.0f));
		secondT = std::clamp(secondT, 0.0f, 1.0f);

		{
			float secondEaseT = easeInCubic(secondT);

			for (int i = 0; i < indexTriangularPrism; i++) {
				triangularPrism[i]->SetPosition(Lerp(secondPosition[i], thirdPosition[i], secondEaseT));
				triangularPrism[i]->SetRotation(Lerp(secondRotate[i], thirdRotate[i], secondEaseT));

				triforceEmitter[i]->Update((1.0f / 60.0f), triangularPrism[i]->GetTransform(), secondEaseT, viewProjection);
			}


			//次の段階へ
			if (secondT >= 1.0f) {
				currentStage = EasingStage::THIRD_STAGE;
			}
		}
		break;

	case EasingStage::THIRD_STAGE:
		///トライフォース完成
		thirdT += (1.0f / (60.0f * 25.0f));
		thirdT = std::clamp(thirdT, 0.0f, 1.0f);

		{
			float thirdEaseT = easeOutCubic(thirdT);

			for (int i = 0; i < indexTriangularPrism; i++) {
				triangularPrism[i]->SetPosition(Lerp(thirdPosition[i], finalPosition[i], thirdEaseT));
				triangularPrism[i]->SetRotation(Lerp(thirdRotate[i], finalRotate[i], thirdEaseT));

				triforceEmitter[i]->Update((1.0f / 60.0f), triangularPrism[i]->GetTransform(), thirdEaseT, viewProjection);
			}


			//次の段階へ
			if (thirdT >= 1.0f) {
				currentStage = EasingStage::COMPLETED;
			}
		}
		break;

	case EasingStage::COMPLETED:
	{
		// 完成()
		float completedEaseT = 1;

		for (int i = 0; i < indexTriangularPrism; i++) {
			triforceEmitter[i]->Update((1.0f / 60.0f), triangularPrism[i]->GetTransform(), completedEaseT, viewProjection);
		}
	}
	break;

	case EasingStage::NOT_STARTED:
	default:
		// 開始前　
		break;
	}
}