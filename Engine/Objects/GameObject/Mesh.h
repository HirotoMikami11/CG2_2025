#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <string>

#include "../DirectXCommon.h"
#include "MyMath/MyFunction.h"
#include "BaseSystem/Logger/Logger.h"

#include <cassert>
#include <fstream>
#include <sstream>



/// <summary>
/// メッシュの種類
/// </summary>
enum class MeshType {
	TRIANGLE,       // 三角形
	SPHERE,         // 球体
	SPRITE,         // スプライト（矩形）
	MODEL_OBJ,      // OBJファイル読み込み
};

/// <summary>
/// メッシュのクラス
/// </summary>
class Mesh final
{
public:
	Mesh() = default;
	~Mesh() = default;

	/// <summary>
   /// プリミティブメッシュの初期化
   /// </summary>
   /// <param name="dxCommon">DirectXCommonのポインタ</param>
   /// <param name="meshType">メッシュの種類</param>
	void Initialize(DirectXCommon* dxCommon, MeshType meshType);


	/// <summary>
	/// カスタムデータからメッシュを初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="modelData">モデルデータ</param>
	void InitializeFromData(DirectXCommon* dxCommon, const ModelData& modelData);





	/// <summary>
	/// 三角形メッシュを作成
	/// </summary>
	void CreateTriangle();

	/// <summary>
	/// 球体メッシュを作成
	/// </summary>
	/// <param name="subdivision">分割数（デフォルト：16）</param>
	void CreateSphere(uint32_t subdivision = 16);

	/// <summary>
	/// スプライト用矩形メッシュを作成
	/// </summary>
	/// <param name="center">中心座標</param>
	/// <param name="size">サイズ</param>
	void CreateSprite(const Vector2& center = { 0.0f, 0.0f }, const Vector2& size = { 1.0f, 1.0f });


	/// <summary>
	/// モデルデータからメッシュを作成
	/// </summary>
	/// <param name="modelData">モデルデータ</param>
	void CreateModel(const ModelData& modelData);

	/// <summary>
	/// 頂点データを直接設定
	/// </summary>
	/// <param name="vertices">頂点データ</param>
	void SetVertices(const std::vector<VertexData>& vertices);

	/// <summary>
	/// インデックスデータを直接設定
	/// </summary>
	/// <param name="indices">インデックスデータ</param>
	void SetIndices(const std::vector<uint32_t>& indices);

	/// <summary>
	/// バッファをコマンドリストにバインド
	/// </summary>
	/// <param name="commandList">コマンドリスト</param>
	void Bind(ID3D12GraphicsCommandList* commandList);

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="commandList">コマンドリスト</param>
	/// <param name="instanceCount">インスタンス数（デフォルト：1）</param>
	void Draw(ID3D12GraphicsCommandList* commandList, uint32_t instanceCount = 1);

	//Getter
	MeshType GetMeshType() const { return meshType_; }
	uint32_t GetVertexCount() const { return static_cast<uint32_t>(vertices_.size()); }
	uint32_t GetIndexCount() const { return static_cast<uint32_t>(indices_.size()); }
	bool HasIndices() const { return !indices_.empty(); }
	const std::vector<VertexData>& GetVertices() const { return vertices_; }
	const std::vector<uint32_t>& GetIndices() const { return indices_; }

	// マテリアル情報取得（TextureManagerで使用）
	const std::string& GetTextureFilePath() const { return material_.textureFilePath; }	//ファイルパス
	bool HasMaterialInfo() const { return !material_.textureFilePath.empty(); }			//ファイルパスがあるかどうか
	const MaterialDataModel& GetMaterialInfo() const { return material_; }				//マテリアルデータがあるか

	/// <summary>
	/// メッシュタイプを文字列に変換（デバッグ用）
	/// </summary>
	/// <param name="type">メッシュタイプ</param>
	/// <returns>タイプ名</returns>
	static std::string MeshTypeToString(MeshType type);
private:
	/// <summary>
	/// バッファリソースを作成・更新
	/// </summary>
	void UpdateBuffers();

	/// <summary>
	/// 頂点バッファを作成
	/// </summary>
	void CreateVertexBuffer();

	/// <summary>
	/// インデックスバッファを作成
	/// </summary>
	void CreateIndexBuffer();

private:
	// DirectXCommon参照
	DirectXCommon* directXCommon_ = nullptr;

	//メッシュの種類
	MeshType meshType_ = MeshType::TRIANGLE; // デフォルトは三角形

	// 頂点・インデックスデータ
	std::vector<VertexData> vertices_;
	std::vector<uint32_t> indices_;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer_;

	// バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

	//読み取り専用のデータ マテリアルクラスに送るためにいったん保持しておく用
	MaterialDataModel material_;


	//三角形の面法線を計算する関数
	void CalculateTriangleNormals();

};

