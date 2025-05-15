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

///ImGui
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

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
//三角形
#include "Triangle.h"
//三角柱を三つ
#include "TriForce.h"
//球体
#include "Sphere.h"

//画像
#include "Sprite.h"


#include "AudioManager.h"
#include "Emitter.h"
#include "SkyDustEmitter.h"
#include "BreakScreenEffect.h"

/// <summary>
/// deleteの前に置いておく、infoの警告消すことで、リークの種類を判別できる
/// </summary>
void DumpLiveObjects() {
	Microsoft::WRL::ComPtr<IDXGIDebug1> debug;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
	}
}


MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
	//1.中で必要となる変数の宣言
	MaterialData materialData;			//構築するMaterialData
	std::string line;					//ファイルから読んだ1行を格納するもの

	//2.ファイルを開く
	std::ifstream file(directoryPath + "/" + filename);//ファイルを開く
	assert(file.is_open());//開けなかった場合は停止する

	//3.実際にファイルを読み、ModelDataを構築していく
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;	//先頭の識別子を読む

		//identifierに応じた処理
		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			//連結してファイルパスにする
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}
	//4.MaterialDataを返す
	return materialData;
}


ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename) {
	//1.中で必要となる変数の宣言
	ModelData modelData;				//構築するModelData
	std::vector<Vector4> positions;		//位置
	std::vector<Vector3> normals;		//法線
	std::vector<Vector2> texcoords;		//テクスチャ座標
	std::string line;					//ファイルから読んだ1行を格納するもの

	//2.ファイルを開く
	std::ifstream file(directoryPath + "/" + filename);//ファイルを開く
	assert(file.is_open());//開けなかった場合は停止する

	//3.実際にファイルを読み、ModelDataを構築していく
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;	//先頭の識別子を読む

		if (identifier == "v") {		//識別子がvの場合	頂点位置
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;	//w成分は1.0fで初期化
			positions.push_back(position);//位置を格納する

		} else if (identifier == "vt") {//識別子がvtの場合	頂点テクスチャ
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoords.push_back(texcoord);//テクスチャ座標を格納する

		} else if (identifier == "vn") {//識別子がvnの場合	頂点法線
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);//法線を格納する

		} else if (identifier == "f") {	//識別子がfの場合	面
			//面は三角形限定
			VertexData triangle[3];//三角形の頂点データを格納する

			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;	//頂点の情報を読む
				//頂点の要素へのindexは「位置/UV/法線」で格納されているので、分解してindexを取得する
				std::stringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/');// '/'区切りでインデックスを読んでいく
					elementIndices[element] = std::stoi(index);
				}

				//要素へのindexから、実際の要素の値を取得して頂点を構成する
				Vector4 position = positions[elementIndices[0] - 1];	//位置は1始まりなので-1する
				Vector2 texcoord = texcoords[elementIndices[1] - 1];	//テクスチャ座標も-1
				Vector3 normal = normals[elementIndices[2] - 1];		//法線も-1

				//右手座標系から左手座標系に変換する
				position.x *= -1.0f;
				normal.x *= -1.0f;
				texcoord.y = 1.0f - texcoord.y; // Y軸反転
				triangle[faceVertex] = { position, texcoord, normal };//頂点データを構成する
			}
			modelData.vertices.push_back(triangle[2]);//頂点データを格納する
			modelData.vertices.push_back(triangle[1]);//頂点データを格納する
			modelData.vertices.push_back(triangle[0]);//頂点データを格納する
		} else if (identifier == "mtllib") {
			//materialTemplateLibraryファイルの名前を取得する
			std::string materialFilename;
			s >> materialFilename;
			//objファイルとmtlファイルは同じディレクトリにあるので、ディレクトリ・ファイル名を渡す
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}
	//4.ModelDataを返す
	return modelData;

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
	//DirectX初期化の末尾にXAudio2エンジンのインスタンス生成
	AudioManager::GetInstance()->Initialize();

	//スクリーンを割るエフェクト生成
	BreakScreenEffect* breakScreenEffect = new BreakScreenEffect();
	// 初期化（DirectX初期化後）
	breakScreenEffect->Initialize(directXCommon->GetDevice());

	///*-----------------------------------------------------------------------*///
	///									三角形									///
	///*-----------------------------------------------------------------------*///
