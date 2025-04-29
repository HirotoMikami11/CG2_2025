#include <Windows.h>
#include<cstdint>
#include<string>
#include<format>
//ファイルやディレクトリに関する操作を行うライブラリ
#include<filesystem>
//ファイルに書いたり読んだりするライブラリ
#include<fstream>
//時間を扱うライブラリ
#include<chrono>

#include<strsafe.h>
// Debug用のあれこれを使えるようにする
#include <dbghelp.h>
#pragma comment (lib,"Dbghelp.lib")

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

///ImGui
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

///DirectXTex
#include"externals/DirectXTex/DirectXTex.h"
#include"externals/DirectXTex/d3dx12.h"

#include<cassert>
#include <vector>
#include "MyFunction.h"


#include "Logger.h"
#include "WinApp.h"
#include "DirectXCommon.h"



/// CrashHandlerの登録をする関数
static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception) {
	///
	///	Dumpを出力する
	/// 

	//時刻を取得して、時刻を名前に入れたファイルを作成。Dumpディレクトリ以下に出力
	SYSTEMTIME time;
	GetLocalTime(&time);
	wchar_t filePath[MAX_PATH] = { 0 };
	CreateDirectory(L"./Dumps", nullptr);
	StringCchPrintfW(filePath, MAX_PATH, L"./Dumps/%04d-%02d%02d-%02d%02d.dmp", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute);
	HANDLE dumpFileHandle = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
	// processId(このexeのId)とクラッシュ（例外）の発生したthreadIdを取得
	DWORD processId = GetCurrentProcessId();
	DWORD threadId = GetCurrentThreadId();
	// 設定情報を入力
	MINIDUMP_EXCEPTION_INFORMATION minidumpInformation{ 0 };//miniDump
	minidumpInformation.ThreadId = threadId;
	minidumpInformation.ExceptionPointers = exception;
	minidumpInformation.ClientPointers = TRUE;

	//Dumpを出力　MiniDumpNormalは最低限の情報を出力するフラグ
	MiniDumpWriteDump(GetCurrentProcess(), processId, dumpFileHandle, MiniDumpNormal, &minidumpInformation, nullptr, nullptr);
	//他に関連付けられているSEH例外ハンドラがあれば実行、通常はプロセスを終了する
	return EXCEPTION_EXECUTE_HANDLER;
}

D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index) {
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index) {
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}

