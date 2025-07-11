#include "DemoScene.h"
#include "Managers/ImGuiManager.h" 

DemoScene::DemoScene()
	: BaseScene("DemoScene") // シーン名を設定
	, cameraController_(nullptr)
	, directXCommon_(nullptr)
	, offscreenRenderer_(nullptr)
	, modelManager_(nullptr)
	, textureManager_(nullptr)
	, viewProjectionMatrix{ MakeIdentity4x4() }
	, viewProjectionMatrixSprite{ MakeIdentity4x4() }
{
}

DemoScene::~DemoScene() = default;


void DemoScene::LoadResources() {
	// リソースの読み込み
	Logger::Log(Logger::GetStream(), "TitleScene: Loading resources...\n");

	// リソースマネージャーの取得
	modelManager_ = ModelManager::GetInstance();
	textureManager_ = TextureManager::GetInstance();

	// モデルを事前読み込み
	modelManager_->LoadModel("resources/Model/Plane", "plane.obj", "plane");

	//TODO:スザンヌ

	//バニー

	//ティーカップ


	Logger::Log(Logger::GetStream(), "TitleScene: Resources loaded successfully\n");


}


void DemoScene::Initialize() {
	// システム参照の取得
	directXCommon_ = Engine::GetInstance()->GetDirectXCommon();
	offscreenRenderer_ = Engine::GetInstance()->GetOffscreenRenderer();
	///*-----------------------------------------------------------------------*///
	///								カメラの初期化									///
	///*-----------------------------------------------------------------------*///
	cameraController_ = CameraController::GetInstance();
	// 座標と回転を指定して初期化
	Vector3 initialPosition = { 0.0f, 3.0f, -15.0f };
	Vector3 initialRotation = { 0.25f, 0.0f, 0.0f };
	cameraController_->Initialize(initialPosition, initialRotation);
	cameraController_->SetActiveCamera("normal");

	// ゲームオブジェクト初期化
	InitializeGameObjects();
}

void DemoScene::InitializeGameObjects() {
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
		triangles_[i]->Initialize(directXCommon_, "triangle", "uvChecker");
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
	sphere_->Initialize(directXCommon_, "sphere", "monsterBall");
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
	model_->Initialize(directXCommon_, "plane");
	model_->SetTransform(transformModel);


	///*-----------------------------------------------------------------------*///
	///								グリッド線									///
	///*-----------------------------------------------------------------------*///

	// グリッド
	gridLine_ = std::make_unique<GridLine>();
	// 100m、1m間隔、10mごとに黒
	gridLine_->Initialize(directXCommon_,
		GridLineType::XZ,			// グリッドタイプ：XZ平面
		100.0f,						// サイズ
		1.0f,						// 間隔
		10.0f,						// 主要線間隔
		{ 0.5f, 0.5f, 0.5f, 1.0f },	// 通常線：灰色
		{ 0.0f, 0.0f, 0.0f, 1.0f }	// 主要線：黒
	);
	gridLine_->SetName("Main Grid");



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

void DemoScene::Update() {
	// カメラ更新
	cameraController_->Update();

	// ゲームオブジェクト更新
	UpdateGameObjects();
}

void DemoScene::UpdateGameObjects() {
	// 三角形を回転させる
	triangles_[0]->AddRotation({ 0.0f, 0.02f, 0.0f });

	// 球体を回転させる
	sphere_->AddRotation({ 0.0f, 0.015f, 0.0f });

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

	// グリッド線更新
	gridLine_->Update(viewProjectionMatrix);

}

void DemoScene::Draw() {
	// グリッド線を描画
	gridLine_->Draw(viewProjectionMatrix);


	// ゲームオブジェクトの描画（オフスクリーンに描画）
	DrawGameObjects();
}

void DemoScene::DrawGameObjects() {
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

void DemoScene::OnEnter() {
	// デモシーンに入る時の処理
	//TODO: 現状シーン移動時、シーンを解放するので、初期化と同等の処理になっている。
}

void DemoScene::OnExit() {
	// デモシーンから出る時の処理
}

void DemoScene::ImGui() {
#ifdef _DEBUG
	// 三角形のImGui
	ImGui::Text("Triangle");
	for (int i = 0; i < kMaxTriangleIndex; i++) {
		ImGui::PushID(i);
		triangles_[i]->ImGui();
		ImGui::PopID();
	}

	ImGui::Spacing();

	// 球体のImGui
	ImGui::Text("Sphere");
	sphere_->ImGui();

	ImGui::Spacing();

	// スプライトのImGui
	ImGui::Text("Sprite");
	sprite_->ImGui();

	ImGui::Spacing();

	// モデルのImGui
	ImGui::Text("Model");
	model_->ImGui();

	ImGui::Spacing();

	// ライトのImGui
	ImGui::Text("Lighting");
	directionalLight_.ImGui("DirectionalLight");

#endif
}

void DemoScene::Finalize() {
	// unique_ptrで自動的に解放される
}