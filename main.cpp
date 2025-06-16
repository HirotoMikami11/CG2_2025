#include <Windows.h>
#include<cstdint>
#include<string>
#include<format>
//ファイルやディレクトリに関する操作を行うライブラリ
#include<filesystem>
//ファイルに書いたり読んだりするライブラリ
#include<fstream>
#include<sstream>
#include <iostream>//すぐ消す
//時間を扱うライブラリ
#include<chrono>

///DirectX12
#include<d3d12.h>
#pragma comment(lib,"d3d12.lib")
#include<dxgi1_6.h>
#pragma comment(lib,"dxgi.lib")
#include <dxgidebug.h>
#pragma comment(lib,"dxguid.lib")

///DXC
#include <dxcapi.h>
#pragma comment(lib,"dxcompiler.lib")

///DirectXTex
#include"externals/DirectXTex/DirectXTex.h"
#include"externals/DirectXTex/d3dx12.h"

//comptr
#include<wrl.h>

#include<cassert>
#include "MyFunction.h"

#include "Logger.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "Dump.h"

#include "AudioManager.h"
#include "InputManager.h"
#include "TextureManager.h"
#include "ImGuiManager.h" 

#include"Transform.h"
///マテリアルとメッシュを統合させたmodelクラス
#include"Model.h"
///トランスフォームと、モデルを統合させたGameObjectクラス
#include "GameObject.h"

//ライト
#include"Light.h"


//カメラ系統
#include"CameraController.h"

//オフスクリーン
#include "OffscreenRenderer.h"
//FPS関連
#include "FrameTimer.h"

/// <summary>
/// deleteの前に置いておく、infoの警告消すことで、リークの種類を判別できる
/// </summary>
void DumpLiveObjects() {
	Microsoft::WRL::ComPtr<IDXGIDebug1> debug;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
	}
}

/// <summary>
/// リソースリークチェック
/// </summary>
struct D3DResourceLeakChecker {
	~D3DResourceLeakChecker() {
		//リソースリークチェック
		Microsoft::WRL::ComPtr <IDXGIDebug1> debug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
			debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);

		}

	}

};

///*-----------------------------------------------------------------------*///
//																			//
///									メイン関数							   ///
//																			//
///*-----------------------------------------------------------------------*///
// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	D3DResourceLeakChecker leakCheak;

	//誰も補足しなかった場合に（Unhandled）、補足する関数を登録（main関数が始まってすぐ）
	Dump::Initialize();


	WinApp* winApp = new WinApp;
	DirectXCommon* directXCommon = new DirectXCommon;

	/// ウィンドウクラスを登録する
	winApp->Initialize();
	///	ログをファイルに書き込む
	Logger::Initalize();

	///DirectX初期化
	directXCommon->Initialize(winApp);

	//DirectXの末尾にキーボード入力のインスタンス生成
	InputManager* inputManager = InputManager::GetInstance();
	inputManager->Initialize(winApp);

	//DirectXの末尾にテクスチャ読み込みのインスタンス生成
	TextureManager* textureManager = TextureManager::GetInstance();
	textureManager->Initialize(directXCommon);

	//DirectX初期化の末尾にXAudio2エンジンのインスタンス生成
	AudioManager* audioManager = AudioManager::GetInstance();
	audioManager->Initialize();

	// FPS関連
	FrameTimer& frameTimer = FrameTimer::GetInstance();
	///*-----------------------------------------------------------------------*///
	///								テクスチャの読み込み							///
	///*-----------------------------------------------------------------------*///

	textureManager->LoadTexture("resources/uvChecker.png", "uvChecker");
	textureManager->LoadTexture("resources/monsterBall.png", "monsterBall");


	///*-----------------------------------------------------------------------*///
	///								音声データの読み込み							///
	///*-----------------------------------------------------------------------*///

	//ゲーム開始前に読み込む音声データ
	//audioManager->LoadAudio("resources/Audio/Alarm01.wav", "Alarm");
	//audioManager->LoadAudio("resources/Audio/Bgm01.mp3", "BGM");
	//audioManager->LoadAudio("resources/Audio/Se01.mp3", "SE");

	////tagを利用して再生
	//audioManager->Play("Alarm");
	//audioManager->SetVolume("Alarm", 0.1f);	

	//audioManager->PlayLoop("BGM");
	//audioManager->SetVolume("BGM", 0.1f);

	//audioManager->PlayLoop("SE");
	//audioManager->SetVolume("SE", 0.1f);

	///*-----------------------------------------------------------------------*///
	///								カメラの初期化									///
	///*-----------------------------------------------------------------------*///

	CameraController* cameraController = CameraController::GetInstance();
	cameraController->Initialize();


	// オフスクリーンレンダラーの初期化（DirectX初期化後に追加）
	std::unique_ptr<OffscreenRenderer> offscreenRenderer = std::make_unique<OffscreenRenderer>();
	offscreenRenderer->Initialize(directXCommon);

	///*-----------------------------------------------------------------------*///
	///									三角形									///
	///*-----------------------------------------------------------------------*///

