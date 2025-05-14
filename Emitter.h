#pragma once
#include "Particle.h"
#include <vector>
#include <queue>
#include "TriangularPyramid.h"

// ImGui
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class Emitter {
public:
	Emitter(ID3D12Device* device);
	~Emitter();


	void Initialize(int ImGuiNumber); // パーティカルプール
	void Update(float deltaTime);
	void Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle, const Matrix4x4& viewProjection);
	void ImGui();

	/// <summary>
	///  エミッターごとにパーティクルの情報を書き換える
	/// </summary>
	/// <param name="Scalemin">Scale最小値</param>
	/// <param name="Scalemax">Scale最大値</param>
	/// <param name="Rotatemin">Rotate最小値</param>
	/// <param name="Rotatemax">Rotate最大値</param>
	/// <param name="Translatemin">Transform最小値</param>
	/// <param name="Translatemax">Transform最大値</param>
	/// <param name="MinSpeed">最低速度/param>
	/// <param name="Maxspeed">最高速度</param>
	/// <param name="MoveDirection">移動する方向</param>
	/// <param name="Spawninterval">生成されるまでの時間</param>
	void SetParticleData(Vector3 Scalemin, Vector3 Scalemax, Vector3 Rotatemin, Vector3 Rotatemax,Vector3 Translatemin, Vector3 Translatemax,
		float MinSpeed,float Maxspeed,Vector3 MoveDirection,float Spawninterval,Vector4 Color);
	
	//デバッグ用imguiを使用するか(最後に消す)
	bool useImGui = true;
	//ImGuiで識別する番号
	int imGuiNumber;

private:
	const size_t MAX_PARTICLES = 300; // 適切な数に調整

	std::vector<Particle*> activeParticles; // アクティブなパーティクル
	std::queue<Particle*> particlePool;    // 待機中のパーティクル
	float spawnTimer;
	float spawnInterval;
	Vector3Transform SetParticles;	//パーティクルにセットするTransform情報
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
	Vector4 color;
	
};

