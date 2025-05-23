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
#include "AudioManager.h"


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


	///*-----------------------------------------------------------------------*///
	///									三角形									///
	///*-----------------------------------------------------------------------*///

#pragma region Triangle

	//																			//
	//							VertexResourceの作成								//
	//																			//

	//実際に頂点リソースを生成
	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResource = CreateBufferResource(directXCommon->GetDevice(), sizeof(VertexData) * 6); //２つ三角形を作るので６個の頂点データ

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
	Microsoft::WRL::ComPtr <ID3D12Resource> materialResource =
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
	materialData->uvTransform = MakeIdentity4x4();

	//																			//
	//					TransformationMatrix用のリソースを作る						//
	//																			//

	//WVP用のリソースを作る、Matrix4x4　１つ分のサイズを用意する
	Microsoft::WRL::ComPtr <ID3D12Resource> wvpResource = CreateBufferResource(directXCommon->GetDevice(), sizeof(TransformationMatrix));
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

#pragma endregion

	///*-----------------------------------------------------------------------*///
	///									球体									///
	///*-----------------------------------------------------------------------*///

#pragma region Sphere

	//																			//
	//							VertexResourceの作成								//
	//																			//

	//実際に頂点リソースを生成
	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResourceSphere = CreateBufferResource(directXCommon->GetDevice(), sizeof(VertexData) * (16 * 16 * 6)); //球用1536

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
	Microsoft::WRL::ComPtr <ID3D12Resource> indexResourceSphere = CreateBufferResource(directXCommon->GetDevice(), sizeof(uint32_t) * 16 * 16 * 6); //２つ三角形を作るので６個の頂点データ


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
	Microsoft::WRL::ComPtr <ID3D12Resource> materialResourceSphere =
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
	materialDataSphere->uvTransform = MakeIdentity4x4();

	//																			//
	//					TransformationMatrix用のリソースを作る						//
	//																			//

	//WVP用のリソースを作る、Matrix4x4　１つ分のサイズを用意する
	Microsoft::WRL::ComPtr <ID3D12Resource> wvpResourceSphere = CreateBufferResource(directXCommon->GetDevice(), sizeof(TransformationMatrix));
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

	Microsoft::WRL::ComPtr <ID3D12Resource> directionalLightResourceSphere = CreateBufferResource(directXCommon->GetDevice(), sizeof(DirectionalLight));
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

#pragma endregion

	///*-----------------------------------------------------------------------*///
	///								矩形Sprite									///
	///*-----------------------------------------------------------------------*///

#pragma region "Sprite"

	//																			//
	//							VertexResourceの作成								//
	//																			//

	//実際に頂点リソースを生成
	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResourceSprite = CreateBufferResource(directXCommon->GetDevice(), sizeof(VertexData) * 4); //２つ三角形で矩形を作るので頂点データ6つ

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
	Microsoft::WRL::ComPtr <ID3D12Resource> indexResourceSprite = CreateBufferResource(directXCommon->GetDevice(), sizeof(uint32_t) * 6); //２つ三角形を作るので６個の頂点データ

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
	Microsoft::WRL::ComPtr <ID3D12Resource> materialResourceSprite =
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
	materialDataSprite->uvTransform = MakeIdentity4x4();

	//																			//
	//					TransformationMatrix用のリソースを作る						//
	//																			//

	//WVP用のリソースを作る、Matrix4x4　１つ分のサイズを用意する
	Microsoft::WRL::ComPtr <ID3D12Resource> transformationMatrixResourceSprite = CreateBufferResource(directXCommon->GetDevice(), sizeof(TransformationMatrix));
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
#pragma endregion

	///*-----------------------------------------------------------------------*///
	///									model									///
	///*-----------------------------------------------------------------------*///

