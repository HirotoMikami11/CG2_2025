#include <Windows.h>
#include<cstdint>
#include<string>
#include<format>

//comptr
#include<wrl.h>
#include "MyFunction.h"

#include "LeakChecker.h"		//リークチェッカー

///トランスフォームと、モデルを統合させたGameObjectクラス
#include "GameObject.h"
///2D描画スプライトクラス
#include "Sprite.h"


#include"Light.h"//ライト
#include"CameraController.h"//カメラ系統

#include "Engine.h"	//エンジン



///*-----------------------------------------------------------------------*///
//																			//
///									メイン関数							   ///
//																			//
///*-----------------------------------------------------------------------*///
// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	D3DResourceLeakChecker leakCheak;

	// エンジンの取得
	Engine* engine = Engine::GetInstance();

	// エンジンの初期化
	engine->Initialize(L"LE2A_15_ミカミ_ヒロト_AL3");

	// システム参照
	DirectXCommon* directXCommon_ ;
	OffscreenRenderer* offscreenRenderer_;
	directXCommon_ = Engine::GetInstance()->GetDirectXCommon();
	offscreenRenderer_ = Engine::GetInstance()->GetOffscreenRenderer();

	///*-----------------------------------------------------------------------*///
	///								カメラの初期化									///
	///*-----------------------------------------------------------------------*///

	CameraController* cameraController = CameraController::GetInstance();
	cameraController->Initialize();

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
		triangle[i]->Initialize(directXCommon_, "uvChecker");			// 初期化
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
	sphere->Initialize(directXCommon_, "monsterBall");						// 初期化
	sphere->SetTransform(transformSphere);									// Transformを設定


#pragma endregion

	///*-----------------------------------------------------------------------*///
	///								矩形Sprite									///
	///*-----------------------------------------------------------------------*///

#pragma region "Sprite"

	std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>();			// スプライトを生成
	sprite->Initialize(directXCommon_, "uvChecker",
		{ 50,50 }, { 100,100 });							// 初期化



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
	model->Initialize(directXCommon_, "resources", "plane.obj");				// 初期化
	model->SetTransform(transformModel);									// Transformを設定


#pragma endregion

	///*-----------------------------------------------------------------------*///
	///									ライト									///
	///*-----------------------------------------------------------------------*///

	Light directionalLight;
	directionalLight.Initialize(directXCommon_, Light::Type::DIRECTIONAL);


	///*-----------------------------------------------------------------------*///
	//																			//
	///									メインループ							   ///
	//																			//
	///*-----------------------------------------------------------------------*///


	//ウィンドウのxボタンが押されるまでループ
	while (!engine->IsClosedWindow()) {

		// エンジンの更新
		engine->Update();



		///*-----------------------------------------------------------------------*///
		//																			//
		///									更新処理								   ///
		//																			//
		///*-----------------------------------------------------------------------*///
		engine->ImGui();

		//開発用UIの処理
		ImGui::Begin("Debug");

		////FPS関連
		//frameTimer.ImGui();

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

		///// オフスクリーンレンダラー（グリッチエフェクト含む）のImGui
		//offscreenRenderer->ImGui();

		ImGui::End();

		//																			//
		//							カメラの更新										//
		//																			//

		cameraController->Update();


		// 三角形を回転させる
		triangle[0]->AddRotation({ 0.0f, 0.03f, 0.0f });


		//球体を回転させる
		//isActiveの判定は行列更新にしかかかってないので現状isAciveは死んでる
		sphere->AddRotation({ 0.0f,0.01f,0.0f });




		//																			//
		//								行列更新										//
		//																			//

		//三角形の更新
		for (int i = 0; i < kMaxTriangleIndex; i++)
		{
			triangle[i]->Update(cameraController->GetViewProjectionMatrix());
		}

		//球体の更新
		sphere->Update(cameraController->GetViewProjectionMatrix());
		//スプライトの更新
		sprite->Update(cameraController->GetViewProjectionMatrixSprite());


		//プレーンモデルの更新
		model->Update(cameraController->GetViewProjectionMatrix());



		///*-----------------------------------------------------------------------*///
		//																			//
		///									描画処理								   ///
		//																			//
		///*-----------------------------------------------------------------------*///

		///描画前処理
		engine->StartDraw();



		// Sphereの描画
		sphere->Draw(directionalLight);
		for (int i = 0; i < kMaxTriangleIndex; i++) {
			triangle[i]->Draw(directionalLight);
		}
		model->Draw(directionalLight);

		sprite->Draw();


		///描画後処理
		engine->EndDraw();

	}

	///*-----------------------------------------------------------------------*///
	//																			//
	///									終了処理								   ///
	//																			//
	///*-----------------------------------------------------------------------*///

	//gameの終了処理


	// エンジンの終了処理(最後)
	engine->Finalize();

	return 0;


}