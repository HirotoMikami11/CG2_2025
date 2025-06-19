#include "GameScene.h"

GameScene::GameScene()
	: cameraController_(nullptr)
	, directXCommon_(nullptr)
	, offscreenRenderer_(nullptr) {
}

GameScene::~GameScene() = default;

void GameScene::Initialize() {
	// システム参照の取得
	directXCommon_ = Engine::GetInstance()->GetDirectXCommon();
	offscreenRenderer_ = Engine::GetInstance()->GetOffscreenRenderer();

	///*-----------------------------------------------------------------------*///
	///								カメラの初期化									///
	///*-----------------------------------------------------------------------*///
	cameraController_ = CameraController::GetInstance();
	cameraController_->Initialize();

	// ゲームオブジェクト初期化
	InitializeGameObjects();


}


void GameScene::InitializeGameObjects() {
	///*-----------------------------------------------------------------------*///
	///									三角形									///
	///*-----------------------------------------------------------------------*///
	Vector3Transform transformTriangle{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{2.0f, 1.5f, 0.0f}
	};

	for (int i = 0; i < kMaxTriangleIndex; i++) {
		triangles_[i] = std::make_unique<Triangle>();
		triangles_[i]->Initialize(directXCommon_, "uvChecker");
		triangles_[i]->SetTransform(transformTriangle);
	}

	///*-----------------------------------------------------------------------*///
	///									球体										///
	///*-----------------------------------------------------------------------*///
	Vector3Transform transformSphere{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f}
	};

	sphere_ = std::make_unique<Sphere>();
	sphere_->Initialize(directXCommon_, "monsterBall");
	sphere_->SetTransform(transformSphere);

	///*-----------------------------------------------------------------------*///
	///									model									///
	///*-----------------------------------------------------------------------*///
	Vector3Transform transformModel{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 3.0f, 0.0f},
		{-2.2f, -1.2f, 0.0f}
	};

	model_ = std::make_unique<Model3D>();
	model_->Initialize(directXCommon_, "resources", "plane.obj");
	model_->SetTransform(transformModel);

	///*-----------------------------------------------------------------------*///
	///								矩形Sprite									///
	///*-----------------------------------------------------------------------*///
	sprite_ = std::make_unique<Sprite>();
	sprite_->Initialize(directXCommon_, "uvChecker", { 50, 50 }, { 100, 100 });

	///*-----------------------------------------------------------------------*///
	///									ライト									///
	///*-----------------------------------------------------------------------*///
	directionalLight_.Initialize(directXCommon_, Light::Type::DIRECTIONAL);
}


void GameScene::Update() {
	// カメラ更新
	cameraController_->Update();

	// ゲームオブジェクト更新
	UpdateGameObjects();


	// ImGui更新
	ImGui::Begin("Debug");

	// 三角形のImGui
	for (int i = 0; i < kMaxTriangleIndex; i++) {
		triangles_[i]->ImGui();
	}

	// 球体のImGui
	sphere_->ImGui();

	// スプライトのImGui
	sprite_->ImGui();

	// モデルのImGui
	model_->ImGui();

	// ライトのImGui
	directionalLight_.ImGui("DirectionalLight");

	ImGui::End();
}

void GameScene::UpdateGameObjects() {
	// 三角形を回転させる
	triangles_[0]->AddRotation({ 0.0f, 0.03f, 0.0f });

	// 球体を回転させる
	sphere_->AddRotation({ 0.0f, 0.01f, 0.0f });

	// 行列更新
	viewProjectionMatrix = cameraController_->GetViewProjectionMatrix();
	viewProjectionMatrixSprite = cameraController_->GetViewProjectionMatrixSprite();

	// 三角形の更新
	for (int i = 0; i < kMaxTriangleIndex; i++) {
		triangles_[i]->Update(viewProjectionMatrix);
	}

	// 球体の更新
	sphere_->Update(viewProjectionMatrix);

	// スプライトの更新
	sprite_->Update(viewProjectionMatrixSprite);

	// モデルの更新
	model_->Update(viewProjectionMatrix);
}

void GameScene::Draw() {
	// ゲームオブジェクトの描画（オフスクリーンに描画）
	DrawGameObjects();
}

void GameScene::DrawGameObjects() {
	// 球体の描画
	sphere_->Draw(directionalLight_);

	// 三角形の描画
	for (int i = 0; i < kMaxTriangleIndex; i++) {
		triangles_[i]->Draw(directionalLight_);
	}

	// モデルの描画
	model_->Draw(directionalLight_);

	// スプライトの描画
	sprite_->Draw();
}

void GameScene::Finalize() {
	// unique_ptrで自動的に解放される
}