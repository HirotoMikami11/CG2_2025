#pragma once
#include <cstdint>

/// <summary>
/// グラフィックス関連の設定値を集約するクラス
/// </summary>
class GraphicsConfig {
public:
	// ウィンドウサイズ
	static const uint32_t kClientWidth = 1280;
	static const uint32_t kClientHeight = 720;
	// テクスチャ管理
	static const uint32_t kMaxTextureCount = 127; // ImGui用に1つ開ける

	// ディスクリプタヒープサイズ
	static const uint32_t kRTVHeapSize = 3;   // スワップチェーン2 + オフスクリーン1
	static const uint32_t kDSVHeapSize = 2;   // メイン + オフスクリーン
	static const uint32_t kSRVHeapSize = 128; // テクスチャ + ImGui

	// オフスクリーンレンダリング設定
	// オフスクリーンレンダリングのサイズはクライアント領域と同じ
	static const uint32_t kOffscreenWidth = kClientWidth;
	static const uint32_t kOffscreenHeight = kClientHeight;

	// インデックス定義
	// RTV[0]SwapChain0, [1]SwapChain1, [2]Offscreen
	static const uint32_t kSwapChainRTV0Index = 0;
	static const uint32_t kSwapChainRTV1Index = 1;
	static const uint32_t kOffscreenRTVIndex = 2;

	// DSV[0]Main, [1]Offscreen
	static const uint32_t kMainDSVIndex = 0;
	static const uint32_t kOffscreenDSVIndex = 1;
};