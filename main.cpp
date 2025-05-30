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

//ライト
#include"Light.h"


//カメラ系統
#include"CameraController.h"


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



	///*-----------------------------------------------------------------------*///
	///								テクスチャの読み込み							///
	///*-----------------------------------------------------------------------*///

	textureManager->LoadTexture("resources/uvChecker.png", "uvChecker");
	textureManager->LoadTexture("resources/monsterBall.png", "monsterBall");


	///*-----------------------------------------------------------------------*///
	///								音声データの読み込み							///
	///*-----------------------------------------------------------------------*///

	////ゲーム開始前に読み込む音声データ
	//audioManager->LoadWave("resources/Audio/Alarm01.wav", "Alarm");
	//audioManager->LoadMP3("resources/Audio/Bgm01.mp3", "BGM");
	//audioManager->LoadMP3("resources/Audio/Se01.mp3", "SE");

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

	

	///*-----------------------------------------------------------------------*///
	///									三角形									///
	///*-----------------------------------------------------------------------*///

#pragma region Triangle

	//																			//
	//						メッシュとマテリアルリソースの作成						//
	//																			//

	Model triangleModel;
	triangleModel.Initialize(directXCommon, MeshType::TRIANGLE);
	//																			//
	//					TransformationMatrix用のリソースを作る						//
	//																			//

	//Transform変数を作る
	Vector3Transform transformTriangle{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{2.0f,1.5f,0.0f}
	};

	Transform triangleTransform;
	triangleTransform.Initialize(directXCommon);
	triangleTransform.SetTransform(transformTriangle);


#pragma endregion

	///*-----------------------------------------------------------------------*///
	///									球体										///
	///*-----------------------------------------------------------------------*///

#pragma region Sphere

	//																			//
	//						メッシュとマテリアルリソースの作成						//
	//																			//

	Model sphereModel;
	sphereModel.Initialize(directXCommon, MeshType::SPHERE);
	sphereModel.GetMaterial().SetLightingEnable(true);

	//																			//
	//					TransformationMatrix用のリソースを作る						//
	//																			//

	//SphereのTransform変数を作る
	Vector3Transform transformSphere{
	{1.0f,1.0f,1.0f},
	{0.0f,0.0f,0.0f},
	{0.0f,0.0f,0.0f}
	};

	Transform sphereTransform;
	sphereTransform.Initialize(directXCommon);
	sphereTransform.SetTransform(transformSphere);

	


#pragma endregion

	///*-----------------------------------------------------------------------*///
	///								矩形Sprite									///
	///*-----------------------------------------------------------------------*///

	//TODO: spriteはのちにGameObjectから独立させる
	//TODO: uvTransformもspriteのみ、


#pragma region "Sprite"

	//																			//
	//						メッシュとマテリアルリソースの作成						//
	//																			//

	Model spriteModel;
	spriteModel.Initialize(directXCommon, MeshType::SPRITE);

	//																			//
	//					TransformationMatrix用のリソースを作る						//
	//																			//
	//SpriteのTransform変数を作る
	Vector3Transform transformSprite{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};

	Transform spriteTransform;
	spriteTransform.Initialize(directXCommon);
	spriteTransform.SetTransform(transformSprite);


#pragma endregion

	///*-----------------------------------------------------------------------*///
	///									model									///
	///*-----------------------------------------------------------------------*///

#pragma region "Model"

	//																			//
	//						メッシュとマテリアルリソースの作成						//
	//																			//

	Model modelModel;
	modelModel.Initialize(directXCommon, MeshType::MODEL_OBJ, "resources", "plane.obj");

	//																			//
	//					TransformationMatrix用のリソースを作る						//
	//																			//

	// モデル用のTransform変数
	Vector3Transform transformModel{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 3.0f, 0.0f},
		{-2.2f,-1.2f, 0.0f}
	};

	//Transform変数を作る
	Transform modelTransform;
	modelTransform.Initialize(directXCommon);
	modelTransform.SetTransform(transformModel);


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

	Vector3Transform uvTransformSprite{
	{1.0f,1.0f,1.0f},
	{0.0f,0.0f,0.0f},
	{0.0f,0.0f,0.0f}
	};





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

		///キー入力の更新
		inputManager->Update();


		//if (inputManager->IsKeyTrigger(DIK_1)) {
		//	OutputDebugStringA("Trigger 1!!\n");
		//}

		//if (inputManager->IsMouseButtonDown(0)) {
		//	OutputDebugStringA("Left!!\n");
		//}

		//if (inputManager->IsMoveMouseWheel()) {
		//	std::string str = std::to_string(inputManager->GetMouseWheel());
		//	OutputDebugStringA(str.c_str());
		//}


		//								更新処理										//


		// ImGuiの受け付け開始
		imguiManager->Begin();

		//開発用UIの処理
		ImGui::Begin("Debug");