#pragma region Triangle

	//Transform変数を作る
	Vector3Transform transformTriangle{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{2.0f,1.5f,0.0f}
	};


	std::unique_ptr<Triangle> triangle[2];								// 三角形の配列を宣言
	const int kMaxTriangleIndex = 2;									//三角形の最大数

	for (int i = 0; i < kMaxTriangleIndex; i++) {
		triangle[i] = std::make_unique<Triangle>();						// 三角形を生成
		triangle[i]->Initialize(directXCommon, "uvChecker");			// 初期化
		triangle[i]->SetTransform(transformTriangle);					// Transformを設定
	}

#pragma endregion

	///*-----------------------------------------------------------------------*///
	///									球体										///
	///*-----------------------------------------------------------------------*///

#pragma region Sphere

	//SphereのTransform変数を作る
	Vector3Transform transformSphere{
	{1.0f,1.0f,1.0f},
	{0.0f,0.0f,0.0f},
	{0.0f,0.0f,0.0f}
	};

	std::unique_ptr<Sphere> sphere = std::make_unique<Sphere>();				// 球体を生成
	sphere->Initialize(directXCommon, "monsterBall");						// 初期化
	sphere->SetTransform(transformSphere);									// Transformを設定


#pragma endregion

	///*-----------------------------------------------------------------------*///
	///								矩形Sprite									///
	///*-----------------------------------------------------------------------*///

	//TODO: spriteはのちにGameObjectから独立させる


#pragma region "Sprite"

	//SpriteのTransform変数を作る
	Vector3Transform transformSprite{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};

	std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>();			// スプライトを生成
	sprite->Initialize(directXCommon, "uvChecker");							// 初期化
	sprite->SetTransform(transformSprite);									// Transformを設定


#pragma endregion

	///*-----------------------------------------------------------------------*///
	///									model									///
	///*-----------------------------------------------------------------------*///


#pragma region "Model"

	// モデル用のTransform変数
	Vector3Transform transformModel{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 3.0f, 0.0f},
		{-2.2f,-1.2f, 0.0f}
	};

	std::unique_ptr<Model3D> model = std::make_unique<Model3D>();			// スプライトを生成
	model->Initialize(directXCommon, "resources", "plane.obj");				// 初期化
	model->SetTransform(transformModel);									// Transformを設定


