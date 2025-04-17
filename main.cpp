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

///*-----------------------------------------------------------------------*///
//																			//
///						ウィンドウプロシージャここから						   ///
//																			//
///*-----------------------------------------------------------------------*///

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

	//ImGuiにメッセージを渡し、マウスやキーボードで操作できるようにする
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}


	//メッセージに応じてゲーム固有の処理を行う
	switch (msg) {
		//ウィンドウが破棄された
	case WM_DESTROY:
		//OSに対してアプリの終了を伝える
		PostQuitMessage(0);
		return 0;
	}
	//標準のメッセージ処理を行う
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

///*-----------------------------------------------------------------------*///
//																			//
///									ログ関連の関数							   ///
//																			//
///*-----------------------------------------------------------------------*///

/// 出力ウィンドウに文字を出す関数
static void Log(const std::string& message) {
	OutputDebugStringA(message.c_str());
}
/// 出力ウィンドウに文字を出し、ログファイルに書き込む関数
static void Log(std::ostream& os, const std::string& message) {
	os << message << std::endl;
	OutputDebugStringA(message.c_str());
}


// string -> wstringに変換する関数
static std::wstring ConvertString(const std::string& str) {
	if (str.empty()) {
		return std::wstring();
	}

	auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
	if (sizeNeeded == 0) {
		return std::wstring();
	}
	std::wstring result(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
	return result;
}
// wstring -> stringに変換する関数
static std::string ConvertString(const std::wstring& str) {
	if (str.empty()) {
		return std::string();
	}

	auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
	if (sizeNeeded == 0) {
		return std::string();
	}
	std::string result(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
	return result;
}



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





/// <summary>
///  DXCを使ってShaderをコンパイルする関数
/// </summary>
/// <param name="filePath">CompileするShaderファイルへのパス</param>
/// <param name="profile">Compileに使用するProfile</param>
/// <param name="dxcUtils">DxcUtil</param>
/// <param name="dxcCompiler">DxcCompiler</param>
/// <param name="includeHandler"></param>
/// <returns></returns>
static IDxcBlob* CompileShader(
	const std::wstring& filePath,
	const wchar_t* profile,
	IDxcUtils* dxcUtils,
	IDxcCompiler3* dxcCompiler,
	IDxcIncludeHandler* includeHandler)
{
	//「これからシェーダーをコンパイルする」とログに出す
	Log(ConvertString(std::format(L"Begin CompileShader, path:{},profile:{}\n", filePath, profile)));

	///hlslファイルを読み込む
	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	//読めなかったら停止する
	assert(SUCCEEDED(hr));

	//読み込んだファイルの内容を設定する
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;//UTF8の文字コードであることを通知

	///Compileする
	LPCWSTR arguments[] = {
		filePath.c_str(),		//コンパイル対象のhlslファイル名
		L"-E",L"main",			//エントリーポイントの指定。基本的にmain以外にはしない
		L"-T",profile,			//ShaderProfileの設定
		L"-Zi",L"Qembed_debug"	//デバッグの情報を埋め込む	(L"-Qembed_debug"でエラー)
		L"-Od",					//最適化を外しておく
		L"-Zpr",				//メモリレイアウトは行優先
	};
	//実際にShaderをコンパイルする
	IDxcResult* shaderResult = nullptr;
	hr = dxcCompiler->Compile(
		&shaderSourceBuffer,			// 読み込んだファイル
		arguments,			            // コンパイルオプション
		_countof(arguments),            // コンパイルオプションの数
		includeHandler,	            // includeが含まれた諸々
		IID_PPV_ARGS(&shaderResult)     // コンパイル結果
	);

	//コンパイルエラーではなくdxcが起動できないなど致命的な状況のとき停止
	assert(SUCCEEDED(hr));

	///警告・エラーがでていないか確認する
		//警告・エラーが出ていたらログに出して停止する
	IDxcBlobUtf8* shaderError = nullptr;
	if (shaderError != nullptr && shaderError->GetStringLength() != 0)
	{
		Log(shaderError->GetStringPointer());
		assert(false);
	}
	///Compile結果を受け取って返す
	//コンパイル結果から実行用のバイナリ部分を取得
	IDxcBlob* shaderBlob = nullptr;//Blob = Binary Large OBject(バイナリーデータの塊)
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	//成功したログを出す
	Log(ConvertString(std::format(L"Compile Succesed,path:{},profile:{}\n", filePath, profile)));
	//もう使わないリソースを開放
	shaderSource->Release();
	shaderResult->Release();

	//実行用のバイナリを返却
	return shaderBlob;
}


/// <summary>
/// Resourceを生成する関数
/// </summary>
/// <param name="device">ID3D12Device</param>
/// <param name="sizeInBytes">リソースサイズ</param>
/// <returns></returns>
static ID3D12Resource* CreateBufferResource(ID3D12Device* device, size_t sizeInBytes) {
	//リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;//UploadHeapを使う
	//リソースの設定
	D3D12_RESOURCE_DESC ResourceDesc{};
	//バッファリソース。テクスチャの場合はまた別の設定をする
	ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	ResourceDesc.Width = sizeInBytes;//リソースサイズ
	//バッファの場合はこれらは1にする決まり
	ResourceDesc.Height = 1;
	ResourceDesc.DepthOrArraySize = 1;
	ResourceDesc.MipLevels = 1;
	ResourceDesc.SampleDesc.Count = 1;
	//バッファの場合はこれにする決まり
	ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//実際にリソースを生成
	ID3D12Resource* Resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&ResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&Resource)
	);
	assert(SUCCEEDED(hr));


	return Resource;
}