#pragma region Triangle


	///三角形の初期化
	Triangle* triangle[2];
	const int indexTriangle = 2;
	for (int i = 0; i < indexTriangle; i++)
	{
		triangle[i] = new Triangle();
		triangle[i]->Initialize(directXCommon->GetDevice());
		triangle[i]->SetPosition({ 0.0f, 0.0f, 0.0f });
		triangle[i]->SetRotation({ 0.0f, i * 0.8f, 0.0f });
		triangle[i]->SetScale({ 2.0f, 2.0f, 2.0f });
		triangle[i]->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
	}

#pragma endregion

	///三角柱の生成
	TriForce* triforce = new TriForce(directXCommon->GetDevice());
	triforce->Initialize();
	//開始と同時にイージング開始させる
	//triforce->StartEasing();
	///パーティクル

	const int EmitterIndex = 2;

	Emitter* emitter[EmitterIndex];
	for (int i = 0; i < EmitterIndex; i++)//エミッターを複数生成
	{
		emitter[i] = new Emitter(directXCommon->GetDevice());
		emitter[i]->Initialize(i);
	}
	//白色、真上に上昇する
	emitter[0]->SetParticleData(
		Vector3(0.025f, 0.025f, 0.025f),
		Vector3(0.06f, 0.06f, 0.06f),
		Vector3(0.0f, 0.0f, 0.0f),
		Vector3(0.0f, 0.0f, 3.14f),
		Vector3(-5.0f, -2.0f, -5.0f),
		Vector3(5.0f, -2.0f, 5.0f),
		0.05f,
		0.15f,
		Vector3(0.0f, 1.0f, 0.0f),
		0.05f,
		Vector4{ 1.0f,1.0f,1.0f,0.0f }
	);
	//黄色、左斜め前方に上昇する
	emitter[1]->SetParticleData(
		Vector3(0.025f, 0.025f, 0.025f),
		Vector3(0.07f, 0.07f, 0.07f),
		Vector3(0.0f, 0.0f, 0.0f),
		Vector3(0.0f, 0.0f, 3.14f),
		Vector3(-5.0f, -2.0f, -5.0f),
		Vector3(5.0f, -0.9f, 5.0f),
		0.33f,
		0.48f,
		Vector3(-1.0f, 0.5f, -1.0f),
		0.05f,
		Vector4{ 1.0f,1.0f,0.0f,0.0f }
	);
	SkyDustEmitter* skyDustEmitter = new SkyDustEmitter(directXCommon->GetDevice());
	skyDustEmitter->Initialize();



	///*-----------------------------------------------------------------------*///
	///									球体									///
	///*-----------------------------------------------------------------------*///

#pragma region Sphere

	//// 球体の初期化
	//Sphere* sphere = new Sphere();
	//sphere->Initialize(directXCommon->GetDevice());
	//sphere->SetPosition({ 0.0f, 0.0f, 0.0f });
	//sphere->SetRotation({ 0.0f, 0.0f, 0.0f });
	//sphere->SetScale({ 1.0f, 1.0f, 1.0f });
	//sphere->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
	//// デフォルトのライト設定
	//sphere->SetDirectionalLight({ 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, 1.0f);

#pragma endregion

	///*-----------------------------------------------------------------------*///
	///								矩形Sprite									///
	///*-----------------------------------------------------------------------*///

#pragma region "Sprite"