#pragma region "Triangle"
		ImGui::Text("Triangle");
		Vector4 triangleColor = triangleModel.GetMaterial().GetColor();
		if (ImGui::ColorEdit4("Triangle_Color", reinterpret_cast<float*>(&triangleColor.x))) {
			triangleModel.GetMaterial().SetColor(triangleColor);
		}

		Vector3 triangleTranslate = triangleTransform.GetTranslate();
		if (ImGui::DragFloat3("Triangle_translate", &triangleTranslate.x, 0.01f)) {
			triangleTransform.SetTranslate(triangleTranslate);
		}

		Vector3 triangleRotate = triangleTransform.GetRotate();
		if (ImGui::DragFloat3("Triangle_rotate", &triangleRotate.x, 0.01f)) {
			triangleTransform.SetRotate(triangleRotate);
		}

		bool triangleLighting = triangleModel.GetMaterial().IsLightingEnabled();
		if (ImGui::Checkbox("triangle_useEnableLighting", &triangleLighting)) {
			triangleModel.GetMaterial().SetLightingEnable(triangleLighting);
		}
		bool triangleLambertian = triangleModel.GetMaterial().IsLambertianReflectanceEnabled();
		if (ImGui::Checkbox("triangle_useLambertianReflectance", &triangleLambertian)) {
			triangleModel.GetMaterial().SetLambertianReflectance(triangleLambertian);
		}


#pragma endregion

#pragma region "Sphere"
		ImGui::Text("Sphere");
		Vector4 sphereColor = sphereModel.GetMaterial().GetColor();
		if (ImGui::ColorEdit4("Sphere_Color", reinterpret_cast<float*>(&sphereColor.x))) {
			sphereModel.GetMaterial().SetColor(sphereColor);
		}
		Vector3 sphereTranslate = sphereTransform.GetTranslate();
		if (ImGui::DragFloat3("Sphere_translate", &sphereTranslate.x, 0.01f)) {
			sphereTransform.SetTranslate(sphereTranslate);
		}
		Vector3 sphereRotate = sphereTransform.GetRotate();
		if (ImGui::DragFloat3("Sphere_rotate", &sphereRotate.x, 0.01f)) {
			sphereTransform.SetRotate(sphereRotate);
		}
	

		ImGui::Checkbox("useMonsterBall", &useMonsterBall);

		bool sphereLighting = sphereModel.GetMaterial().IsLightingEnabled();
		if (ImGui::Checkbox("Sphere_useEnableLighting", &sphereLighting)) {
			sphereModel.GetMaterial().SetLightingEnable(sphereLighting);
		}
		bool sphereLambertian = sphereModel.GetMaterial().IsLambertianReflectanceEnabled();
		if (ImGui::Checkbox("Sphere_useLambertianReflectance", &sphereLambertian)) {
			sphereModel.GetMaterial().SetLambertianReflectance(sphereLambertian);
		}

#pragma endregion