#pragma region "Model"

	//モデルデータ作成
	ModelData modelData = LoadObjFile("resources", "plane.obj");

	directXCommon->LoadTextureResourceForSRV(modelData.material.textureFilePath, 2);
	directXCommon->MakeSRV(modelData.material.textureFilePath, 2);

	//																			//
	//							VertexResourceの作成								//
	//																			//
	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResourceModel = CreateBufferResource(directXCommon->GetDevice(), sizeof(VertexData) * modelData.vertices.size());

	//																			//
	//							VertexBufferViewの作成							//
	//																			//
	//頂点バッファビュー作成
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewModel{};
	vertexBufferViewModel.BufferLocation = vertexResourceModel->GetGPUVirtualAddress();			//リソースの戦闘のアドレスから使う
	vertexBufferViewModel.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());	//使用するリソースのサイズは頂点のサイズ
	vertexBufferViewModel.StrideInBytes = sizeof(VertexData);									//1頂点当たりのサイズ

	//																			//
	//							Material用のResourceを作る						//
	//																			//
	// モデル用のマテリアルリソースを作成
	Microsoft::WRL::ComPtr <ID3D12Resource> materialResourceModel = CreateBufferResource(directXCommon->GetDevice(), sizeof(Material));
	Material* materialDataModel = nullptr;
	materialResourceModel->Map(0, nullptr, reinterpret_cast<void**>(&materialDataModel));
	materialDataModel->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialDataModel->enableLighting = true;
	materialDataModel->useLambertianReflectance = false;
	materialDataModel->uvTransform = MakeIdentity4x4();


	//																			//
	//							DirectionalLightのResourceを作る						//
	//																			//

	Microsoft::WRL::ComPtr <ID3D12Resource> directionalLightResourceModel = CreateBufferResource(directXCommon->GetDevice(), sizeof(DirectionalLight));
	//データを書き込む
	DirectionalLight* directionalLightDataModel = nullptr;
	//書き込むためのアドレスを取得
	directionalLightResourceModel->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightDataModel));
	//単位行列を書き込んでおく
	directionalLightDataModel->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightDataModel->direction = { 0.0f,-1.0f,0.0f };
	directionalLightDataModel->intensity = 1.0f;


	//																			//
	//					TransformationMatrix用のリソースを作る						//
	//																			//

	Microsoft::WRL::ComPtr <ID3D12Resource> transformMatrixResourceModel = CreateBufferResource(directXCommon->GetDevice(), sizeof(TransformationMatrix));
	TransformationMatrix* transformMatrixDataModel = nullptr;
	transformMatrixResourceModel->Map(0, nullptr, reinterpret_cast<void**>(&transformMatrixDataModel));
	transformMatrixDataModel->WVP = MakeIdentity4x4();
	transformMatrixDataModel->World = MakeIdentity4x4();

	//																			//
	//						Resourceにデータを書き込む								//
	//																			//
	VertexData* vertexDataModel = nullptr;
	vertexResourceModel->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataModel));						//書き込むためのアドレス取得
	std::memcpy(vertexDataModel, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());	//頂点データをリソースにコピー

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

	Vector3Transform uvTransformSprite{
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
	// モデル用のTransform変数
	Vector3Transform transformModel{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f}
	};

	//ImGuiで使用する変数
	bool useMonsterBall = true;

	//ゲーム開始前に読み込む音声データ
	AudioManager::GetInstance()->LoadWave("resources/Alarm01.wav", "Alarm");
	//tagを利用して再生
	AudioManager::GetInstance()->PlayLoop("Alarm");
	AudioManager::GetInstance()->SetVolume("Alarm",0.1f);


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
		ImGui::Begin("Debug");
		ImGui::Text("Triangle");
		ImGui::ColorEdit4("Triangle_Color", reinterpret_cast<float*>(&materialData->color.x));
		ImGui::DragFloat3("Triangle_translate", &transform.translate.x, 0.01f);
		ImGui::DragFloat3("Triangle_rotate", &transform.rotate.x, 0.01f);

		ImGui::Text("Sphere");
		ImGui::ColorEdit4("Sphere_Color", reinterpret_cast<float*>(&materialDataSphere->color.x));
		ImGui::DragFloat3("Sphere_translate", &transformSphere.translate.x, 0.01f);
		ImGui::DragFloat3("Sphere_rotate", &transformSphere.rotate.x, 0.01f);
		ImGui::DragFloat3("Sphere_LightDirection", &directionalLightDataSphere->direction.x, 0.01f);

		ImGui::Checkbox("useMonsterBall", &useMonsterBall);
		ImGui::Checkbox("useEnableLighting", reinterpret_cast<bool*>(&materialDataSphere->enableLighting));
		ImGui::Checkbox("useLambertianReflectance", reinterpret_cast<bool*>(&materialDataSphere->useLambertianReflectance));

		ImGui::Text("Sprite");
		ImGui::ColorEdit4("Sprite_Color", reinterpret_cast<float*>(&materialDataSprite->color.x));
		ImGui::DragFloat3("Sprite_translate", &transformSprite.translate.x, 0.01f);
		ImGui::DragFloat3("Sprite_rotate", &transformSprite.rotate.x, 0.01f);
		ImGui::DragFloat2("Sprite_UVtranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat2("Sprite_UVscale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
		ImGui::SliderAngle("Sprite_UVrotate", &uvTransformSprite.rotate.z);

		ImGui::Text("Model");
		ImGui::DragFloat3("Model_translate", &transformModel.translate.x, 0.01f);
		ImGui::DragFloat3("Model_rotate", &transformModel.rotate.x, 0.01f);

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

		transformSphere.rotate.y += 0.01f;

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
		//uvTransformの更新
		UpdateUVTransform(uvTransformSprite, materialDataSprite);


		//																			//
		//							Model用のWVP									//
		//																			//

		UpdateMatrix4x4(transformModel, viewProjectionMatrix, transformMatrixDataModel);


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

		//																			//
		//								三角形用の描画									//
		//																			//

#pragma region Triangle

		directXCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());	//マテリアルのCBufferの場所を設定
		directXCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());			//wvp用のCBufferの場所を設定
		directXCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, directXCommon->GetTextureGPUSrvHandles()[0]);	//uvChecker
		directXCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);	//VBを設定
		// 描画！（DrawCall／ドローコール）。３頂点で1つのインスタンス
		directXCommon->GetCommandList()->DrawInstanced(6, 1, 0, 0);

