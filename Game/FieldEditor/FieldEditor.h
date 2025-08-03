#pragma once
#include <memory>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "CameraController/CameraController.h"
#include "Objects/Light/Light.h"
#include "Objects/GameObject/GameObject.h"
#include "GameObjects/Rock/Rock.h"

/// <summary>
/// フィールドオブジェクトデータ
/// </summary>
struct FieldObjectData {
	Vector3 position;
	Vector3 rotation;
	Vector3 scale;
	RockType rockType;
	int id; // オブジェクトの一意ID
	bool isSelected; // 選択状態
	std::unique_ptr<Rock> actualRock; // 実際のRockオブジェクト

	// デフォルトコンストラクタ
	FieldObjectData()
		: position{ 0.0f, 0.0f, 0.0f }
		, rotation{ 0.0f, 0.0f, 0.0f }
		, scale{ 1.0f, 1.0f, 1.0f }
		, rockType(RockType::Rock1)
		, id(0)
		, isSelected(false)
		, actualRock(nullptr) {
	}

	// コピーコンストラクタを削除
	FieldObjectData(const FieldObjectData&) = delete;
	FieldObjectData& operator=(const FieldObjectData&) = delete;

	// ムーブコンストラクタ
	FieldObjectData(FieldObjectData&& other) noexcept
		: position(other.position)
		, rotation(other.rotation)
		, scale(other.scale)
		, rockType(other.rockType)
		, id(other.id)
		, isSelected(other.isSelected)
		, actualRock(std::move(other.actualRock)) {
	}

	// ムーブ代入演算子
	FieldObjectData& operator=(FieldObjectData&& other) noexcept {
		if (this != &other) {
			position = other.position;
			rotation = other.rotation;
			scale = other.scale;
			rockType = other.rockType;
			id = other.id;
			isSelected = other.isSelected;
			actualRock = std::move(other.actualRock);
		}
		return *this;
	}
};

/// <summary>
/// フィールドエディタクラス（デバッグ専用）
/// </summary>
class FieldEditor {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	FieldEditor();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~FieldEditor();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="cameraController">カメラコントローラー</param>
	void Initialize(DirectXCommon* dxCommon, CameraController* cameraController);

	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	void Update(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// 描画（プレビューモデル）
	/// </summary>
	/// <param name="directionalLight">平行光源</param>
	void Draw(const Light& directionalLight);

	/// <summary>
	/// ImGui
	/// </summary>
	void ImGui();

	/// <summary>
	/// CSVファイルに保存
	/// </summary>
	/// <param name="fileName">ファイル名</param>
	/// <returns>保存に成功したかどうか</returns>
	bool SaveToCSV(const std::string& fileName);

	/// <summary>
	/// CSVファイルから読み込み
	/// </summary>
	/// <param name="fileName">ファイル名</param>
	/// <returns>読み込みに成功したかどうか</returns>
	bool LoadFromCSV(const std::string& fileName);

	/// <summary>
	/// 全てのオブジェクトをクリア
	/// </summary>
	void ClearAllObjects();

	// Getter
	bool IsEditorEnabled() const { return isEditorEnabled_; }
	bool IsPreviewVisible() const { return showPreviewRocks_; }

private:
	/// <summary>
	/// オブジェクトを追加
	/// </summary>
	/// <param name="objectData">オブジェクトデータ</param>
	void AddObject(const FieldObjectData& objectData);

	/// <summary>
	/// オブジェクトを削除
	/// </summary>
	/// <param name="id">削除するオブジェクトのID</param>
	void RemoveObject(int id);

	/// <summary>
	/// 次のIDを取得
	/// </summary>
	/// <returns>次のID</returns>
	int GetNextId();

	/// <summary>
	/// 新規追加用Rockを作成・更新
	/// </summary>
	void CreateOrUpdatePreviewRock();

	/// <summary>
	/// Rockの色を更新（選択状態に応じて）
	/// </summary>
	/// <param name="rock">Rockオブジェクト</param>
	/// <param name="rockType">岩タイプ</param>
	/// <param name="isSelected">選択状態</param>
	/// <param name="isPreview">プレビュー用かどうか</param>
	void UpdateRockColor(Rock* rock, RockType rockType, bool isSelected, bool isPreview = false);

	/// <summary>
	/// 岩タイプごとの基本色を取得
	/// </summary>
	/// <param name="rockType">岩タイプ</param>
	/// <returns>色</returns>
	Vector4 GetRockTypeBaseColor(RockType rockType);

	/// <summary>
	/// 新規追加用データをリセット
	/// </summary>
	void ResetNewRockData();

	/// <summary>
	/// 全Rockの選択状態を更新
	/// </summary>
	/// <param name="selectedIndex">選択されたインデックス（-1で選択解除）</param>
	void UpdateSelectionStates(int selectedIndex);

	/// <summary>
	/// 選択インデックスが有効かチェック
	/// </summary>
	/// <returns>有効かどうか</returns>
	bool IsValidSelectedIndex() const;

	/// <summary>
	/// インラインエディタを描画
	/// </summary>
	/// <param name="objData">オブジェクトデータ</param>
	/// <param name="index">インデックス</param>
	void DrawInlineEditor(FieldObjectData& objData, size_t index);

	/// <summary>
	/// アクションボタン群を描画
	/// </summary>
	/// <param name="objData">オブジェクトデータ</param>
	/// <param name="index">インデックス</param>
	void DrawActionButtons(FieldObjectData& objData, size_t index);

	/// <summary>
	/// 詳細エディタを描画
	/// </summary>
	void DrawDetailedEditor();

	// システム参照
	DirectXCommon* directXCommon_ = nullptr;
	CameraController* cameraController_ = nullptr;

	// エディタ有効フラグ
	bool isEditorEnabled_ = false;

	// 表示設定
	bool showPreviewRocks_ = true; // プレビューRock表示

	// フィールドオブジェクト
	std::vector<FieldObjectData> objectDataList_;

	// エディタ用パラメータ（新規追加用）
	Vector3 editorPosition_ = { 0.0f, 0.0f, 0.0f };
	Vector3 editorRotation_ = { 0.0f, 0.0f, 0.0f };
	Vector3 editorScale_ = { 1.0f, 1.0f, 1.0f };
	RockType currentRockType_ = RockType::Rock1;

	// 新規追加用プレビューRock
	std::unique_ptr<Rock> previewRock_;

	// 選択中のオブジェクト
	int selectedObjectId_ = -1;

	// ID管理
	int nextId_ = 0;

	// ファイル名
	char csvFileName_[256] = "resources/CSV_Data/Field/field.csv";

	// エディタ定数
	static constexpr float kColorBrightness = 0.4f;
	static constexpr float kSelectedColorBoost = 0.4f;
	static constexpr float kNormalAlpha = 0.8f;
	static constexpr float kSelectedAlpha = 1.0f;
	static constexpr float kPreviewAlpha = 0.6f;
};