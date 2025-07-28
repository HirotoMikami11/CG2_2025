#include "Engine.h"

Engine* Engine::GetInstance() {
	static Engine instance;
	return &instance;
}

void Engine::Initialize(const std::wstring& title) {
	// ダンプ初期化
	Dump::Initialize();
	// 基礎システムの初期化
	InitializeBase(title);
	// Manager関連の初期化
	InitializeManagers();
	//初期化時にリソースを読み込んでおく
	LoadDefaultResources();
}

void Engine::InitializeBase(const std::wstring& title) {
	// ウィンドウ初期化
	winApp_ = std::make_unique<WinApp>();
	winApp_->Initialize(L"LE2A_15_ミカミ_ヒロト_CG2");

	// ログ初期化
	Logger::Initalize();

	// DirectX初期化
	directXCommon_ = std::make_unique<DirectXCommon>();
	directXCommon_->Initialize(winApp_.get());
}

void Engine::InitializeManagers() {
	// 入力マネージャー初期化
	inputManager_ = InputManager::GetInstance();
	inputManager_->Initialize(winApp_.get());

	// テクスチャマネージャー初期化
	textureManager_ = TextureManager::GetInstance();
	textureManager_->Initialize(directXCommon_.get());

	// オーディオマネージャー初期化
	audioManager_ = AudioManager::GetInstance();
	audioManager_->Initialize();

	// FPSタイマー取得
	frameTimer_ = &FrameTimer::GetInstance();

	// モデルマネージャー初期化
	modelManager_ = ModelManager::GetInstance();
	modelManager_->Initialize(directXCommon_.get());

	// カメラコントローラー取得
	cameraController_ = CameraController::GetInstance();

	// ImGui初期化
	imguiManager_ = ImGuiManager::GetInstance();
	imguiManager_->Initialize(winApp_.get(), directXCommon_.get());

	// オフスクリーンレンダラー初期化
	offscreenRenderer_ = std::make_unique<OffscreenRenderer>();
	offscreenRenderer_->Initialize(directXCommon_.get());
}

void Engine::LoadDefaultResources() {
	///汎用的でエンジン側で先に読み込んでおきたいもののみここで読み込む

	///*-----------------------------------------------------------------------*///
	///								テクスチャの読み込み							///
	///*-----------------------------------------------------------------------*///

	textureManager_->LoadTexture("resources/uvChecker.png", "uvChecker");
	textureManager_->LoadTexture("resources/monsterBall.png", "monsterBall");
	textureManager_->LoadTexture("resources/white2x2.png", "white");

	///*-----------------------------------------------------------------------*///
	///								音声データの読み込み							///
	///*-----------------------------------------------------------------------*///

	//ゲーム開始前に読み込む音声データ
	audioManager_->LoadAudio("resources/Audio/Alarm01.wav", "Alarm");
	audioManager_->LoadAudio("resources/Audio/Bgm01.mp3", "BGM");
	audioManager_->LoadAudio("resources/Audio/Se01.mp3", "SE");

	////tagを利用して再生
	//audioManager_->Play("Alarm");
	//audioManager_->SetVolume("Alarm", 0.1f);	

	//audioManager_->PlayLoop("BGM");
	//audioManager_->SetVolume("BGM", 0.1f);

	//audioManager_->PlayLoop("SE");
	//audioManager_->SetVolume("SE", 0.1f);

	///*-----------------------------------------------------------------------*///
	///								モデルデータの読み込み							///
	///*-----------------------------------------------------------------------*///
	// プリミティブ事前読み込み
	modelManager_->LoadPrimitive(MeshType::SPHERE, "sphere");
	modelManager_->LoadPrimitive(MeshType::TRIANGLE, "triangle");
	modelManager_->LoadPrimitive(MeshType::PLANE, "plane");
}

void Engine::Update() {
	// ウィンドウメッセージの処理
	if (!winApp_->ProsessMessege()) {
		ClosedWindow_ = true; //ウィンドウを閉じる
	}

	// FPS開始
	frameTimer_->BeginFrame();

	// 入力更新
	inputManager_->Update();

	/// ImGuiの受付開始
	imguiManager_->Begin();

	// オフスクリーンレンダラーの更新（エフェクト含む)
	offscreenRenderer_->Update(frameTimer_->GetDeltaTime());
}


void Engine::StartDrawOffscreen() {
	/// ImGuiの受付終了
	imguiManager_->End();

	// フレーム開始
	directXCommon_->BeginFrame();

	/// オフスクリーンの描画準備（3D描画用）
	offscreenRenderer_->PreDraw();
}

void Engine::EndDrawOffscreen() {
	/// オフスクリーンの描画終了
	offscreenRenderer_->PostDraw();
}


void Engine::StartDrawBackBuffer() {

	// 通常描画の描画準備（バックバッファ描画開始）
	directXCommon_->PreDraw();

	// オフスクリーンの画面の実態描画
	offscreenRenderer_->DrawOffscreenTexture();
}


void Engine::EndDrawBackBuffer() {
	// ImGuiの画面への描画
	imguiManager_->Draw(directXCommon_->GetCommandList());

	// 通常描画の終わり
	directXCommon_->PostDraw();

	// 描画そのもののEndFrame
	directXCommon_->EndFrame();
}

void Engine::Finalize() {
	// ImGui終了処理
	if (imguiManager_) {
		imguiManager_->Finalize();
	}

	// オフスクリーンレンダラー終了処理
	if (offscreenRenderer_) {
		offscreenRenderer_->Finalize();
		offscreenRenderer_.reset();
	}

	// オーディオ終了処理
	if (audioManager_) {
		audioManager_->Finalize();
	}

	// 入力終了処理
	if (inputManager_) {
		inputManager_->Finalize();
	}

	// モデル終了処理
	if (modelManager_) {
		modelManager_->Finalize();
	}

	// テクスチャ終了処理
	if (textureManager_) {
		textureManager_->Finalize();
	}

	// ウィンドウ終了処理
	if (winApp_) {
		winApp_->Finalize();
		winApp_.reset();
	}

	// DirectX終了処理
	if (directXCommon_) {
		directXCommon_->Finalize();
		directXCommon_.reset();
	}

	// COM終了処理
	CoUninitialize();
}

void Engine::ImGui() {
#ifdef _DEBUG

	//開発用UIの処理
	ImGui::Begin("Engine_data");

	//FPS関連
	frameTimer_->ImGui();

	/// オフスクリーンレンダラー（グリッチエフェクト含む）のImGui
	offscreenRenderer_->ImGui();

	///入力のImGui
	inputManager_->ImGui();

	///音声関連のImGui
	audioManager_->ImGui();

	ImGui::End();



	///
	/// カメラコントローラーのデバッグUI
	///
	
	///個別のウィンドウに表示
	if (cameraController_) {
		//カメラコントローラーのアップデートがないとエラー
		cameraController_->ImGui();
	}

#endif
}