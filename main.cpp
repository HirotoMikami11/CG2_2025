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
#include<cassert>


///*-----------------------------------------------------------------------*///
//																			//
///						ウィンドウプロシージャここから						   ///
//																			//
///*-----------------------------------------------------------------------*///

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg,
	WPARAM wparam, LPARAM lparam) {
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

// 出力ウィンドウに文字を出す関数
static void Log(const std::string& message) {
	OutputDebugStringA(message.c_str());
}
// 出力ウィンドウに文字を出し、ログファイルに書き込む関数
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



//CrashHandlerの登録
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
	MINIDUMP_EXCEPTION_INFORMATION minidumpInformation{ 0 };
	minidumpInformation.ThreadId = threadId;
	minidumpInformation.ExceptionPointers = exception;
	minidumpInformation.ClientPointers = TRUE;

	//Dumpを出力　MiniDumpNormalは最低限の情報を出力するフラグ
	MiniDumpWriteDump(GetCurrentProcess(), processId, dumpFileHandle, MiniDumpNormal, &minidumpInformation, nullptr, nullptr);
	//他に関連付けられているSEH例外ハンドラがあれば実行、通常はプロセスを終了する
	return EXCEPTION_EXECUTE_HANDLER;
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

	//ディスクリプタヒープの生成
	ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc{};
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;	//レンダーターゲットビュー用
	rtvDescriptorHeapDesc.NumDescriptors = 2;						//ダブルバッファ用に２つ。多くても構わない
	hr = device->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
	// ディスクリプタヒープが作れなかったので起動できない
	assert(SUCCEEDED(hr));
	Log(logStream, "Complete create DescriptorHeap!!\n");//ディスクリプタヒープ生成完了のログを出す

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

			//ゲームの処理

			///*-----------------------------------------------------------------------*///
			//																			//
			///				画面をクリアする処理が含まれたコマンドリストを作る				   ///
			//																			//
			///*-----------------------------------------------------------------------*///

			//これから書き込むバックバッファのインデックスの取得
			UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();
			//描画先のRTVを設定する
			commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, nullptr);
			// 指定した色で画面全体をクリアする
			float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };//青っぽい色。RGBAの順
			commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);
			//コマンドリストの内容を確定させる。全てのコマンドを積んでからCloseすること
			hr = commandList->Close();
			assert(SUCCEEDED(hr));//ダメなら起動できない
			//Log(logStream, "clear color to the command list and closed.\n");//コマンドリスト画面クリアの内容確定のログを出す

			///コマンドをキックする
			ID3D12CommandList* commandLists[] = { commandList };
			commandQueue->ExecuteCommandLists(1, commandLists);
			//GPUとOSに画面の交換を行うように通知
			swapChain->Present(1, 0);
			//次のフレーム用のコマンドリストを準備
			hr = commandAllocator->Reset();
			assert(SUCCEEDED(hr));
			hr = commandList->Reset(commandAllocator, nullptr);
			assert(SUCCEEDED(hr));
			//Log(logStream, "Command list has been reset, ready for the next frame.\n");//コマンドリストリセット、次フレームの準備完了のログを出す

		}
	}

	return 0;


}