//// スプライトの初期化
//	Sprite* sprite = new Sprite();
//	sprite->Initialize(directXCommon->GetDevice());
//	sprite->SetPosition({ 320.0f, 180.0f });         // 中心座標
//	sprite->SetSize({ 640.0f, 360.0f });            // 実際のサイズ
//	sprite->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
#pragma endregion

	///*-----------------------------------------------------------------------*///
	///									model									///
	///*-----------------------------------------------------------------------*///

#pragma region "Model"

	////モデルデータ作成
	//ModelData modelData = LoadObjFile("resources", "plane.obj");

	//directXCommon->LoadTextureResourceForSRV(modelData.material.textureFilePath, 2);
	//directXCommon->MakeSRV(modelData.material.textureFilePath, 2);

	//																			//
	//							VertexResourceの作成								//
	//																			//
	//Microsoft::WRL::ComPtr <ID3D12Resource> vertexResourceModel = CreateBufferResource(directXCommon->GetDevice(), sizeof(VertexData) * modelData.vertices.size());

	////																			//
	////							VertexBufferViewの作成							//
	////																			//
	////頂点バッファビュー作成
	//D3D12_VERTEX_BUFFER_VIEW vertexBufferViewModel{};
	//vertexBufferViewModel.BufferLocation = vertexResourceModel->GetGPUVirtualAddress();			//リソースの戦闘のアドレスから使う
	//vertexBufferViewModel.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());	//使用するリソースのサイズは頂点のサイズ
	//vertexBufferViewModel.StrideInBytes = sizeof(VertexData);									//1頂点当たりのサイズ

	//																			//
	//							Material用のResourceを作る						//
	//																			//
	//// モデル用のマテリアルリソースを作成
	//Microsoft::WRL::ComPtr <ID3D12Resource> materialResourceModel = CreateBufferResource(directXCommon->GetDevice(), sizeof(Material));
	//Material* materialDataModel = nullptr;
	//materialResourceModel->Map(0, nullptr, reinterpret_cast<void**>(&materialDataModel));
	//materialDataModel->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	//materialDataModel->enableLighting = true;
	//materialDataModel->useLambertianReflectance = false;
	//materialDataModel->uvTransform = MakeIdentity4x4();


	////																			//
	////							DirectionalLightのResourceを作る						//
	////																			//

	//Microsoft::WRL::ComPtr <ID3D12Resource> directionalLightResourceModel = CreateBufferResource(directXCommon->GetDevice(), sizeof(DirectionalLight));
	////データを書き込む
	//DirectionalLight* directionalLightDataModel = nullptr;
	////書き込むためのアドレスを取得
	//directionalLightResourceModel->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightDataModel));
	////単位行列を書き込んでおく
	//directionalLightDataModel->color = { 1.0f,1.0f,1.0f,1.0f };
	//directionalLightDataModel->direction = { 0.0f,-1.0f,0.0f };
	//directionalLightDataModel->intensity = 1.0f;


	////																			//
	////					TransformationMatrix用のリソースを作る						//
	////																			//

	//Microsoft::WRL::ComPtr <ID3D12Resource> transformMatrixResourceModel = CreateBufferResource(directXCommon->GetDevice(), sizeof(TransformationMatrix));
	//TransformationMatrix* transformMatrixDataModel = nullptr;
	//transformMatrixResourceModel->Map(0, nullptr, reinterpret_cast<void**>(&transformMatrixDataModel));
	//transformMatrixDataModel->WVP = MakeIdentity4x4();
	//transformMatrixDataModel->World = MakeIdentity4x4();

	////																			//
	////						Resourceにデータを書き込む								//
	////																			//
	//VertexData* vertexDataModel = nullptr;
	//vertexResourceModel->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataModel));						//書き込むためのアドレス取得
	//std::memcpy(vertexDataModel, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());	//頂点データをリソースにコピー