#pragma endregion




	///*-----------------------------------------------------------------------*///
	///									ライト									///
	///*-----------------------------------------------------------------------*///

	Light directionalLight;
	directionalLight.Initialize(directXCommon, Light::Type::DIRECTIONAL);

	///*-----------------------------------------------------------------------*///
	//																			//
	///								ImGuiの初期化								   ///
	//																			//
	///*-----------------------------------------------------------------------*///

	// ImGuiの初期化
	ImGuiManager* imguiManager = ImGuiManager::GetInstance();
	imguiManager->Initialize(winApp, directXCommon);


	//								　変数宣言									//


	//ImGuiで使用する変数
	bool useMonsterBall = true;
	///*-----------------------------------------------------------------------*///
	//																			//
	///									メインループ							   ///
	//																			//
	///*-----------------------------------------------------------------------*///


	//ウィンドウのxボタンが押されるまでループ
	while (winApp->ProsessMessege()) {
		//							　ゲームの処理										//
		///FPSの開始
		frameTimer.BeginFrame();

		///キー入力の更新
		inputManager->Update();
		//								更新処理										//


		// ImGuiの受け付け開始
		imguiManager->Begin();

		//開発用UIの処理
		ImGui::Begin("Debug");

		//FPS関連
		frameTimer.ImGui();

		///三角形のImgui
		for (int i = 0; i < kMaxTriangleIndex; i++)
		{
			triangle[i]->ImGui();
		}
		/// 球体のImgui
		sphere->ImGui();
		/// スプライトのImgui
		sprite->ImGui();
		/// モデルのImgui
		model->ImGui();
		//ライト
		directionalLight.ImGui("DriectonalLight");

		/// オフスクリーンレンダラー（グリッチエフェクト含む）のImGui
		//offscreenRenderer->ImGui();

		ImGui::End();

		//																			//
		//							カメラの更新										//
		//																			//

		cameraController->Update();

		//																			//
		//								更新処理										//
		//																			//


		// 三角形を回転させる
		triangle[0]->AddRotation({ 0.0f, 0.03f, 0.0f });


		//球体を回転させる
		//isActiveの判定は行列更新にしかかかってないので現状isAciveは死んでる
		sphere->AddRotation({ 0.0f,0.01f,0.0f });


		// オフスクリーンレンダラーの更新（エフェクト含む
		offscreenRenderer->Update();


		//																			//
		//								行列更新										//
		//																			//

		//三角形の更新(現状行列更新のみ)
		for (int i = 0; i < kMaxTriangleIndex; i++)
		{
			triangle[i]->Update(cameraController->GetViewProjectionMatrix());
		}

		//球体の更新(現状行列更新のみ)
		sphere->Update(cameraController->GetViewProjectionMatrix());
		//スプライトの更新(現状行列更新のみ)
		sprite->Update(cameraController->GetViewProjectionMatrixSprite());
		//プレーンモデルの更新(現状行列更新のみ)
		model->Update(cameraController->GetViewProjectionMatrix());

		// ImGuiの受け付け終了
		imguiManager->End();

		//								描画処理										//
		///*-----------------------------------------------------------------------*///
		//																			//
		///				画面をクリアする処理が含まれたコマンドリストを作る				   ///
		//																			//
		///*-----------------------------------------------------------------------*///


		directXCommon->BeginFrame();

		///																			///
		///						オフスクリーンレンダリング								///
		///																			///
		//// オフスクリーンの描画準備
		offscreenRenderer->PreDraw();

		// Sphereの描画
		sphere->Draw(directionalLight);
		for (int i = 0; i < kMaxTriangleIndex; i++) {
			triangle[i]->Draw(directionalLight);
		}
		model->Draw(directionalLight);
		sprite->DrawSprite(directionalLight);



		//// オフスクリーンの描画終了
		offscreenRenderer->PostDraw();
		///																			///
		///								通常レンダリング								///
		///																			///

		// 通常描画の描画準備
		directXCommon->PreDraw();
		///通常描画

		// オフスクリーンの画面の実態描画
		offscreenRenderer->DrawOffscreenTexture();




		// ImGuiの画面への描画
		imguiManager->Draw(directXCommon->GetCommandList());

		// 通常描画の描画終わり
		directXCommon->PostDraw();

		// 描画そのもののEndFrame
		directXCommon->EndFrame();
	}


	// ImGuiの終了処理
	imguiManager->Finalize();

	///*-----------------------------------------------------------------------*///
	//																			//
	///							ReportLiveObjects							   ///
	//																			//
	///*-----------------------------------------------------------------------*///

	offscreenRenderer->Finalize();
	// Audioの終了処理
	audioManager->Finalize();
	//inputの終了処理(中身は何もない)
	inputManager->Finalize();
	//textureの終了処理
	textureManager->Finalize();


	// winAppの終了処理
	winApp->Finalize();
	delete winApp;
	// directXの終了処理
	directXCommon->Finalize();
	delete directXCommon;


	//COMの終了処理
	CoUninitialize();

	return 0;


}