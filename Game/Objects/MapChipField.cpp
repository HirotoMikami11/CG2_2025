#include "MapChipField.h"
#include <map>

namespace { // 衝突回避のnamespace

	std::map<std::string, MapChipType> mapChipTable = {
		{"0", MapChipType::kBlank},
		{"1", MapChipType::kBlock},
	};
}
MapChipField::MapChipField() {}

MapChipField::~MapChipField() {}

void MapChipField::Initialize() {

	//マップチップの大きさ(追従時の限界値で使う)
	mapChipSize_ = {
		.x{kNumBlockHorizontal * kBlockWidth},
		.y{kNumBlockVirtical * kBlockHeight},
	};

}

void MapChipField::ResetMapChipData() {
	mapChipData_.data.clear();
	mapChipData_.data.resize(kNumBlockVirtical);
	for (std::vector<MapChipType>& mapChipDataLine : mapChipData_.data) {
		mapChipDataLine.resize(kNumBlockHorizontal);
	}
}

void MapChipField::LoadMapChipCSV(const std::string& filePath) {
	// マップチップデータをリセット
	ResetMapChipData();

	// ファイルを開く
	std::ifstream file;
	// マップチップCSV
	file.open(filePath);
	assert(file.is_open());

	// マップチップCSV
	std::stringstream mapChipCsv;
	// ファイルの内容を文字列ストリームにコピー
	mapChipCsv << file.rdbuf();
	// ファイルを閉じる
	file.close();

	// CSVからマップチップデータを読み込む
	for (uint32_t i = 0; i < kNumBlockVirtical; ++i) {
		std::string line;
		getline(mapChipCsv, line);

		// 1行分の文字列をストリームに変換して解析しやすくなる
		std::istringstream line_stream(line);

		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			std::string word;
			getline(line_stream, word, ',');
			if (mapChipTable.contains(word)) {
				mapChipData_.data[i][j] = mapChipTable[word];
			}
		}
	}
}

MapChipType MapChipField::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex) {
	/// 横列の範囲外を指定したら空白を返す
	if (xIndex < 0 || kNumBlockHorizontal - 1 < xIndex) {
		return MapChipType::kBlank;
	}
	/// 縦列の範囲外を指定したら空白を返す
	if (yIndex < 0 || kNumBlockVirtical - 1 < yIndex) {
		return MapChipType::kBlank;
	}
	return mapChipData_.data[yIndex][xIndex];
}

MapChipField::IndexSet MapChipField::GetMapChipIndexSetByPosition(const Vector3& position) {
	IndexSet indexSet = {};
	indexSet.xindex = static_cast<uint32_t>(std::floor((position.x + (kBlockWidth / 2)) / kBlockWidth));
	// y座標は反転しているので、virtical-1から引く
	indexSet.yindex = static_cast<uint32_t>((kNumBlockVirtical - 1) - std::floor((position.y + (kBlockHeight / 2)) / kBlockHeight));
	return indexSet;

}

MapChipField::Rect MapChipField::GetRectByIndex(uint32_t xIndex, uint32_t yIndex) { // 指定ブロックの中心座標を取得する
	Vector3 center = GetMapChipPositionByIndex(xIndex, yIndex);

	Rect rect;
	rect.left = center.x - (kBlockWidth / 2.0f);
	rect.right = center.x + (kBlockWidth / 2.0f);
	rect.bottom = center.y - (kBlockHeight / 2.0f);
	rect.top = center.y + (kBlockHeight / 2.0f);

	return rect;
}

Vector3 MapChipField::GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex) {

	/// Y座標はマップチップとワールド座標で齟齬があるので
	/// マップチップのyIndexが一番下（最大値のとき）ワールド座標Yが０になるように反転
	return Vector3(kBlockWidth * xIndex, kBlockHeight * (kNumBlockVirtical - 1 - yIndex), 0);
}

