#pragma once
#include <memory>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "Objects/Light/Light.h"
#include "GameObjects/Rock/Rock.h"

/// <summary>
/// フィールドローダークラス
/// </summary>
class FieldLoader {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	FieldLoader();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~FieldLoader();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// CSVファイルからフィールドを読み込み
	/// </summary>
	/// <param name="fileName">CSVファイル名</param>
	/// <returns>読み込みに成功したかどうか</returns>
	bool LoadField(const std::string& fileName);

	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	void Update(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="directionalLight">平行光源</param>
	void Draw(const Light& directionalLight);

	/// <summary>
	/// フィールドオブジェクトを全てクリア
	/// </summary>
	void ClearField();

	/// <summary>
	/// フィールドが読み込まれているかどうか
	/// </summary>
	/// <returns>読み込まれている場合true</returns>
	bool IsFieldLoaded() const { return isFieldLoaded_; }

	/// <summary>
	/// 読み込まれたオブジェクト数を取得
	/// </summary>
	/// <returns>オブジェクト数</returns>
	size_t GetObjectCount() const { return rocks_.size(); }

private:
	// システム参照
	DirectXCommon* directXCommon_ = nullptr;

	// フィールドオブジェクト
	std::vector<std::unique_ptr<Rock>> rocks_;

	// 読み込み状態
	bool isFieldLoaded_ = false;
};