/// <summary>
/// DescriptorHeapを作成する関数
/// </summary>
/// <param name="device"ID3D12Device*</param>
/// <param name="heapType">作成するヒープの種類</param>
/// <param name="numDescriptors">ディスクリプタ数</param>
/// <param name="shaderVisible">シェーダーから参照可能にするかのフラグ</param>
/// <returns></returns>
static ID3D12DescriptorHeap* CreateDesctiptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible) {
	//ディスクリプタヒープの生成
	ID3D12DescriptorHeap* descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	// ディスクリプタヒープが作れなかったので起動できない
	assert(SUCCEEDED(hr));
	return descriptorHeap;
}

static DirectX::ScratchImage LoadTexture(const std::string& filePath) {
	//テクスチャファイルを読んでプログラムで扱える
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	// ミップマップ
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0,
		mipImages);
	assert(SUCCEEDED(hr));
	// ミップマップ付きのデータを返す
	return mipImages;
}

static ID3D12Resource* CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata) {
	//metadataを基にResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{ };
	resourceDesc.Width = UINT(metadata.width);								//textureの幅
	resourceDesc.Height = UINT(metadata.height);							//textureの高さ
	resourceDesc.MipLevels = UINT16(metadata.mipLevels);					//mipmapの数
	resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize);				//奥行 or配列Textureの配列数
	resourceDesc.Format = metadata.format;									//textureのFormat
	resourceDesc.SampleDesc.Count = 1;										//サンプリングカウント。1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);	//textureの次元数

	//利用するHeapの設定（非常に特殊な運用方法）
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;							//細かい設定を行う
	//heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;	//WriteBackポリシーでCPUアクセス可能
	//heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;				//プロセッサの近くに配置

	//Resourceの生成
	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,							//heapの設定
		D3D12_HEAP_FLAG_NONE,						//heapの特殊な設定。特になし
		&resourceDesc,								//resourceの設定
		D3D12_RESOURCE_STATE_COPY_DEST,			//初回のresourceState。textureは基本読むだけ
		nullptr,									//clear最適地。使わないのでnullptr
		IID_PPV_ARGS(&resource));					//作成するResourceポインタへのポインタ
	assert(SUCCEEDED(hr));
	return resource;

}
//
//void UploadTextureData(ID3D12Resource* texture,
//	const DirectX::ScratchImage& mipImages)
//{
//	//Meta情報を取得
//	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
//	//全MipMapについて
//	for (size_t mipLevel = 0; mipLevel < metadata.mipLevels; ++mipLevel) {
//		//MipMapLevelを指定して各Imageを取得
//		const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);
//		//textureに転送
//		HRESULT hr = texture->WriteToSubresource(
//			UINT(mipLevel),
//			nullptr,				//全領域へコピー
//			img->pixels,			//元データのアドレス
//			UINT(img->rowPitch),	//１ラインサイズ
//			UINT(img->slicePitch)	//１枚サイズ
//		);
//		assert(SUCCEEDED(hr));
//	}
//
//}


