#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include "MyFunction.h"
#include "Easing.h"
#include <cmath>

class BreakScreenEffect
{
public:
	BreakScreenEffect() = default;
	~BreakScreenEffect() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="device">デバイス</param>
	void Initialize(ID3D12Device* device);

	/// <summary>
	/// 更新（自動アニメーション）
	/// </summary>
	void Update();

	/// <summary>
	/// ポストのエフェクト描画
	/// </summary>
	/// <param name="commandList">コマンドリスト</param>
	/// <param name="offScreenSRVHandle">オフスクリーンテクスチャのSRVハンドル</param>
	void Draw(ID3D12GraphicsCommandList* commandList,
		D3D12_GPU_DESCRIPTOR_HANDLE offScreenSRVHandle);

	/// <summary>
	/// 割れエフェクトをリセット（新しいランダム中心点を生成）
	/// </summary>
	void Reset();

	/// <summary>
	/// エフェクトが完了したかチェック
	/// </summary>
	bool GetCompleted() const { return isCompleted; }

	/// <summary>
	/// エフェクトが途中か
	/// </summary>
	bool GetActive() const { return isActive; }

	/// <summary>
	/// エフェクトをアクティブに変更
	/// </summary>
	void SetActive(bool active) { isActive = active; }

	/// <summary>
	/// 全体のアニメーション時間を取得（デバッグ用）
	/// </summary>
	float GetTotalAnimationTime() const { return totalAnimationTime; }

	/// <summary>
	/// 画面外への移動フェーズかどうかをチェック
	/// </summary>
	bool IsMovingOut() const { return isMovingOut; }
private:
	/// <summary>
	/// 三角形の頂点データを作成
	/// </summary>
	void CreateTriangleVertices();

	/// <summary>
	/// バッファリソースを作成
	/// </summary>
	void CreateBuffers(ID3D12Device* device);

	/// <summary>
	/// ポストエフェクト専用のPSOを作成
	/// </summary>
	void CreatePostEffectPSO(ID3D12Device* device);

	/// <summary>
	/// ランダムな中心点を生成
	/// </summary>
	void GenerateRandomCenter();

	static const int kTriangleCount = 8; // X字に分割するための三角形数

	// 頂点バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

	// 各三角形用のマテリアルバッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> materialBuffer;
	Material* materialData = { nullptr };

	// 各三角形用のトランスフォームバッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> transformBuffers[8];
	TransformationMatrix* transformDatas[8] = { nullptr };

	// DirectionalLightバッファ(使わない)
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightBuffer;
	DirectionalLight* directionalLightData = nullptr;

	// ポストエフェクト専用のPSO
	Microsoft::WRL::ComPtr<ID3D12RootSignature> postEffectRootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> postEffectPSO;

	// 三角形の頂点データ
	std::vector<VertexData> triangleVertices;

	// 割れの進行度とアニメーション関連
	float breakScreenProgress = 0.0f;
	bool isActive = false;		// エフェクト途中かどうか
	bool isCompleted = false;	// エフェクトが完了したかどうか
	bool isMovingOut = false;	// 画面外に移動中かどうか
	bool isReturning = false;	// 元の位置に戻り中かどうか

	// ランダム中心点
	Vector2 randomCenter = { 0.0f, 0.0f };

	// 各三角形の初期位置と方向
	Vector3 triangleDirections[8];
	Vector3 originalPositions[8];

	// アニメーション時間
	float totalAnimationTime = 0.0f;	// 全体のアニメーション時間
	float phaseTime = 0.0f;				// 現在のフェーズでの経過時間

	// 各フェーズの時間設定
	const float shatterDuration = 2.0f;	// 割れるのにかかる時間
	const float moveOutDuration = 3.0f;		// 画面外への移動時間
	const float returnDuration = 3.0f;		// 戻りの移動時間
	const float maxMoveDistance = 3.0f;		// 最大移動距離（画面外まで）
};