#include <Windows.h>

#include "LeakChecker.h"	//リークチェッカー
#include "Engine/Engine.h"	//エンジン
#include "Game/Game.h"		//ゲーム

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
	engine->Initialize(L"LE2A_15_ミカミ_ヒロト_CG2");

	//ゲーム
	Game* game = new Game;
	game->Initialize();


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

		//エンジンのImGui
		engine->ImGui();

		//ゲームの更新処理
		game->Update();

		///*-----------------------------------------------------------------------*///
		//																			//
		///									描画処理								   ///
		//																			//
		///*-----------------------------------------------------------------------*///

		///描画前処理
		engine->StartDraw();


		//ゲームの描画
		game->Draw();


		///描画後処理
		engine->EndDraw();

	}

	///*-----------------------------------------------------------------------*///
	//																			//
	///									終了処理								   ///
	//																			//
	///*-----------------------------------------------------------------------*///

	//gameの終了処理
	game->Finalize();
	delete game;


	// エンジンの終了処理(最後)
	engine->Finalize();

	return 0;


}