[[nodiscard]]
static ID3D12Resource* UploadTextureData(ID3D12Resource* texture,
	const DirectX::ScratchImage& mipImages,
	ID3D12Device* device,
	ID3D12GraphicsCommandList* commandList) {
	std::vector<D3D12_SUBRESOURCE_DATA> subresource;
	DirectX::PrepareUpload(device, mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresource);
	uint64_t intermediateSize = GetRequiredIntermediateSize(texture, 0, UINT(subresource.size()));
	ID3D12Resource* intermediateResource = CreateBufferResource(device, intermediateSize);
	UpdateSubresources(commandList, texture, intermediateResource, 0, 0, UINT(subresource.size()), subresource.data());
	//textureへの転送後は利用できるよう、D3D12_RESOURCE_STATE_COPY_DESTからD3D12_RESOURCE_STATE_GENERIC_READへResourceStateを変更する
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	commandList->ResourceBarrier(1, &barrier);
	return intermediateResource;

	//p15EXまで完了
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

	//COMの初期化
	CoInitializeEx(0, COINIT_MULTITHREADED);


	///
	///	ログをファイルに書き込む
	///

	//ログにディレクトリを用意する
	std::filesystem::create_directory("logs");
	//現在時刻を取得（UTC時刻）
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	//ログファイルの名前にコンマ何秒はいらないので、削って秒にする
	std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>
		nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
	//日本時間（PCの設定時間）に変換
	std::chrono::zoned_time localTime{ std::chrono::current_zone(),nowSeconds };
	//formatを使って年月日_時分秒の文字列に変換
	std::string dateString = std::format("{:%Y%m%d_%H%M%S}", localTime);
	//時刻を使ってファイル名を決定
	std::string logFilePath = std::string("logs/") + dateString + ".log";
	//ファイルを作って書き込み準備
	std::ofstream logStream(logFilePath);


	//出力ウィンドウへの文字出力
	Log(logStream, "Hello,DirectX!/n");

	///
	/// ウィンドウクラスを登録する
	///

	WNDCLASS wc{};
	//ウィンドウプロシージャ
	wc.lpfnWndProc = WindowProc;

	//ウィンドウクラス名
	wc.lpszClassName = L"CG2WindowClass";
	//インスタンスハンドル
	wc.hInstance = GetModuleHandle(nullptr);
	//カーソル
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	//ウィンドウクラスを登録する
	RegisterClass(&wc);


	// クライアント領域のサイズ
	const int32_t kClientWidth = 1280;
	const int32_t kClientHeight = 720;

	//　ウィンドウサイズを表す構造体にクライアント領域を入れる
	RECT wrc = { 0,0,kClientWidth,kClientHeight };

	//クライアント領域をもとに実際のサイズにwrcを変更してもらう
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	//ウィンドウの生成
	HWND hwnd = CreateWindow(
		wc.lpszClassName,		//利用するクラス名
		L"CG2",					//タイトルバーの文字
		WS_OVERLAPPEDWINDOW,	//よく見るウィンドウスタイル
		CW_USEDEFAULT,			//表示X座標
		CW_USEDEFAULT,			//表示Y座標
		wrc.right - wrc.left,	//ウィンドウの横幅
		wrc.bottom - wrc.top,	//ウィンドウの縦幅
		nullptr,				//親ウィンドウハンドル
		nullptr,				//メニューハンドル
		wc.hInstance,			//インスタンスハンドル
		nullptr);				//オプション

	//ウィンドウを表示する関数
	ShowWindow(hwnd, SW_SHOW);


	///*-----------------------------------------------------------------------*///
	//																			//
	///									DebugLayer							   ///
	//																			//
	///*-----------------------------------------------------------------------*///
#ifdef _DEBUG
	///DirectX12初期化前に行う必要があるため、ウィンドウを作った後すぐの位置
	ID3D12Debug1* debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		//デバッグレイヤーを有効化する
		debugController->EnableDebugLayer();
		//さらにGPU側でもチェックを行うにする
		debugController->SetEnableGPUBasedValidation(TRUE);
	}