#pragma endregion

		//																			//
		//								Sphereの描画									//
		//																			//
#pragma region Sphere

		//directXCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourceSphere->GetGPUVirtualAddress());	//マテリアルのCBufferの場所を設定
		//directXCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResourceSphere->GetGPUVirtualAddress());			//wvp用のCBufferの場所を設定

		//directXCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResourceSphere->GetGPUVirtualAddress());
		////wvp用のCBufferの場所を設定
		//directXCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, useMonsterBall ? directXCommon->GetTextureSrvHandles()[1] : directXCommon->GetTextureSrvHandles()[0]);
		//directXCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewSphere);	//VBを設定
		//directXCommon->GetCommandList()->IASetIndexBuffer(&indexBufferViewSphere);//Index
		/////indexBufferで表示するため、VertexBufferはコメントアウト
		//// 描画！（DrawCall／ドローコール）。３頂点で1つのインスタンス
		////directXCommon->GetCommandList()->DrawInstanced(16 * 16 * 6, 1, 0, 0);
		//directXCommon->GetCommandList()->DrawIndexedInstanced(16 * 16 * 6, 1, 0, 0, 0);

#pragma endregion
		//																			//
		//								Spriteの描画									//
		//																			//

#pragma region Sprite

		//directXCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());	//マテリアルのCBufferの場所を設定
		//directXCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);//Vertex
		//directXCommon->GetCommandList()->IASetIndexBuffer(&indexBufferViewSprite);//Index

		//directXCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, directXCommon->GetTextureSrvHandles()[0]);

		////TransformMatrixCBufferの場所を設定
		//directXCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());

		//// 描画（DrawCall／ドローコール)
		////三角形を二つ描画するので6つ
		//directXCommon->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);

#pragma endregion

		//																			//
		//								Modelの描画									//
		//																			//

#pragma region Model

		directXCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourceModel->GetGPUVirtualAddress());
		directXCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformMatrixResourceModel->GetGPUVirtualAddress());
		directXCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResourceModel->GetGPUVirtualAddress()); // 同じライトを使用
		directXCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, directXCommon->GetTextureGPUSrvHandles()[2]); // テクスチャ
		directXCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewModel);
		// インデックスバッファがない場合は直接頂点で描画
		directXCommon->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);

#pragma endregion

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

	// AudioManagerの終了処理
	AudioManager::GetInstance()->Finalize();

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