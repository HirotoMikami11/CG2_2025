#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "MyMath/MyFunction.h"


/// <summary>
/// 単一線分描画クラス
/// 1本の線分の描画のみ
/// </summary>
class Line
{
public:
	Line() = default;
	~Line() = default;

	/// <summary>
	/// 静的初期化（一度だけ呼び出す）
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	static void InitializeStatic(DirectXCommon* dxCommon);

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 線分の設定
	/// </summary>
	/// <param name="start">開始点</param>
	/// <param name="end">終了点</param>
	void SetPoints(const Vector3& start, const Vector3& end);

	/// <summary>
	/// 線の色を設定
	/// </summary>
	/// <param name="color">色</param>
	void SetColor(const Vector4& color);

	/// <summary>
	/// 線分を描画（静的関数：複数線分の一括描画用）
	/// </summary>
	/// <param name="commandList">コマンドリスト</param>
	/// <param name="vertices">頂点データ</param>
	/// <param name="vertexCount">頂点数</param>
	/// <param name="materialBuffer">マテリアルバッファ</param>
	/// <param name="transformBuffer">トランスフォームバッファ</param>
	static void DrawLines(
		ID3D12GraphicsCommandList* commandList,
		const VertexData* vertices,
		uint32_t vertexCount,
		ID3D12Resource* materialBuffer,
		ID3D12Resource* transformBuffer
	);

	// Getter
	const Vector3& GetStart() const { return start_; }
	const Vector3& GetEnd() const { return end_; }
	const Vector4& GetColor() const { return color_; }

	/// <summary>
	/// 頂点データを取得（GridRendererで使用）
	/// </summary>
	/// <param name="startVertex">開始点頂点データ（出力）</param>
	/// <param name="endVertex">終了点頂点データ（出力）</param>
	void GetVertexData(VertexData& startVertex, VertexData& endVertex) const;

private:
	// 線分データ
	Vector3 start_{ 0.0f, 0.0f, 0.0f };
	Vector3 end_{ 1.0f, 0.0f, 0.0f };
	Vector4 color_{ 1.0f, 1.0f, 1.0f, 1.0f };

	// DirectXCommon参照（静的描画用）
	static DirectXCommon* directXCommon_;
};