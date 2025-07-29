#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <memory>
#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "MyMath/MyFunction.h"

/// <summary>
/// 線分用の頂点データ構造体
/// </summary>
struct LineVertex {
	Vector4 position;  // xyz座標 + w
	Vector4 color;     // RGBA色
	Vector2 texcoord;  // テクスチャ座標（未使用だが互換性のため）
	Vector3 normal;    // 法線（未使用だが互換性のため）
};

/// <summary>
/// 線分描画データ
/// </summary>
struct LineData {
	Vector3 start;
	Vector3 end;
	Vector4 color;
};

/// <summary>
/// 複数線分の一括描画システム
/// KamataEngineのPrimitiveDrawerを参考にした実装
/// </summary>
class LineRenderer {
public:
	// 線分の最大数（KamataEngineと同じ）
	static const uint32_t kMaxLineCount = 4096;
	// 線分の頂点数
	static const uint32_t kVertexCountPerLine = 2;

	LineRenderer() = default;
	~LineRenderer() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 線分を追加
	/// </summary>
	/// <param name="start">開始点</param>
	/// <param name="end">終了点</param>
	/// <param name="color">色</param>
	void AddLine(const Vector3& start, const Vector3& end, const Vector4& color);

	/// <summary>
	/// 線分リストをクリア
	/// </summary>
	void Reset();

	/// <summary>
	/// 一括描画
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	void Draw(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// ImGui表示
	/// </summary>
	void ImGui();

	// Getter
	uint32_t GetLineCount() const { return static_cast<uint32_t>(lineData_.size()); }
	bool IsEmpty() const { return lineData_.empty(); }
	bool IsFull() const { return lineData_.size() >= kMaxLineCount; }

	// Setter
	void SetVisible(bool visible) { isVisible_ = visible; }
	bool IsVisible() const { return isVisible_; }

private:
	/// <summary>
	/// 頂点バッファを更新
	/// </summary>
	void UpdateVertexBuffer();

	/// <summary>
	/// マテリアルバッファを更新
	/// </summary>
	void UpdateMaterialBuffer();

private:
	// DirectXCommon参照
	DirectXCommon* directXCommon_ = nullptr;

	// 線分データ
	std::vector<LineData> lineData_;

	// 表示フラグ
	bool isVisible_ = true;

	// DirectX12リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
	Microsoft::WRL::ComPtr<ID3D12Resource> transformBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	// マップされたデータ
	LineVertex* vertexData_ = nullptr;
	TransformationMatrix* transformData_ = nullptr;

	// 更新フラグ
	bool needsVertexUpdate_ = false;
	bool isInitialized_ = false;
};