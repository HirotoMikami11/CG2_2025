#pragma once
#include "SkyDustParticle.h"
#include <vector>
#include <queue>
#include "TriangularPyramid.h"

// ImGui
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class SkyDustEmitter {
public:
	SkyDustEmitter(ID3D12Device* device);
	~SkyDustEmitter();

	void Initialize(); // 初期化関数を追加
	void Update(float deltaTime);
	void Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle, const Matrix4x4& viewProjection);
	void ImGui();

	//デバッグ用imguiを使用するか(最後に消す)
	bool useImGui = true;

private:
	const size_t MAX_PARTICLES = 45; // 適切な数に調整

	std::vector<SkyDustParticle*> activeParticles; // アクティブなパーティクル
	std::queue<SkyDustParticle*> particlePool;    // 待機中のパーティクル

	float spawnTimer;
	float spawnInterval;
	Vector3Transform SetParticles;    // パーティクルにセットするTransform情報
	ID3D12Device* device;

	//	ランダムで生成されるパーティクルの最大値最小値
	Vector3 translateMin;
	Vector3 translateMax;
	Vector3 rotateMin;
	Vector3 rotateMax;
	Vector3 scaleMin;
	Vector3 scaleMax;

	// パーティクルの移動用変数
	float minSpeed;				// 最小移動速度
	float maxSpeed;				// 最大移動速度
	Vector3 moveDirection;		// 移動方向
	float directionVariance;	// 方向のばらつき
	


};
