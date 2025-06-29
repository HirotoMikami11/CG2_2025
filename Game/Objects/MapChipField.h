#pragma once

#include "Engine.h"

#include <cassert>
#include <fstream>
#include <sstream>
#include "MyMath/MyFunction.h"

enum class MapChipType {
	kBlank, // 空白
	kBlock, // ブロック
};

struct MapChipData {
	std::vector<std::vector<MapChipType>> data;
};

/// <summary>
/// マップチップクラス
/// </summary>
class MapChipField {

public:
	struct IndexSet {
		uint32_t xindex;
		uint32_t yindex;
	};

	//範囲矩形
	struct Rect {
		float left;		//左端
		float right;	//右端
		float bottom;	//下端
		float top;		//上端
	};


	/// <summary>
	/// コンストクラタ
	/// </summary>
	MapChipField();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~MapChipField();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// マップチップのデータをリセットする関数
	/// </summary>
	void ResetMapChipData();

	/// <summary>
	/// CSVからマップチップを読み込む関数
	/// </summary>
	/// <param name="filePath">ファイルパス</param>
	void LoadMapChipCSV(const std::string& filePath);




	// 縦列の個数
	uint32_t GetNumBlockkVirtical() { return kNumBlockVirtical; }
	// 横列
	uint32_t GetNumBlockkHorizontal() { return kNumBlockHorizontal; }
	//マップの大きさ
	Vector2 GetMapSize() { return mapChipSize_; }


	/// <summary>
	/// 横縦のインデックスを指定してその位置のマップチップ種別を取得する関数
	/// </summary>
	/// <param name="xIndex"></param>
	/// <param name="yIndex"></param>
	/// <returns></returns>
	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);

	/// <summary>
	/// 指定座標がマップチップの何番目にあるかを計算する関数
	/// </summary>
	/// <param name="position"></param>
	/// <returns></returns>
	IndexSet GetMapChipIndexSetByPosition(const Vector3& position);


	Rect GetRectByIndex(uint32_t xIndex, uint32_t yIndex);

	/// <summary>
	/// 横縦のインデックスを指定して、その位置のマップチップのワールド座標を取得する
	/// </summary>
	/// <param name="xIndex"></param>
	/// <param name="yIndex"></param>
	/// <returns></returns>
	Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex);



private:

	// 1ブロックのサイズ
	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;

	// ブロックの個数
	static inline const uint32_t kNumBlockVirtical = 20;
	static inline const uint32_t kNumBlockHorizontal = 50;

	//マップチップ全体の大きさ
	Vector2 mapChipSize_;

	MapChipData mapChipData_;


	// システム参照
	DirectXCommon* directXCommon_;

};