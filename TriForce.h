#pragma once

#include"TriangularPrism.h"
#include <algorithm>  // std::clamp
#include"Easing.h"

#include"TriforceEmitter.h"
// ImGui
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class TriForce {
public:
	// コンストラクタ、デストラクタ
	TriForce(ID3D12Device* device);
	~TriForce();

	void Initialize();
	void ResetProgress();
	void StartEasing() { shouldStartEasing = true; }
	void StopEasing() { shouldStartEasing = false; }
	bool IsEasing() const { return shouldStartEasing && !IsCompleted(); }
	void MoveEasing(const Matrix4x4& viewProjection);
	void Update(const Matrix4x4& viewProjectionMatrix);
	void Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle);

	const Vector3Transform& GetTransform() const { return triangularPrism[0]->GetTransform(); }
	bool IsCompleted() const { return thirdT >= 1.0f; }
	float GetProgress() const { return thirdT; }

private:
	///トライフォースになる三角柱
	TriangularPrism* triangularPrism[3];
	///インデックス
	const int indexTriangularPrism = 3;

	///残像を生成するエミッター
	TriforceEmitter* triforceEmitter[3];

	/// 第一段階：画面上部から落下
	Vector3 firstPosition[3];
	Vector3 firstRotate[3];
	float firstT;

	/// 第二段階：中間位置への移動
	Vector3 secondPosition[3];
	Vector3 secondRotate[3];
	float secondT;

	/// 第三段階：最終的な組み立て
	Vector3 thirdPosition[3];
	Vector3 thirdRotate[3];
	float thirdT;

	/// 最終完成位置・回転
	Vector3 finalPosition[3];
	Vector3 finalRotate[3];

	///移動用のイージングが開始するまでの時間
	const float kStartTime = 6.0f;
	float easeStartTimer = kStartTime;

	///イージング開始フラグ
	bool shouldStartEasing;

	/// 各段階の状態管理用列挙型
	enum class EasingStage {
		NOT_STARTED,	//演出開始前
		FIRST_STAGE,	//最初の段階
		SECOND_STAGE,	//中間の段階
		THIRD_STAGE,	//最後の段階
		COMPLETED		//完成！
	};

	EasingStage currentStage;	//イージングの現在の状況を持つ
};