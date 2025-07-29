#pragma once
#include <memory>
#include <d3d12.h>
#include <wrl.h>

#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "Objects/GameObject/Transform3D.h"
#include "Objects/Line/Line.h"
#include "MyMath/MyFunction.h"

/// <summary>
/// グリッド線専用のマテリアル構造体
/// </summary>
struct GridLineMaterial {
	Vector4 color;  // 基本色のみ（16バイト）
};

/// <summary>
/// グリッド平面の種類
/// </summary>
enum class GridLineType {
	XZ,  // XZ平面上のグリッド
	XY,  // XY平面上のグリッド
	YZ   // YZ平面上のグリッド
};

/// <summary>
/// グリッド描画専用クラス
/// 責務：グリッド線の生成・管理・描画
/// 個別描画と一括描画の両方に対応
/// </summary>
class GridLine
{
public:
	GridLine() = default;
	~GridLine() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	void Initialize(DirectXCommon* dxCommon,
		const GridLineType& GridLineType,
		float size = 100.0f,
		float interval = 1.0f,
		float majorInterval = 10.0f,
		const Vector4& normalColor = { 0.5f, 0.5f, 0.5f, 1.0f },
		const Vector4& majorColor = { 0.0f, 0.0f, 0.0f, 1.0f }
	);

	/// <summary>
	/// グリッドを生成
	/// </summary>
	/// <param name="size">グリッドサイズ（一辺の長さ）</param>
	/// <param name="interval">間隔</param>
	/// <param name="majorInterval">主要線の間隔</param>
	/// <param name="normalColor">通常線の色</param>
	/// <param name="majorColor">主要線の色</param>
	void CreateGrid(
		const GridLineType& GridLineType = GridLineType::XZ,
		float size = 100.0f,
		float interval = 1.0f,
		float majorInterval = 10.0f,
		const Vector4& normalColor = { 0.5f, 0.5f, 0.5f, 1.0f },
		const Vector4& majorColor = { 0.0f, 0.0f, 0.0f, 1.0f }
	);

	/// <summary>
	/// カスタム線分を追加
	/// </summary>
	/// <param name="start">開始点</param>
	/// <param name="end">終了点</param>
	/// <param name="color">色</param>
	void AddLine(const Vector3& start, const Vector3& end, const Vector4& color);

	/// <summary>
	/// 全ての線分をクリア
	/// </summary>
	void Clear();

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	void Update(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// 描画処理
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	void Draw(const Matrix4x4& viewProjectionMatrix);


	/// <summary>
	/// ImGui用デバッグ表示
	/// </summary>
	void ImGui();

	// Getter
	bool IsVisible() const { return isVisible_; }
	bool IsActive() const { return isActive_; }
	const std::string& GetName() const { return name_; }
	size_t GetLineCount() const { return lines_.size(); }


	// Setter
	void SetVisible(bool visible) { isVisible_ = visible; }
	void SetActive(bool active) { isActive_ = active; }
	void SetName(const std::string& name) { name_ = name; }

	// Transform関連
	Transform3D& GetTransform() { return transform_; }
	const Transform3D& GetTransform() const { return transform_; }
	void SetTransform(const Vector3Transform& newTransform) { transform_.SetTransform(newTransform); }

private:

	/// <summary>
	/// 各軸の線を生成
	/// </summary>

	void CreateXZGrid(float halfSize);
	void CreateXYGrid(float halfSize);
	void CreateYZGrid(float halfSize);

	// 基本情報
	DirectXCommon* directXCommon_ = nullptr;
	bool isVisible_ = true;
	bool isActive_ = true;
	std::string name_ = "GridLine";

	// Transform（グリッド全体の変換）
	Transform3D transform_;

	// 線分データ
	std::vector<std::unique_ptr<Line>> lines_;

	// グリッド設定（ImGui用）
	float gridSize_ = 50.0f;
	float gridInterval_ = 1.0f;
	float gridMajorInterval_ = 10.0f;
	Vector4 gridNormalColor_ = { 0.5f, 0.5f, 0.5f, 1.0f };
	Vector4 gridMajorColor_ = { 0.0f, 0.0f, 0.0f, 1.0f };
};