#pragma region Sprite
		ImGui::Text("Sprite");
		Vector4 spriteColor = spriteModel.GetMaterial().GetColor();
		if (ImGui::ColorEdit4("Sprite_Color", reinterpret_cast<float*>(&spriteColor.x))) {
			spriteModel.GetMaterial().SetColor(spriteColor);
		}

		Vector3 spriteTranslate = spriteTransform.GetTranslate();
		if (ImGui::DragFloat3("Sprite_translate", &spriteTranslate.x, 0.01f)) {
			spriteTransform.SetTranslate(spriteTranslate);
		}
		Vector3 spriteRotate = spriteTransform.GetRotate();
		if (ImGui::DragFloat3("Sprite_rotate", &spriteRotate.x, 0.01f)) {
			spriteTransform.SetRotate(spriteRotate);
		}


		ImGui::DragFloat2("Sprite_UVtranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat2("Sprite_UVscale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
		ImGui::SliderAngle("Sprite_UVrotate", &uvTransformSprite.rotate.z);


#pragma endregion

#pragma region Model
		ImGui::Text("Model");
		Vector3 modelTranslate = modelTransform.GetTranslate();
		if (ImGui::DragFloat3("Model_translate", &modelTranslate.x, 0.01f)) {
			modelTransform.SetTranslate(modelTranslate);
		}
		Vector3 modelRotate = modelTransform.GetRotate();
		if (ImGui::DragFloat3("Model_rotate", &modelRotate.x, 0.01f)) {
			modelTransform.SetRotate(modelRotate);
		}


		bool modelLighting = modelModel.GetMaterial().IsLightingEnabled();
		if (ImGui::Checkbox("Model_useEnableLighting", &modelLighting)) {
			modelModel.GetMaterial().SetLightingEnable(modelLighting);
		}
		bool modelLambertian = modelModel.GetMaterial().IsLambertianReflectanceEnabled();
		if (ImGui::Checkbox("Model_useLambertianReflectance", &modelLambertian)) {
			modelModel.GetMaterial().SetLambertianReflectance(modelLambertian);
		}

#pragma endregion

		directionalLight.ImGui("DriectonalLight");

		ImGui::End();

		//																			//
		//							カメラの更新										//
		//																			//

		cameraController->Update();

		//																			//
		//							三角形用のWVP										//
		//																			//

		//三角形を回転させる
		triangleTransform.AddRotation({ 0.0f,0.03f,0.0f });

		//行列の更新
		triangleTransform.UpdateMatrix(cameraController->GetViewProjectionMatrix());

		//																			//
		//							球体用のWVP										//
		//																			//

		//球体を回転させる
		sphereTransform.AddRotation({ 0.0f,0.01f,0.0f });

		//行列の更新
		sphereTransform.UpdateMatrix(cameraController->GetViewProjectionMatrix());

		//																			//
		//							Sprite用のWVP									//
		//																			//

		//行列の更新
		spriteTransform.UpdateMatrix(cameraController->GetViewProjectionMatrixSprite());
		//uvTransformの更新
		spriteModel.GetMaterial().UpdateUVTransform(uvTransformSprite);


		//																			//
		//							Model用のWVP									//
		//																			//


		modelTransform.UpdateMatrix(cameraController->GetViewProjectionMatrix());

		// ImGuiの受け付け終了
		imguiManager->End();

		//								描画処理										//
		///*-----------------------------------------------------------------------*///
		//																			//
		///				画面をクリアする処理が含まれたコマンドリストを作る				   ///
		//																			//
		///*-----------------------------------------------------------------------*///


		directXCommon->PreDraw();


		///*-----------------------------------------------------------------------*///
		//																			//
		///							描画に必要コマンドを積む						   ///
		//																			//
		///*-----------------------------------------------------------------------*///

		//																			//
		//								三角形用の描画									//
		//																			//

#pragma region Triangle

		directXCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, triangleModel.GetMaterial().GetResource()->GetGPUVirtualAddress());
		directXCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, triangleTransform.GetResource()->GetGPUVirtualAddress());
		directXCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureManager->GetTextureHandle("uvChecker"));	//uvChecker
		directXCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLight.GetResource()->GetGPUVirtualAddress());
		triangleModel.GetMesh().Bind(directXCommon->GetCommandList());			// メッシュをバインド
		triangleModel.GetMesh().Draw(directXCommon->GetCommandList());			// メッシュを描画

#pragma endregion

		//																			//
		//								Sphereの描画									//
		//																			//
#pragma region Sphere

		directXCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, sphereModel.GetMaterial().GetResource()->GetGPUVirtualAddress());
		directXCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, sphereTransform.GetResource()->GetGPUVirtualAddress());
		directXCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, useMonsterBall ? textureManager->GetTextureHandle("monsterBall") : textureManager->GetTextureHandle("uvChecker"));
		directXCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLight.GetResource()->GetGPUVirtualAddress());
		sphereModel.GetMesh().Bind(directXCommon->GetCommandList());			// メッシュをバインド
		sphereModel.GetMesh().Draw(directXCommon->GetCommandList());			// メッシュを描画

#pragma endregion
		//																			//
		//								Spriteの描画									//
		//																			//

#pragma region Sprite
		directXCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, spriteModel.GetMaterial().GetResource()->GetGPUVirtualAddress());
		directXCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, spriteTransform.GetResource()->GetGPUVirtualAddress());
		directXCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureManager->GetTextureHandle("uvChecker"));
		directXCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLight.GetResource()->GetGPUVirtualAddress());

		spriteModel.GetMesh().Bind(directXCommon->GetCommandList());				// メッシュをバインド
		spriteModel.GetMesh().Draw(directXCommon->GetCommandList());				// メッシュを描画


#pragma endregion

		//																			//
		//								Modelの描画									//
		//																			//

#pragma region Model

		directXCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, modelModel.GetMaterial().GetResource()->GetGPUVirtualAddress());
		directXCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, modelTransform.GetResource()->GetGPUVirtualAddress());
		directXCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureManager->GetTextureHandle("planeTexture"));
		directXCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLight.GetResource()->GetGPUVirtualAddress()); // 同じライトを使用

		modelModel.GetMesh().Bind(directXCommon->GetCommandList());				// メッシュをバインド
		modelModel.GetMesh().Draw(directXCommon->GetCommandList());				// メッシュを描画

#pragma endregion


		// ImGuiの画面への描画
		imguiManager->Draw(directXCommon->GetCommandList());

		//	画面に描く処理はすべて終わり、画面に映すので、状態を遷移
		directXCommon->PostDraw();
	}


	// ImGuiの終了処理
	imguiManager->Finalize();

	///*-----------------------------------------------------------------------*///
	//																			//
	///							ReportLiveObjects							   ///
	//																			//
	///*-----------------------------------------------------------------------*///

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