#pragma endregion
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

	//WorldViewProjectionMatrixを作る
	Vector3Transform cameraTransform{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,-10.0f}
	};

	// モデル用のTransform変数
	Vector3Transform transformModel{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f}
	};

	//ImGuiで使用する変数
	bool useMonsterBall[2];

	for (int i = 0; i < 2; i++)
	{
		useMonsterBall[i] = false;
	}

	//シーンの変更
	bool directionScene = true;

	////ゲーム開始前に読み込む音声データ
	//AudioManager::GetInstance()->LoadWave("resources/Alarm01.wav", "Alarm");
	////tagを利用して再生
	//AudioManager::GetInstance()->PlayLoop("Alarm");
	//AudioManager::GetInstance()->SetVolume("Alarm",0.1f);
	///*-----------------------------------------------------------------------*///
	//																			//
	///									メインループ							   ///
	//																			//
	///*-----------------------------------------------------------------------*///


	//ウィンドウのxボタンが押されるまでループ
	while (winApp->ProsessMessege()) {
		//							　ゲームの処理										//


		//								更新処理										//
		///*-----------------------------------------------------------------------*///
		//																			//
		///								更新処理									   ///
		//																			//
		///*-----------------------------------------------------------------------*///

		//ImGuiにフレームが始まることを伝える
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		//開発用UIの処理
		ImGui::Begin("ChangeScene");
		ImGui::Checkbox("directionScene", &directionScene);
		ImGui::End();


		//viewprojectionを計算
		Matrix4x4 viewProjectionMatrix = MakeViewProjectionMatrix(cameraTransform, (float(kClientWidth) / float(kClientHeight)));

		if (!directionScene) {
			///必須内容のシーン
#pragma region normalScene

#pragma	region ImGui
			//開発用UIの処理
			ImGui::Begin("Debug");

			///
			///三角形を動かすUI
			/// 
			for (int i = 0; i < indexTriangle; i++) {
				//imguiで%d使えなかったため
				std::string label_translate = "Triangle_translate " + std::to_string(i);
				std::string label_rotate = "Triangle_rotate " + std::to_string(i);
				std::string label_scale = "Triangle_scale " + std::to_string(i);
				std::string label_color = "Triangle_color " + std::to_string(i);

				ImGui::Text("Triangle %d", i);

				//色
				ImGui::ColorEdit4(label_color.c_str(), reinterpret_cast<float*>(&triangle[i]->GetColor().x));
				ImGui::Separator();
				// 位置・回転・スケール
				ImGui::DragFloat3(label_translate.c_str(), const_cast<float*>(&triangle[i]->GetTransform().translate.x), 0.01f);
				ImGui::DragFloat3(label_rotate.c_str(), const_cast<float*>(&triangle[i]->GetTransform().rotate.x), 0.01f);
				ImGui::DragFloat3(label_scale.c_str(), const_cast<float*>(&triangle[i]->GetTransform().scale.x), 0.01f);
				ImGui::Separator();
				//コンボボックスの選択肢
				const char* textures[] = { "uvChecker", "MonsterBall" };

				// 各三角形ごとにテクスチャを変更
				static int selected_textures[2] = { 0 };
				if (ImGui::Combo(("Select texture " + std::to_string(i)).c_str(), &selected_textures[i], textures, IM_ARRAYSIZE(textures))) {
					// アイテムが変更されたときに処理
					if (selected_textures[i] == 0) {
						useMonsterBall[i] = false;  // uvCheakerが選ばれた時
					} else if (selected_textures[i] == 1) {
						useMonsterBall[i] = true;  // monsterbollが選ばれた時
					}
				}

			}
			ImGui::End();
#pragma endregion

			//																			//
			//							三角形用のWVP										//
			//																			//


			//三角形を回転させる
			for (int i = 0; i < indexTriangle; i++) {
				Vector3Transform transform[2];
				transform[i] = triangle[i]->GetTransform();
				//transform[i].rotate.y += 0.03f;
				//triangle[i]->SetRotation(transform[i].rotate);

				//行列の更新
				triangle[i]->Update(viewProjectionMatrix);

			}


			//																			//
			//							球体用のWVP										//
			//																			//

			// // 球体を回転させる
			//Vector3Transform transformSphere = sphere->GetTransform();
			//transformSphere.rotate.y += 0.01f;
			//sphere->SetRotation(transformSphere.rotate);

			//// 行列更新
			//sphere->Update(viewProjectionMatrix);

			//																			//
			//							Sprite用のWVP									//
			//																			//

			//// スプライトの更新
			//sprite->Update();

			//																			//
			//							Model用のWVP									//
			//																			//

			//UpdateMatrix4x4(transformModel, viewProjectionMatrix, transformMatrixDataModel);


#pragma endregion

		} else {
			///映像演出のシーン
#pragma region DiectionScene

			



			// トライフォースの更新（イージング開始フラグに応じて動作）
			triforce->MoveEasing(viewProjectionMatrix);
			triforce->Update(viewProjectionMatrix);

			// triforceが完了したらbreakScreenEffectを開始
			static bool lastTriforceCompleted = false;
			bool triforceCompleted = triforce->IsCompleted();

			// トライフォースが完了して、エフェクトがアクティブでない場合のみ開始
			if (triforceCompleted && !lastTriforceCompleted && !breakScreenEffect->GetActive()) {
				breakScreenEffect->SetActive(true);
				//triforce->StopEasing(); // エフェクト開始時にイージングを停止
			}
			lastTriforceCompleted = triforceCompleted;

			// isMovingOutが終わったタイミングでトライフォースをリセット
			static bool lastMovingOut = false;
			static bool hasResetTriforce = false;

			// isMovingOutからfalseに変化した瞬間（画面外への移動が完了した瞬間）
			bool movingOutJustFinished = lastMovingOut && !breakScreenEffect->IsMovingOut() && breakScreenEffect->GetActive();

			if (movingOutJustFinished && !hasResetTriforce) {
				// トライフォースを完全に初期状態にリセット（破片が画面外に移動中）
				triforce->Initialize();
				triforce->ResetProgress(); // 進行度とイージングフラグをリセット
				hasResetTriforce = true;
			}

			// エフェクトが完了して非アクティブになったタイミングで次のサイクル開始
			static bool lastEffectActive = false;
			bool effectJustFinished = lastEffectActive && !breakScreenEffect->GetActive();

			if (effectJustFinished) {
				// エフェクトリセットと次のサイクル開始
				breakScreenEffect->Reset(); // エフェクトをリセット
				hasResetTriforce = false;   // リセットフラグをクリア

				//トライフォースのイージング開始
				triforce->StartEasing();
			}

			// 現在の状態を記録
			lastMovingOut = breakScreenEffect->IsMovingOut();
			lastEffectActive = breakScreenEffect->GetActive();

			// breakScreenEffectの更新（自動アニメーション）
			if (breakScreenEffect->GetActive()) {
				breakScreenEffect->Update(); // 引数なしで自動アニメーション
			}




			for (int i = 0; i < EmitterIndex; i++)
			{
				emitter[i]->Update((1.0f / 60.0f));
			}

			skyDustEmitter->Update((1.0f / 60.0f));

#pragma endregion

		}
		//ImGuiの内部コマンドを生成する(描画処理に入る前)
		ImGui::Render();

		//								描画処理										//
		///*-----------------------------------------------------------------------*///
		//																			//
		///								描画処理						　			   ///
		//																			//
		///*-----------------------------------------------------------------------*///


		///*-----------------------------------------------------------------------*///
		//																			//
		///							描画に必要コマンドを積む						   ///
		//																			//
		///*-----------------------------------------------------------------------*///


		//																			//
		//							オフスクリーンの処理								//
		//																			//
		
		/// オフスクリーンレンダリングを演出シーンでのみ使用
		/// 演出シーンかつBreakScreenEffectがtrueな時のみオフスクリーンレンダリング
		if (directionScene && breakScreenEffect->GetActive()) {
			// 1. オフスクリーンレンダリング開始
			directXCommon->BeginOffScreen();

			// 2. 演出シーンの描画をオフスクリーンに行う
			triforce->Draw(directXCommon->GetCommandList(), directXCommon->GetTextureGPUSrvHandles()[2]);
			for (int i = 0; i < EmitterIndex; i++) {
				emitter[i]->Draw(directXCommon->GetCommandList(), directXCommon->GetTextureGPUSrvHandles()[2], viewProjectionMatrix);
			}
			skyDustEmitter->Draw(directXCommon->GetCommandList(), directXCommon->GetTextureGPUSrvHandles()[2], viewProjectionMatrix);

			// 3. オフスクリーンレンダリング終了
			directXCommon->EndOffScreen();

			// 4. メインのレンダーターゲットに戻してポストエフェクトを描画
			directXCommon->PreDraw(directionScene, breakScreenEffect->GetActive());

			// 5. breakScreenエフェクトを描画
			breakScreenEffect->Draw(
				directXCommon->GetCommandList(),
				directXCommon->GetOffScreenSRVHandle()
			);



		} else {

		//																	//
		//							通常描画の処理								//
		//																	//

			// 通常の描画
			directXCommon->PreDraw(directionScene, breakScreenEffect->GetActive());

			if (!directionScene) {
				///必須内容のシーン

#pragma region normalScene

				//																			//
				//								三角形用の描画									//
				//																			//
#pragma region Triangle
				for (int i = 0; i < indexTriangle; i++) {

					//directXCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResourceSphere->GetGPUVirtualAddress());

					triangle[i]->Draw(
						directXCommon->GetCommandList(),
						useMonsterBall[i] ? directXCommon->GetTextureGPUSrvHandles()[1] : directXCommon->GetTextureGPUSrvHandles()[0]
					);
				}
#pragma endregion

				//																			//
				//								Sphereの描画									//
				//																			//
#pragma region Sphere
				//sphere->Draw(
				//	directXCommon->GetCommandList(),
				//	directXCommon->GetTextureGPUSrvHandles()[0]
				//);
#pragma endregion
				//																			//
				//								Spriteの描画									//
				//																			//

#pragma region Sprite

	/*			sprite->Draw(
					directXCommon->GetCommandList(),
					directXCommon->GetTextureGPUSrvHandles()[0]
				);*/

#pragma endregion

				//																			//
				//								Modelの描画									//
				//																			//

#pragma region Model

			//directXCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourceModel->GetGPUVirtualAddress());
			//directXCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformMatrixResourceModel->GetGPUVirtualAddress());
			//directXCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResourceModel->GetGPUVirtualAddress()); // 同じライトを使用
			//directXCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, directXCommon->GetTextureGPUSrvHandles()[2]); // テクスチャ
			//directXCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewModel);
			//// インデックスバッファがない場合は直接頂点で描画
			//directXCommon->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);

#pragma endregion

#pragma endregion

			} else {
				///映像演出のシーン
#pragma region DiectionScene

				// 透明描画（オフスクリーンエフェクトなし）
				triforce->Draw(directXCommon->GetCommandList(), directXCommon->GetTextureGPUSrvHandles()[2]);
				for (int i = 0; i < EmitterIndex; i++) {
					emitter[i]->Draw(directXCommon->GetCommandList(), directXCommon->GetTextureGPUSrvHandles()[2], viewProjectionMatrix);
				}
				skyDustEmitter->Draw(directXCommon->GetCommandList(), directXCommon->GetTextureGPUSrvHandles()[2], viewProjectionMatrix);
			}
#pragma endregion
		}

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

	for (int i = 0; i < EmitterIndex; i++)
	{
		delete emitter[i];
	}

	delete skyDustEmitter;

	//三角形の前で解放
	delete triforce;

	//三角形を生成するものの解放処理
	for (int i = 0; i < indexTriangle; i++) {
		delete triangle[i];
	}
	//delete sphere;
	//delete sprite;
	delete breakScreenEffect;
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