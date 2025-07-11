#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "MyMath/MyFunction.h"

/// <summary>
/// 線分用のマテリアル構造体
/// </summary>
struct LineMaterial {
	Vector4 color;  // 基本色（16バイト）
};

/// <summary>
/// ラインクラス
/// </summary>
class Line
{
public:
	Line() = default;
	~Line() = default;

	/// <summary>
	/// 個別線分の初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	void Initialize(DirectXCommon* dxCommon);

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
	/// 更新処理
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	void Update(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	void Draw(const Matrix4x4& viewProjectionMatrix);

	// Getter
	const Vector3& GetStart() const { return start_; }
	const Vector3& GetEnd() const { return end_; }
	const Vector4& GetColor() const { return color_; }
	bool IsVisible() const { return isVisible_; }

	// Setter
	void SetVisible(bool visible) { isVisible_ = visible; }

	/// <summary>
	/// 頂点データを取得（一括描画用）
	/// </summary>
	/// <param name="startVertex">開始点頂点データ（出力）</param>
	/// <param name="endVertex">終了点頂点データ（出力）</param>
	void GetVertexData(VertexData& startVertex, VertexData& endVertex) const;

private:
	/// <summary>
	/// 頂点バッファを作成/更新
	/// </summary>
	void UpdateVertexBuffer();

	/// <summary>
	/// マテリアルバッファを作成/更新
	/// </summary>
	void UpdateMaterialBuffer();

private:
	// DirectXCommon参照（各インスタンス毎）
	DirectXCommon* directXCommon_ = nullptr;

	// 線分データ
	Vector3 start_{ 0.0f, 0.0f, 0.0f };
	Vector3 end_{ 1.0f, 0.0f, 0.0f };
	Vector4 color_{ 1.0f, 1.0f, 1.0f, 1.0f };
	bool isVisible_ = true;

	// DirectX12リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialBuffer_;
	Microsoft::WRL::ComPtr<ID3D12Resource> transformBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	// マップされたデータ
	LineMaterial* materialData_ = nullptr;
	TransformationMatrix* transformData_ = nullptr;

	// 更新フラグ
	bool needsVertexUpdate_ = true;
	bool needsMaterialUpdate_ = true;
	bool isInitialized_ = false;
};