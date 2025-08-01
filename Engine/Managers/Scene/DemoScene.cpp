#include "DemoScene.h"
#include "Managers/ImGui/ImGuiManager.h" 

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
	modelManager_->LoadModel("resources/Model/Plane", "plane.obj", "model_Plane");

	//TODO:スザンヌ
	//modelManager_->LoadModel("resources/Model/Suzanne", "suzanne.obj", "model_Suzanne");

	//バニー
	modelManager_->LoadModel("resources/Model/Bunny", "bunny.obj", "model_Bunny");
	//ティーポット
	modelManager_->LoadModel("resources/Model/Teapot", "teapot.obj", "model_Teapot");

	//マルチメッシュ
	modelManager_->LoadModel("resources/Model/MultiMesh", "multiMesh.obj", "model_MultiMesh");
	//マルチマテリアル
	modelManager_->LoadModel("resources/Model/MultiMaterial", "multiMaterial.obj", "model_MultiMaterial");


	Logger::Log(Logger::GetStream(), "TitleScene: Resources loaded successfully\n");
}


void DemoScene::ConfigureOffscreenEffects()
{
	/// オフスクリーンレンダラーのエフェクト設定

	// 全てのエフェクトを無効化
	offscreenRenderer_->DisableAllEffects();


	auto* depthFogEffect = offscreenRenderer_->GetDepthFogEffect();
	if (depthFogEffect) {
		depthFogEffect->SetEnabled(true);
		depthFogEffect->SetFogDistance(0.2f, 30.0f); // 深度フォグの距離を設定
	}
	auto* depthOfFieldEffect = offscreenRenderer_->GetDepthOfFieldEffect();
	if (depthOfFieldEffect) {
		depthOfFieldEffect->SetEnabled(true);
	}
	auto* vignetteEffect = offscreenRenderer_->GetVignetteEffect();
	if (vignetteEffect) {
		vignetteEffect->SetEnabled(true);
	}

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
	Vector3 initialPosition = { 0.0f, 6.8f, -18.0f };
	Vector3 initialRotation = { 0.4f, 0.0f, 0.0f };
	cameraController_->Initialize(initialPosition, initialRotation);
	cameraController_->SetActiveCamera("normal");

	// ゲームオブジェクト初期化
	InitializeGameObjects();

	// ポストエフェクトの初期設定
	ConfigureOffscreenEffects();
}

void DemoScene::InitializeGameObjects() {

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
		{0.0f, 3.14f, 0.0f},
		{0.0f, 1.0f, 2.0f}
	};

	model_ = std::make_unique<Model3D>();
	model_->Initialize(directXCommon_, "model_Plane");
	model_->SetTransform(transformModel);

	///*-----------------------------------------------------------------------*///
	///									Teapot									///
	///*-----------------------------------------------------------------------*///
	Vector3Transform transformTeapot{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{-3.7f, 0.75f, -5.5f}
	};

	modelTeapot_ = std::make_unique<Model3D>();
	modelTeapot_->Initialize(directXCommon_, "model_Teapot");
	modelTeapot_->SetTransform(transformTeapot);

	///*-----------------------------------------------------------------------*///
	///									Bunny									///
	///*-----------------------------------------------------------------------*///
	Vector3Transform transformBunny{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 3.0f, 0.0f},
		{3.7f, 0.0f, -5.5f}
	};

	modelBunny_ = std::make_unique<Model3D>();
	modelBunny_->Initialize(directXCommon_, "model_Bunny");
	modelBunny_->SetTransform(transformBunny);

	///*-----------------------------------------------------------------------*///
	///								MultiMesh									///
	///*-----------------------------------------------------------------------*///
	Vector3Transform transformMultiMesh{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 3.0f, 0.0f},
		{-5.31f, -0.3f, 3.7f}
	};

	modelMultiMesh_ = std::make_unique<Model3D>();
	modelMultiMesh_->Initialize(directXCommon_, "model_MultiMesh");
	modelMultiMesh_->SetTransform(transformMultiMesh);

	///*-----------------------------------------------------------------------*///
	///								MultiMaterial								///
	///*-----------------------------------------------------------------------*///
	Vector3Transform transformMultiMaterial{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 3.0f, 0.0f},
		{2.23f, -0.3f, 3.7f}
	};

	modelMultiMaterial_ = std::make_unique<Model3D>();
	modelMultiMaterial_->Initialize(directXCommon_, "model_MultiMaterial");
	modelMultiMaterial_->SetTransform(transformMultiMaterial);


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


	// 球体を回転させる
	sphere_->AddRotation({ 0.0f, 0.015f, 0.0f });

	// 行列更新
	viewProjectionMatrix = cameraController_->GetViewProjectionMatrix();
	viewProjectionMatrixSprite = cameraController_->GetViewProjectionMatrixSprite();



	// 球体の更新
	sphere_->Update(viewProjectionMatrix);

	
	uvTransformX += 0.01f; // UV変換のY座標を更新 
	sprite_->SetUVTransformTranslate({ uvTransformX, 0.0f });
	// スプライトの更新
	sprite_->Update(viewProjectionMatrixSprite);
	// モデルの更新
	model_->Update(viewProjectionMatrix);
	// ティーポットモデルの更新
	modelTeapot_->Update(viewProjectionMatrix);
	//バニーモデルの更新
	modelBunny_->Update(viewProjectionMatrix);
	//マルチメッシュモデルの更新
	modelMultiMesh_->Update(viewProjectionMatrix);
	//マルチマテリアルモデルの更新
	modelMultiMaterial_->Update(viewProjectionMatrix);

	// グリッド線更新
	gridLine_->Update(viewProjectionMatrix);
}

void DemoScene::DrawOffscreen() {
	// グリッド線を描画（3D要素）
	gridLine_->Draw(viewProjectionMatrix);

	// 3Dゲームオブジェクトの描画（オフスクリーンに描画）
	DrawGameObjects();
}

void DemoScene::DrawBackBuffer() {
	// UI(スプライトなど)の描画（オフスクリーン外に描画）
	sprite_->Draw();
}

void DemoScene::DrawGameObjects() {
	// 球体の描画
	sphere_->Draw(directionalLight_);
	// モデルの描画
	model_->Draw(directionalLight_);
	//ティーポットモデルの描画
	modelTeapot_->Draw(directionalLight_);
	//バニーモデルの描画
	modelBunny_->Draw(directionalLight_);
	//マルチメッシュモデルの描画
	modelMultiMesh_->Draw(directionalLight_);
	//マルチマテリアルモデルの描画
	modelMultiMaterial_->Draw(directionalLight_);
}

void DemoScene::ImGui() {
#ifdef _DEBUG

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
	// ティーポットモデルのImGui
	ImGui::Text("ModelTeapot");
	modelTeapot_->ImGui();

	ImGui::Spacing();
	// バニーモデルのImGui
	ImGui::Text("ModelBunny");
	modelBunny_->ImGui();

	ImGui::Spacing();
	// マルチメッシュモデルのImGui
	ImGui::Text("ModelMultiMesh");
	modelMultiMesh_->ImGui();

	ImGui::Spacing();
	// マルチマテリアルモデルのImGui
	ImGui::Text("ModelMultiMaterial");
	modelMultiMaterial_->ImGui();

	ImGui::Spacing();
	// ライトのImGui
	ImGui::Text("Lighting");
	directionalLight_.ImGui("DirectionalLight");

#endif
}

void DemoScene::Finalize() {
	// unique_ptrで自動的に解放される
}