#endif

	///*-----------------------------------------------------------------------*///
	//																			//
	///									DXGIFactory							   ///
	//																			//
	///*-----------------------------------------------------------------------*///

	//DXGIファクトリーの生成
	IDXGIFactory7* dxgiFactory = nullptr;
	//HRESULTはwindows系のエラーコードであり、
	//関数が成功したかどうかをSUCEEDEDマクロで判定できる
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	//初期化の根本的な部分でエラーが出た場合はプログラムが間違っているか、どうにもできない場合が多いため、assertにする
	assert(SUCCEEDED(hr));


	///
	///	使用するアダプタ（大まかにGPU）を決定
	/// 

	//使用するアダプタ用の変数、最初はnullptrを入れる
	IDXGIAdapter4* useAdapter = nullptr;
	//良い順にアダプタを摘む
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i,
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) !=
		DXGI_ERROR_NOT_FOUND; ++i) {
		//アダプターの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));//取得できないと一大事なのでassert
		//ソフトウェアアダプタでなければ採用する
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)) {
			//採用したアダプタの情報をログに出力。wstringの方なので注意

			///コンバートストリングしてstr変化
			Log(logStream, ConvertString(std::format(L"Use Adapater:{}\n", adapterDesc.Description)));
			break;
		}
		useAdapter = nullptr;//ソフトウェアアダプタの場合は見なかったことにする


	}
	//適切なアダプタが見つからなかったので起動できない
	assert(useAdapter != nullptr);

	///
	///	D3D12Deviceの生成
	/// 

	ID3D12Device* device = nullptr;
	//機能レベルとログ出力用の文字列
	D3D_FEATURE_LEVEL featureLevels[] = {
D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
	};

	const char* featureLevelStrings[] = { "12.2","12.1","12.0" };
	//高い順に生成できるか試していく
	for (size_t i = 0; i < _countof(featureLevels); ++i) {
		//採用したアダプターでデバイスを生成
		hr = D3D12CreateDevice(useAdapter, featureLevels[i], IID_PPV_ARGS(&device));
		//指定した機能レベルでデバイスが生成できたかを確認
		if (SUCCEEDED(hr)) {
			//生成できたのでログ出力を行ってループを抜ける
			Log(logStream, std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
			break;
		}
	}
	//デバイスの生成がうまくいかなかったので起動できない。
	assert(device != nullptr);
	Log(logStream, "Complete create D3D12Device!!\n");//初期化完了のログを出す



	///	DirectX12のエラー・警告が出た時止まるようにする

#ifdef _DEBUG	//デバッグ時
	ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		// ヤバイエラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		// エラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		//警告時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true); //解放忘れが判明した時、一時的にコメントアウトすれば、ログに情報を出力できる。確認できたらすぐ元に戻す。
		//解放
		infoQueue->Release();

		//抑制するメッセージのID
		D3D12_MESSAGE_ID denyIds[] = {
			//Windows11のDXGIデバッグレイヤーとDX12デバッグレイヤーの相互作用バグによるエラーメッセージ
		D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		//抑制するレベル
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		//指定したメッセージの表示を抑制する
		infoQueue->PushStorageFilter(&filter);

	}
#endif


	///*-----------------------------------------------------------------------*///
	//																			//
	///					CommandQueueとCommandListを生成						   ///
	//																			//
	///*-----------------------------------------------------------------------*///

	//コマンドキュー(GPUに命令をするもの)を作成
	ID3D12CommandQueue* commandQueue = nullptr;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	//コマンドキューの生成が上手くいかなかったので起動できない
	assert(SUCCEEDED(hr));
	Log(logStream, "Complete create commandQueue!!\n");//コマンドキュー生成完了のログを出す

	//コマンドアロケータ（コマンドリスト（まとまった命令郡）保存用のメモリ管理するもの）
	ID3D12CommandAllocator* commandAllocator = nullptr;
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	//コマンドアロケータの生成が上手くいかなかったので起動できない
	assert(SUCCEEDED(hr));
	Log(logStream, "Complete create commandAllocator!!\n");//コマンドアロケータ生成完了のログを出す

	// コマンドリスト（まとまった命令郡）を生成する
	ID3D12GraphicsCommandList* commandList = nullptr;
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList));
	//コマンドリストの生成がうまくいかなかったので起動できない 
	assert(SUCCEEDED(hr));
	Log(logStream, "Complete create commandList!!\n");//コマンドリスト生成完了のログを出す


	///*-----------------------------------------------------------------------*///
	//																			//
	///							SwapChainを生成								   ///
	//																			//
	///*-----------------------------------------------------------------------*///

	//スワップチェーン（バブルバッファリングの２枚の画面を管理するもの）を生成する
	IDXGISwapChain4* swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = kClientWidth;								//画面の幅、ウィンドウのクライアント領域を同じものにしておく
	swapChainDesc.Height = kClientHeight;							//画面の高さ、ウィンドウのクライアント領域を同じものにしておく
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;				//色の形式
	swapChainDesc.SampleDesc.Count = 1;								//マルチサンプル市内
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	//描画のターゲットとして利用する
	swapChainDesc.BufferCount = 2;									//ダブルバッファ
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;		//モニタに移したら、中身を破棄
	//コマンドキュー、ウィンドウハンドル、設定を渡して生成する
	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue, hwnd, &swapChainDesc, nullptr, nullptr,
		reinterpret_cast<IDXGISwapChain1**>(&swapChain));
	//スワップチェーンの生成がうまくいかなかったので起動できない 
	assert(SUCCEEDED(hr));
	Log(logStream, "Complete create swapChain!!\n");//スワップチェーン生成完了のログを出す


	///*-----------------------------------------------------------------------*///
	//																			//
	///							DescriptorHeapを生成							   ///
	//																			//
	///*-----------------------------------------------------------------------*///

	//RTV用ヒープでディスクリプタの数は2，RTVはShader内で触れるものではないのでShaderVisibleはfalse
	ID3D12DescriptorHeap* rtvDescriptorHeap = CreateDesctiptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
	Log(logStream, "Complete create RTVDescriptorHeap!!\n");//RTVディスクリプタヒープ生成完了のログを出す

	//SRV用ヒープでディスクリプタの数は128，SRVはShader内で触れるものなのでShaderVisibleはtrue
	ID3D12DescriptorHeap* srvDescriptorHeap = CreateDesctiptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
	Log(logStream, "Complete create SRVDescriptorHeap!!\n");//SRVディスクリプタヒープ生成完了のログを出す

	//
	///　SwapChainからResourceを引っ張ってくる
	//

	//　SwapChainからResourceを引っ張ってくる
	ID3D12Resource* swapChainResources[2] = { nullptr };
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
	//　うまく取得できなければ起動できない
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	assert(SUCCEEDED(hr));

	///*-----------------------------------------------------------------------*///
	//																			//
	///									RTVを生成							   ///
	//																			//
	///*-----------------------------------------------------------------------*///

	//RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;			//出力結果をSRGBに変換して書き込む
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;		//2Dテクスチャとして書き込む
	//ディスクリプタの先頭を取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	//RTVを2つ作るのでディスクリプタを2つ用意
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	//まず1つ目を作る。1つ目は最初のところに作る。作る場所を指定してあげる必要がある
	rtvHandles[0] = rtvStartHandle;
	device->CreateRenderTargetView(swapChainResources[0], &rtvDesc, rtvHandles[0]);
	//2つ目のディスクリプタハンドルを得る（自力で）
	rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//2つ目を作る
	device->CreateRenderTargetView(swapChainResources[1], &rtvDesc, rtvHandles[1]);


	///*-----------------------------------------------------------------------*///
	//																			//
	///									SRVを生成							   ///
	//																			//
	///*-----------------------------------------------------------------------*///
	//textureを読んで転送する
	DirectX::ScratchImage mipImages = LoadTexture("resources/uvChecker.png");
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	ID3D12Resource* textureResource = CreateTextureResource(device, metadata);
	ID3D12Resource* intermediateResource = UploadTextureData(textureResource, mipImages, device, commandList);

	//metadataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	//SRVを作成するDescriptorHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	//先頭はImGuiが使っているのでその次を使う
	textureSrvHandleCPU.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);//現在のディスクリプタヒープの次の位置を指定？
	textureSrvHandleGPU.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);//現在のディスクリプタヒープの次の位置を指定？

	//SRVの生成
	device->CreateShaderResourceView(textureResource, &srvDesc, textureSrvHandleCPU);



	///*-----------------------------------------------------------------------*///
	//																			//
	///							FenceとEventを生成する							///
	//																			//
	///*-----------------------------------------------------------------------*///

	//初期値0でFence（CPUとGPUの同期をとれるもの）を作る
	ID3D12Fence* fence = nullptr;
	uint64_t fenceValue = 0;
	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));	//Fenceが生成できなかったので起動できない
	Log(logStream, "Complete create fence!!\n");//フェンス生成完了のログを出す

	//FenceのSignal(GPUに指定の位置で指定の値を書き込んでもらう命令)を待つためのEvent(メッセージ)を作成する
	HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent != nullptr);	 //作成が上手くできなかったら起動できない

	///*-----------------------------------------------------------------------*///
	//																			//
	///									DXCの初期化								///
	//																			//
	///*-----------------------------------------------------------------------*///

	//dxcCompilerの初期化
	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));

	//現時点でincludeはしないが、includeに対応するための設定を行っておく
	IDxcIncludeHandler* includeHandler = nullptr;
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));
	Log(logStream, "Complete: DXC compiler initialization.\n"); //DXC初期化完了

	///*-----------------------------------------------------------------------*///
	//																			//
	///								PSOを生成する									///
	//																			//
	///*-----------------------------------------------------------------------*///

	//																			//
	//								RootSignature作成							//
	//																			//

	//rootSignature=具体的にshaderがどこでデータを読めばいいのかという情報をまとめたもの
	//RootSignatureの作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//DescriptorRange=shaderがアクセスするリソース(テクスチャ、バッファなど)をまとめて定義する
	//DescriptorRangeを作る
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;//0から始まる
	descriptorRange[0].NumDescriptors = 1;//数は一つだけ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//Offsetは自動計算

	//RootParameter作成。(pixelShaderのMaterialとVertexShaderのTransformの2つ)
	D3D12_ROOT_PARAMETER rootParameters[3] = {};
	//PixelShader
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;	//ConstantBufferViewを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixelShaderで使う
	rootParameters[0].Descriptor.ShaderRegister = 0;					//レジスタ番号0とバインド(PSのgMaterial(b0)と結びつける)
	//VertexShader
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;	//ConstantBufferViewを使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; //PixelShaderで使う
	rootParameters[1].Descriptor.ShaderRegister = 0;					//レジスタ番号0とバインド(VSのgTransformationMatrix(b0)と結びつける)

	//DescriptorTableを作成する
	//DescriptorTableはDescriptorRangeをまとめたもの
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;	//DescriptorTableを使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;				//PixelShaderで使う
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;			//Tableの中身の配列を指定
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);//Tableで利用する数


	//Samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;			//バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		//0~1の範囲外をリピート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;		//比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;						//あるだけmipmapを使用する
	staticSamplers[0].ShaderRegister = 0;									//レジスタ番号を使う
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	//pixelShaderを使う
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);


	descriptionRootSignature.pParameters = rootParameters;				//ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters);	//配列の長さ

	//シリアライズしてバイナリにする
	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Log(logStream, reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	//バイナリを元に生成
	ID3D12RootSignature* rootSignature = nullptr;
	hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	//																			//
	//							InputLayoutの設定								//
	//																			//

	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};

	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	//																			//
	//							BlendStateの設定									//
	//																			//

	D3D12_BLEND_DESC blendDesc{};
	//全ての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;


	//																			//
	//						RasterizerStateの設定								//
	//																			//

	D3D12_RASTERIZER_DESC rasterizerDesc{};
	//裏面（時計回り）を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	//三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	//Shaderをコンパイル
	IDxcBlob* vertexShaderBlob = CompileShader(L"Object3d.VS.hlsl",
		L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(vertexShaderBlob != nullptr);

	IDxcBlob* pixelShaderBlob = CompileShader(L"Object3d.PS.hlsl",
		L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(pixelShaderBlob != nullptr);


	///PSOの作成

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature;					// RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;					// InputLayout 
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
		vertexShaderBlob->GetBufferSize() };									// VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
		pixelShaderBlob->GetBufferSize() };										// PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc;							// BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;				// RasterizerState

	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジ (形状)のタイプ。三角形 
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定(気にしなくて良い)
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// 実際に生成
	ID3D12PipelineState* graphicsPipelineState = nullptr;
	hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));
	//生成できなかったら起動できない
	Log(logStream, "Complete create PSO!!\n");//PSO生成完了のログを出す

	///*-----------------------------------------------------------------------*///
	//																			//
	///									VertexResource						   ///
	//																			//
	///*-----------------------------------------------------------------------*///

	//実際に頂点リソースを生成
	ID3D12Resource* vertexResource = CreateBufferResource(device, sizeof(VertexData) * 3);

	//																			//
	//							VertexBufferViewの作成							//
	//																			//
	//頂点バッファビューを作成
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	//リソースの戦闘のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	//仕様数リソースのサイズは頂点3つ分のサイズ
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 3;
	//1頂点当たりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	//																			//
	//							Material用のResourceを作る						//
	//																			//

	//マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意
	ID3D12Resource* materialResource =
		CreateBufferResource(device, sizeof(VertexData));
	//マテリアルデータに書き込む
	Vector4* materialData = nullptr;
	//書き込むためのアドレスを取得
	materialResource->
		Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	//今回は赤を書き込んでみる
	*materialData = Vector4(1.0f, 1.0f, 1.0f, 1.0f);


	//																			//
	//					TransformationMatrix用のリソースを作る						//
	//																			//

	//WVP用のリソースを作る、Matrix4x4　１つ分のサイズを用意する
	ID3D12Resource* wvpResource = CreateBufferResource(device, sizeof(Matrix4x4));
	//データを書き込む
	Matrix4x4* wvpData = nullptr;
	//書き込むためのアドレスを取得
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	//単位行列を書き込んでおく
	*wvpData = MakeIdentity4x4();

	//																			//
	//						Resourceにデータを書き込む								//
	//																			//

	//頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	//書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	//左下
	vertexData[0].position = { -0.5f,-0.5f,0.0f,1.0f };
	vertexData[0].texcoord = { 0.0f,1.0f };
	//上
	vertexData[1] = { 0.0f,0.5f,0.0f,1.0f };
	vertexData[1].texcoord = { 0.5f,0.0f };
	//右下
	vertexData[2] = { 0.5f,-0.5f,0.0f,1.0f };
	vertexData[2].texcoord = { 1.0f,1.0f };

	///*-----------------------------------------------------------------------*///
	//																			//
	///							ViewportとScissor							   ///
	//																			//
	///*-----------------------------------------------------------------------*///

	//ビューポート
	D3D12_VIEWPORT viewport{};
	//クライアント領域のサイズと一緒にして画面全体に表示
	viewport.Width = kClientWidth;
	viewport.Height = kClientHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	//シザー矩形
	D3D12_RECT scissorRect{};
	//基本的にビューポートと同じ矩形が構成されるようにする
	scissorRect.left = 0;
	scissorRect.right = kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = kClientHeight;

	///*-----------------------------------------------------------------------*///
	//																			//
	///								ImGuiの初期化								   ///
	//																			//
	///*-----------------------------------------------------------------------*///
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX12_Init(
		device,
		swapChainDesc.BufferCount,
		rtvDesc.Format,
		srvDescriptorHeap,
		srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart()
	);

	//								　変数宣言									//

	//Transform変数を作る
	Vector3Transform transform{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};

	//WorldViewProjectionMatrixを作る
	Vector3Transform cameraTransform{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,-5.0f}
	};



	///*-----------------------------------------------------------------------*///
	//																			//
	///									メインループ							   ///
	//																			//
	///*-----------------------------------------------------------------------*///

	MSG msg{};
	//ウィンドウのxボタンが押されるまでループ
	while (msg.message != WM_QUIT) {
		//Windowにメッセージが来てたら最優先で処理
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			//							　ゲームの処理										//


			//								更新処理										//

			//ImGuiにフレームが始まることを伝える
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			//開発用UIの処理
			ImGui::ShowDemoWindow();
			ImGui::Begin("Material Color");
			if (ImGui::ColorEdit4("Color", reinterpret_cast<float*>(&materialData->x))) {
				// 色が変更された時の処理（必要ならここで更新通知など）
			}
			ImGui::End();
			//																			//
			//							三角形を動かす										//
			//																			//

			//三角形を回転させる
			transform.rotate.y += 0.03f;
			//viewprojectionを計算
			Matrix4x4 viewProjectionMatrix = MakeViewProjectionMatrix(cameraTransform, (float(kClientWidth) / float(kClientHeight)));
			//行列の更新
			UpdateMatrix4x4(transform, viewProjectionMatrix, wvpData);

			//ImGuiの内部コマンドを生成する(描画処理に入る前)
			ImGui::Render();

			//								描画処理										//
			///*-----------------------------------------------------------------------*///
			//																			//
			///				画面をクリアする処理が含まれたコマンドリストを作る				   ///
			//																			//
			///*-----------------------------------------------------------------------*///

			//これから書き込むバックバッファのインデックスの取得
			UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();


			///TransitionBarrierの設定
			D3D12_RESOURCE_BARRIER barrier{};
			//今回のバリアはTransition
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			// Noneにしておく
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			//バリアを張る対象のリソース。現在のバックバッファに対して行う
			barrier.Transition.pResource = swapChainResources[backBufferIndex];
			// 遷移前（現在）のResourceState
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
			//遷移後のResourceState
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			//TransitionBarrierを張る
			commandList->ResourceBarrier(1, &barrier);


			//描画先のRTVを設定する
			commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, nullptr);
			// 指定した色で画面全体をクリアする
			float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };//青っぽい色。RGBAの順
			commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);

			//	描画用のDesctiptorHeapの設定
			ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap };
			commandList->SetDescriptorHeaps(1, descriptorHeaps);


			///*-----------------------------------------------------------------------*///
			//																			//
			///							描画に必要コマンドを積む						   ///
			//																			//
			///*-----------------------------------------------------------------------*///

			commandList->RSSetViewports(1, &viewport);					//Viewportを設定
			commandList->RSSetScissorRects(1, &scissorRect);			//Scissorを設定

			// RootSignatureを設定。PSOに設定しているけど別途設定（PSOと同じもの）が必要
			commandList->SetGraphicsRootSignature(rootSignature);
			commandList->SetPipelineState(graphicsPipelineState);		//PSOを設定
			commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());	//マテリアルのCBufferの場所を設定
			commandList->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());			//wvp用のCBufferの場所を設定
			commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
			commandList->IASetVertexBuffers(0, 1, &vertexBufferView);	//VBを設定

			// 形状を設定。PSOに設定しているものとはまた別。RootSignatureと同じように同じものを設定すると考えておけばいい
			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			// 描画！（DrawCall／ドローコール）。３頂点で1つのインスタンス
			commandList->DrawInstanced(3, 1, 0, 0);

			//実際のcommandListのImGuiの描画コマンドを積む
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);

			//	画面に描く処理はすべて終わり、画面に映すので、状態を遷移
			//	今回はRenerTargetからPresentにする
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
			//TransitionBarrierを張る
			commandList->ResourceBarrier(1, &barrier);

			
			//コマンドリストの内容を確定させる。全てのコマンドを積んでからCloseすること
 			hr = commandList->Close();
			assert(SUCCEEDED(hr));//ダメなら起動できない

			///コマンドをキックする
			ID3D12CommandList* commandLists[] = { commandList };
			commandQueue->ExecuteCommandLists(1, commandLists);
			//GPUとOSに画面の交換を行うように通知
			swapChain->Present(1, 0);

			///GPUにSignalを送る
			//Fenceの値を更新
			fenceValue++;
			//GPUがここまでたどり着いたときに、Fenceの値を指定した値に代入するようにSignalを送る
			commandQueue->Signal(fence, fenceValue);

			//Fenceの値が指定したSignal値にたどり着いているか確認する
			//GetCompleteValueの初期値はFence作成時に渡した初期値
			if (fence->GetCompletedValue() < fenceValue) {
				//指定したSignalにたどり着いていないので，たどり着くまで待つようにイベントを設定する
				fence->SetEventOnCompletion(fenceValue, fenceEvent);
				//イベント待つ
				WaitForSingleObject(fenceEvent, INFINITE);
			}

			//次のフレーム用のコマンドリストを準備
			hr = commandAllocator->Reset();
			assert(SUCCEEDED(hr));
			hr = commandList->Reset(commandAllocator, nullptr);
			assert(SUCCEEDED(hr));
			intermediateResource->Release();

		}
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
	materialResource->Release();	//マテリアル
	wvpResource->Release();			//wvp
	graphicsPipelineState->Release();
	signatureBlob->Release();
	if (errorBlob) {
		errorBlob->Release();
	}
	rootSignature->Release();
	pixelShaderBlob->Release();
	vertexShaderBlob->Release();

	textureResource->Release();
	intermediateResource->Release();

	//オブジェクトの解放処理
	CloseHandle(fenceEvent);
	fence->Release();
	rtvDescriptorHeap->Release();
	srvDescriptorHeap->Release();
	swapChainResources[0]->Release();
	swapChainResources[1]->Release();
	swapChain->Release();
	commandList->Release();
	commandAllocator->Release();
	commandQueue->Release();
	device->Release();
	useAdapter->Release();
	dxgiFactory->Release();
#ifdef _DEBUG
	debugController->Release();
#endif // _DEBUG
	CloseWindow(hwnd);


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