///*-----------------------------------------------------------------------*///
//																			//
///									メイン関数							   ///
//																			//
///*-----------------------------------------------------------------------*///
// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	//誰も補足しなかった場合に（Unhandled）、補足する関数を登録（main関数が始まってすぐ）
	SetUnhandledExceptionFilter(ExportDump);


	WinApp* winApp = new WinApp;
	DirectXCommon* directXCommon = new DirectXCommon;

	///
	/// ウィンドウクラスを登録する
	///

	winApp->Initialize();

	///
	///	ログをファイルに書き込む
	///

	Logger::Initalize();
	directXCommon->Initialize(winApp);

	///*-----------------------------------------------------------------------*///
	///									三角形									///
	///*-----------------------------------------------------------------------*///

	//																			//
	//							VertexResourceの作成								//
	//																			//

	//実際に頂点リソースを生成
	ID3D12Resource* vertexResource = CreateBufferResource(directXCommon->GetDevice(), sizeof(VertexData) * 6); //２つ三角形を作るので６個の頂点データ

	//																			//
	//							VertexBufferViewの作成							//
	//																			//

	//頂点バッファビューを作成
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	//リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	//仕様数リソースのサイズは頂点3つ分のサイズ
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 6; //２つ三角形を作るので６個の頂点データ
	//1頂点当たりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	//																			//
	//							Material用のResourceを作る						//
	//																			//

	//マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意
	ID3D12Resource* materialResource =
		CreateBufferResource(directXCommon->GetDevice(), sizeof(Material));
	//マテリアルデータに書き込む
	Material* materialData = nullptr;
	//書き込むためのアドレスを取得
	materialResource->
		Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	//白で初期化
	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData->enableLighting = false;
	materialData->useLambertianReflectance = false;


	//																			//
	//					TransformationMatrix用のリソースを作る						//
	//																			//

	//WVP用のリソースを作る、Matrix4x4　１つ分のサイズを用意する
	ID3D12Resource* wvpResource = CreateBufferResource(directXCommon->GetDevice(), sizeof(TransformationMatrix));
	//データを書き込む
	TransformationMatrix* wvpData = nullptr;
	//書き込むためのアドレスを取得
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	//単位行列を書き込んでおく
	wvpData->WVP = MakeIdentity4x4();
	wvpData->World = MakeIdentity4x4();

	//																			//
	//						Resourceにデータを書き込む								//
	//																			//

	//頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	//書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	//一つ目の三角形

	//左下
	vertexData[0].position = { -0.5f,-0.5f,0.0f,1.0f };
	vertexData[0].texcoord = { 0.0f,1.0f };
	//上
	vertexData[1].position = { 0.0f,0.5f,0.0f,1.0f };
	vertexData[1].texcoord = { 0.5f,0.0f };
	//右下
	vertexData[2].position = { 0.5f,-0.5f,0.0f,1.0f };
	vertexData[2].texcoord = { 1.0f,1.0f };

	//二つ目の三角形
	vertexData[3].position = { -0.5f,-0.5f,0.5f,1.0f };
	vertexData[3].texcoord = { 0.0f,1.0f };
	//上
	vertexData[4].position = { 0.0f,0.0f,0.0f,1.0f };
	vertexData[4].texcoord = { 0.5f,0.0f };
	//右下
	vertexData[5].position = { 0.5f,-0.5f,-0.5f,1.0f };
	vertexData[5].texcoord = { 1.0f,1.0f };

	//法線情報(三角形なので別のを後で用意)
	for (int i = 0; i < 6; i++)
	{
		vertexData[i].normal.x = vertexData[i].position.x;
		vertexData[i].normal.y = vertexData[i].position.y;
		vertexData[i].normal.z = vertexData[i].position.z;
	}


	///*-----------------------------------------------------------------------*///
	///									球体									///
	///*-----------------------------------------------------------------------*///

	//																			//
	//							VertexResourceの作成								//
	//																			//

	//実際に頂点リソースを生成
	ID3D12Resource* vertexResourceSphere = CreateBufferResource(directXCommon->GetDevice(), sizeof(VertexData) * (16 * 16 * 6)); //球用1536

	//																			//
	//							VertexBufferViewの作成							//
	//																			//

	//頂点バッファビューを作成
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSphere{};
	//リソースの先頭のアドレスから使う
	vertexBufferViewSphere.BufferLocation = vertexResourceSphere->GetGPUVirtualAddress();
	//仕様数リソースのサイズは頂点3つ分のサイズ
	vertexBufferViewSphere.SizeInBytes = sizeof(VertexData) * ((16 + 1) * (16 + 1)); //分割数nの時、交点は(n+1)になるため
	//1頂点当たりのサイズ
	vertexBufferViewSphere.StrideInBytes = sizeof(VertexData);

	//																			//
	//							indexResourceの作成								//
	//																			//

	//実際にインデックスリソースを生成
	ID3D12Resource* indexResourceSphere = CreateBufferResource(directXCommon->GetDevice(), sizeof(uint32_t) * 16 * 16 * 6); //２つ三角形を作るので６個の頂点データ


	//																			//
	//							indexBufferViewの作成							//
	//																			//

	//インデックスバッファビューを作成
	D3D12_INDEX_BUFFER_VIEW indexBufferViewSphere{};
	//リソースの先頭のアドレスから使う
	indexBufferViewSphere.BufferLocation = indexResourceSphere->GetGPUVirtualAddress();
	//使用するリソースのサイズはインデックス6つ分のサイズ
	indexBufferViewSphere.SizeInBytes = sizeof(uint32_t) * 16 * 16 * 6; //三角リスト方式の頂点数と同じ
	//1頂点当たりのサイズはuint32_t
	indexBufferViewSphere.Format = DXGI_FORMAT_R32_UINT;


	//インデックスリソースにデータを書き込む
	uint32_t* indexDataSphere = nullptr;
	//書き込むためのアドレスを取得
	indexResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSphere));
	CreateSphereIndexData(indexDataSphere);


	//																			//
	//							Material用のResourceを作る						//
	//																			//

	//マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意
	ID3D12Resource* materialResourceSphere =
		CreateBufferResource(directXCommon->GetDevice(), sizeof(Material));
	//マテリアルデータに書き込む
	Material* materialDataSphere = nullptr;
	//書き込むためのアドレスを取得
	materialResourceSphere->
		Map(0, nullptr, reinterpret_cast<void**>(&materialDataSphere));
	//白で初期化

	materialDataSphere->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialDataSphere->enableLighting = true;
	materialDataSphere->useLambertianReflectance = false;

	//																			//
	//					TransformationMatrix用のリソースを作る						//
	//																			//

	//WVP用のリソースを作る、Matrix4x4　１つ分のサイズを用意する
	ID3D12Resource* wvpResourceSphere = CreateBufferResource(directXCommon->GetDevice(), sizeof(TransformationMatrix));
	//データを書き込む
	TransformationMatrix* wvpDataSphere = nullptr;
	//書き込むためのアドレスを取得
	wvpResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&wvpDataSphere));
	//単位行列を書き込んでおく
	wvpDataSphere->WVP = MakeIdentity4x4();
	wvpDataSphere->World = MakeIdentity4x4();


	//																			//
	//							DirectionalLightのResourceを作る						//
	//																			//

	ID3D12Resource* directionalLightResourceSphere = CreateBufferResource(directXCommon->GetDevice(), sizeof(DirectionalLight));
	//データを書き込む
	DirectionalLight* directionalLightDataSphere = nullptr;
	//書き込むためのアドレスを取得
	directionalLightResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightDataSphere));
	//単位行列を書き込んでおく
	directionalLightDataSphere->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightDataSphere->direction = { 0.0f,-1.0f,0.0f };
	directionalLightDataSphere->intensity = 1.0f;

	//																			//
	//						Resourceにデータを書き込む								//
	//																			//

	//頂点リソースにデータを書き込む
	VertexData* vertexDataSphere = nullptr;
	//書き込むためのアドレスを取得
	vertexResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSphere));

	//球体のデータ
	CreateSphereVertexData(vertexDataSphere);


	///*-----------------------------------------------------------------------*///
	///								矩形Sprite									///
	///*-----------------------------------------------------------------------*///

	//																			//
	//							VertexResourceの作成								//
	//																			//

	//実際に頂点リソースを生成
	ID3D12Resource* vertexResourceSprite = CreateBufferResource(directXCommon->GetDevice(), sizeof(VertexData) * 4); //２つ三角形で矩形を作るので頂点データ6つ

	//																			//
	//							VertexBufferViewの作成							//
	//																			//

	//頂点バッファビューを作成
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	//リソースの戦闘のアドレスから使う
	vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	//仕様数リソースのサイズは頂点3つ分のサイズ
	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 4; //２つ三角形を作るので６個の頂点データ
	//1頂点当たりのサイズ
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);

	//																			//
	//							indexResourceの作成								//
	//																			//

	//実際にインデックスリソースを生成
	ID3D12Resource* indexResourceSprite = CreateBufferResource(directXCommon->GetDevice(), sizeof(uint32_t) * 6); //２つ三角形を作るので６個の頂点データ

	//																			//
	//							indexBufferViewの作成							//
	//																			//
	//インデックスバッファビューを作成
	D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
	//リソースの先頭のアドレスから使う
	indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
	//使用するリソースのサイズはインデックス6つ分のサイズ
	indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6; //２つ三角形を作るので６個の頂点データ
	//1頂点当たりのサイズはuint32_t
	indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;


	//インデックスリソースにデータを書き込む
	uint32_t* indexDataSprite = nullptr;
	//書き込むためのアドレスを取得
	indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	indexDataSprite[0] = 0; indexDataSprite[1] = 1; indexDataSprite[2] = 2;
	indexDataSprite[3] = 1; indexDataSprite[4] = 3; indexDataSprite[5] = 2;


	//																			//
	//							Material用のResourceを作る						//
	//																			//

//マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意
	ID3D12Resource* materialResourceSprite =
		CreateBufferResource(directXCommon->GetDevice(), sizeof(Material));
	//マテリアルデータに書き込む
	Material* materialDataSprite = nullptr;
	//書き込むためのアドレスを取得
	materialResourceSprite->
		Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
	//白で初期化

	materialDataSprite->color = { 1.0f,1.0f,1.0,1.0f };
	materialDataSprite->enableLighting = false;
	materialDataSprite->useLambertianReflectance = false;


	//																			//
	//					TransformationMatrix用のリソースを作る						//
	//																			//

//WVP用のリソースを作る、Matrix4x4　１つ分のサイズを用意する
	ID3D12Resource* transformationMatrixResourceSprite = CreateBufferResource(directXCommon->GetDevice(), sizeof(TransformationMatrix));
	//データを書き込む
	TransformationMatrix* transformationMatrixDataSprite = nullptr;
	//書き込むためのアドレスを取得
	transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));
	//単位行列を書き込んでおく

	transformationMatrixDataSprite->WVP = MakeIdentity4x4();
	transformationMatrixDataSprite->World = MakeIdentity4x4();

	//																			//
	//						Resourceにデータを書き込む								//
	//																			//

	//頂点リソースにデータを書き込む
	VertexData* vertexDataSprite = nullptr;
	//書き込むためのアドレスを取得
	vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));
	//Spriteを表示するための四頂点
	CreateSpriteVertexData(vertexDataSprite, { 320.0f,180.0f }, { 320.0f,180.0f });

	///*-----------------------------------------------------------------------*///
	//																			//
	///								ImGuiの初期化								   ///
	//																			//
	///*-----------------------------------------------------------------------*///
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(winApp->GetHwnd());
	ImGui_ImplDX12_Init(
		directXCommon->GetDevice(),
		directXCommon->GetSwapChainDesc().BufferCount,
		directXCommon->GetRTVDesc().Format,
		directXCommon->GetSRVDescriptorHeap(),
		directXCommon->GetSRVDescriptorHeap()->GetCPUDescriptorHandleForHeapStart(),
		directXCommon->GetSRVDescriptorHeap()->GetGPUDescriptorHandleForHeapStart()
	);

	//								　変数宣言									//

	//Transform変数を作る
	Vector3Transform transform{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{2.0f,1.5f,0.0f}
	};

	//SphereのTransform変数を作る
	Vector3Transform transformSphere{
	{1.0f,1.0f,1.0f},
	{0.0f,0.0f,0.0f},
	{0.0f,0.0f,0.0f}
	};

	//SpriteのTransform変数を作る
	Vector3Transform transformSprite{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};

	//WorldViewProjectionMatrixを作る
	Vector3Transform cameraTransform{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,-10.0f}
	};

	//ImGuiで使用する変数
	bool useMonsterBall = true;
	bool useEnableLighting = true;

	///*-----------------------------------------------------------------------*///
	//																			//
	///									メインループ							   ///
	//																			//
	///*-----------------------------------------------------------------------*///


	//ウィンドウのxボタンが押されるまでループ
	while (winApp->ProsessMessege()) {
		//							　ゲームの処理										//


		//								更新処理										//

		//ImGuiにフレームが始まることを伝える
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		//開発用UIの処理
		ImGui::Begin("Material Color");
		ImGui::ColorEdit4("Color", reinterpret_cast<float*>(&materialData->color.x));
		ImGui::Text("Triangle");
		ImGui::DragFloat3("Triangle_translate", &transform.translate.x, 0.01f);
		ImGui::DragFloat3("Triangle_rotate", &transform.rotate.x, 0.01f);
		ImGui::Text("Sphere");
		ImGui::DragFloat3("Sphere_translate", &transformSphere.translate.x, 0.01f);
		ImGui::DragFloat3("Sphere_rotate", &transformSphere.rotate.x, 0.01f);
		ImGui::DragFloat3("Sphere_LightDirection", &directionalLightDataSphere->direction.x, 0.01f);


		ImGui::Checkbox("useMonsterBall", &useMonsterBall);
		ImGui::Checkbox("useEnableLighting", reinterpret_cast<bool*>(&materialDataSphere->enableLighting));
		ImGui::Checkbox("useLambertianReflectance", reinterpret_cast<bool*>(&materialDataSphere->useLambertianReflectance));
		ImGui::Text("Sprite");
		ImGui::DragFloat3("Sprite_translate", &transformSprite.translate.x, 0.01f);
		ImGui::DragFloat3("Sprite_rotate", &transformSprite.rotate.x, 0.01f);
		ImGui::End();
		//																			//
		//							三角形用のWVP										//
		//																			//

		//三角形を回転させる
		transform.rotate.y += 0.03f;
		//viewprojectionを計算
		Matrix4x4 viewProjectionMatrix = MakeViewProjectionMatrix(cameraTransform, (float(kClientWidth) / float(kClientHeight)));
		//行列の更新
		UpdateMatrix4x4(transform, viewProjectionMatrix, wvpData);

		//																			//
		//							球体用のWVP										//
		//																			//

		//viewprojectionを計算
		Matrix4x4 viewProjectionMatrixSphere = MakeViewProjectionMatrix(cameraTransform, (float(kClientWidth) / float(kClientHeight)));
		//行列の更新
		UpdateMatrix4x4(transformSphere, viewProjectionMatrix, wvpDataSphere);

		//																			//
		//							Sprite用のWVP									//
		//																			//

		//viewprojectionを計算
		Matrix4x4 viewProjectionMatrixSprite = MakeViewProjectionMatrixSprite();
		//行列の更新
		UpdateMatrix4x4(transformSprite, viewProjectionMatrixSprite, transformationMatrixDataSprite);


		//ImGuiの内部コマンドを生成する(描画処理に入る前)
		ImGui::Render();

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


		directXCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());	//マテリアルのCBufferの場所を設定
		directXCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());			//wvp用のCBufferの場所を設定
		directXCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, directXCommon->GetTextureSrvHandles()[0]);	//uvChecker
		directXCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);	//VBを設定
		// 描画！（DrawCall／ドローコール）。３頂点で1つのインスタンス
		directXCommon->GetCommandList()->DrawInstanced(6, 1, 0, 0);


		//																			//
		//									Sphere									//
		//																			//

		directXCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourceSphere->GetGPUVirtualAddress());	//マテリアルのCBufferの場所を設定
		directXCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResourceSphere->GetGPUVirtualAddress());			//wvp用のCBufferの場所を設定

		directXCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResourceSphere->GetGPUVirtualAddress());
		//wvp用のCBufferの場所を設定
		directXCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, useMonsterBall ? directXCommon->GetTextureSrvHandles()[1] : directXCommon->GetTextureSrvHandles()[0]);
		directXCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewSphere);	//VBを設定
		directXCommon->GetCommandList()->IASetIndexBuffer(&indexBufferViewSphere);//Index
		///indexBufferで表示するため、VertexBufferはコメントアウト
		// 描画！（DrawCall／ドローコール）。３頂点で1つのインスタンス
		//directXCommon->GetCommandList()->DrawInstanced(16 * 16 * 6, 1, 0, 0);
		directXCommon->GetCommandList()->DrawIndexedInstanced(16 * 16 * 6, 1, 0, 0, 0);

		//																			//
		//									Sprite									//
		//																			//

		directXCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());	//マテリアルのCBufferの場所を設定


		directXCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);//Vertex
		directXCommon->GetCommandList()->IASetIndexBuffer(&indexBufferViewSprite);//Index

		directXCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, directXCommon->GetTextureSrvHandles()[0]);

		//TransformMatrixCBufferの場所を設定
		directXCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());

		// 描画（DrawCall／ドローコール)
		///indexBufferで表示するため、VertexBufferはコメントアウト
		//三角形を二つ描画するので6つ
		//directXCommon->GetCommandList()->DrawInstanced(6, 1, 0, 0);
		directXCommon->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);




		//実際の directXCommon-> GetCommandList()のImGuiの描画コマンドを積む
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), directXCommon->GetCommandList());

		//	画面に描く処理はすべて終わり、画面に映すので、状態を遷移
		directXCommon->PostDraw();
	}


	//ImGuiの終了処理
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	///*-----------------------------------------------------------------------*///
	//																			//
	///							ReportLiveObjects							   ///
	//																			//
	///*-----------------------------------------------------------------------*///



	//三角形を生成するものの解放処理
	vertexResource->Release();
	materialResource->Release();
	wvpResource->Release();


	//Sprite
	vertexResourceSprite->Release();
	transformationMatrixResourceSprite->Release();
	materialResourceSprite->Release();
	indexResourceSprite->Release();


	//Sphere
	vertexResourceSphere->Release();
	materialResourceSphere->Release();
	wvpResourceSphere->Release();
	directionalLightResourceSphere->Release();
	indexResourceSphere->Release();


	winApp->Finalize();
	delete winApp;
	directXCommon->Finalize();
	delete directXCommon;

	//リソースリークチェック
	IDXGIDebug1* debug;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		debug->Release();
	}

	//COMの終了処理
	CoUninitialize();

	return 0;


}