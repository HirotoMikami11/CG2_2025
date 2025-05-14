#pragma once

#include"TriangularPrism.h"
#include <algorithm>  // std::clamp
#include"Easing.h"

#include"TriforceEmitter.h"

class TriForce {
public:
	// コンストラクタ、デストラクタ
	TriForce(ID3D12Device* device);
	~TriForce();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// イージングの進行状態を完全にリセット
	/// </summary>
	void ResetProgress();

	/// <summary>
	/// イージング開始フラグを設定
	/// </summary>
	void StartEasing() { shouldStartEasing = true; }

	/// <summary>
	/// イージング停止
	/// </summary>
	void StopEasing() { shouldStartEasing = false; }

	/// <summary>
	/// イージング中かどうかを取得
	/// </summary>
	bool IsEasing() const { return shouldStartEasing && t < 1.0f; }

	/// <summary>
	/// イージングする動作の関数
	/// </summary>
	/// <param name="easing_num">デバッグ用</param>
	/// <param name="viewProjection">ビュープロジェクション行列</param>
	void MoveEasing(int easing_num, const Matrix4x4& viewProjection);

	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="viewProjectionMatrix"></param>
	void Update(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="commandList"></param>
	/// <param name="textureHandle"></param>
	void Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle);

	Vector4& GetColor() {
		return triangularPrism[0]->GetColor();
	}

	const Vector3Transform& GetTransform() const { return triangularPrism[0]->GetTransform(); }

	bool IsCompleted() const { return t >= 1.0f; }
	float GetProgress() const { return t; }

private:
	///トライフォースになる三角柱
	TriangularPrism* triangularPrism[3];
	///インデックス
	const int indexTriangularPrism = 3;

	///残像を生成するエミッター
	TriforceEmitter* triforceEmitter[3];

	///イージング用の変数
	Vector3 moveStart[3];
	Vector3 moveEnd[3];
	Vector3 rotateStart[3];
	Vector3 rotateEnd[3];
	float t;

	///移動用のイージングが完了してからのタイマー
	float endEaseTimer;

	///イージング開始フラグ
	bool shouldStartEasing;
};