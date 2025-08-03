#pragma once
#include <memory>
#include <vector>
#include <string>
#include <chrono>

#include "Objects/GameObject/GameObject.h"
#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "Objects/Light/Light.h"
#include "GameObjects/Enemy/BaseEnemy.h"
#include "MyMath/MyMath.h"

// 前方宣言
class EnemyPopCommand;
class CameraController;

/// <summary>
/// 敵プリセット定義
/// </summary>
struct EnemyPreset {
	std::string name;
	EnemyType enemyType;
	EnemyPattern pattern;
	int waitTime;
	Vector3 defaultScale;

	EnemyPreset(const std::string& n, EnemyType type, EnemyPattern pat, int wait, Vector3 scale)
		: name(n), enemyType(type), pattern(pat), waitTime(wait), defaultScale(scale) {
	}
};

/// <summary>
/// エディタで管理する敵配置データ
/// </summary>
struct EnemyPlacementData {
	Vector3 position;
	EnemyType enemyType;
	EnemyPattern pattern;
	int waitTime;           // この敵の前の待機時間
	bool isSelected;        // エディタで選択されているか
	std::unique_ptr<GameObject> previewModel; // プレビュー用モデル
	std::string modelName;  // モデル名
	Vector3 modelScale;     // モデルスケール

	// デフォルトコンストラクタ
	EnemyPlacementData()
		: position{ 0.0f, 0.0f, 100.0f }
		, enemyType(EnemyType::Normal)
		, pattern(EnemyPattern::Straight)
		, waitTime(0)
		, isSelected(false)
		, previewModel(nullptr)
		, modelName("sphere")
		, modelScale{ 3.0f, 3.0f, 3.0f } {
	}

	// コピーコンストラクタを削除
	EnemyPlacementData(const EnemyPlacementData&) = delete;

	// コピー代入演算子を削除
	EnemyPlacementData& operator=(const EnemyPlacementData&) = delete;

	// ムーブコンストラクタ
	EnemyPlacementData(EnemyPlacementData&& other) noexcept
		: position(other.position)
		, enemyType(other.enemyType)
		, pattern(other.pattern)
		, waitTime(other.waitTime)
		, isSelected(other.isSelected)
		, previewModel(std::move(other.previewModel))
		, modelName(std::move(other.modelName))
		, modelScale(other.modelScale) {
	}

	// ムーブ代入演算子
	EnemyPlacementData& operator=(EnemyPlacementData&& other) noexcept {
		if (this != &other) {
			position = other.position;
			enemyType = other.enemyType;
			pattern = other.pattern;
			waitTime = other.waitTime;
			isSelected = other.isSelected;
			previewModel = std::move(other.previewModel);
			modelName = std::move(other.modelName);
			modelScale = other.modelScale;
		}
		return *this;
	}
};

/// <summary>
/// 敵配置エディタクラス
/// </summary>
class EnemyPlacementEditor {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	EnemyPlacementEditor();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~EnemyPlacementEditor();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="cameraController">カメラコントローラーのポインタ</param>
	void Initialize(DirectXCommon* dxCommon, CameraController* cameraController = nullptr);

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
	/// ImGui描画
	/// </summary>
	void ImGui();

	/// <summary>
	/// CSV読み込み時に敵配置データを設定
	/// </summary>
	/// <param name="enemyPopCommand">敵生成コマンドのポインタ</param>
	void LoadFromEnemyPopCommand(EnemyPopCommand* enemyPopCommand);

	/// <summary>
	/// CSVに保存
	/// </summary>
	/// <param name="filePath">保存先ファイルパス</param>
	bool SaveToCSV(const std::string& filePath);

	/// <summary>
	/// CSVから読み込み
	/// </summary>
	/// <param name="filePath">読み込みファイルパス</param>
	bool LoadFromCSV(const std::string& filePath);

	/// <summary>
	/// 一時保存
	/// </summary>
	bool SaveTemporary();

	/// <summary>
	/// 一時保存データから読み込み
	/// </summary>
	bool LoadTemporary();

	/// <summary>
	/// 一時保存ファイルリストを取得
	/// </summary>
	std::vector<std::string> GetTemporaryFileList() const;

	/// <summary>
	/// 選択された敵をコピー
	/// </summary>
	void CopySelectedEnemy();

	/// <summary>
	/// クリップボードから敵をペースト
	/// </summary>
	void PasteEnemyFromClipboard();

	/// <summary>
	/// クリップボードにデータがあるか
	/// </summary>
	bool HasClipboardData() const { return hasClipboardData_; }

	// Getter
	bool IsEditorEnabled() const { return isEditorEnabled_; }
	bool IsPreviewVisible() const { return showPreviewModels_; }
	bool IsCSVEnemiesVisible() const { return showCSVEnemies_; }

private:
	// DirectXCommon参照
	DirectXCommon* directXCommon_;

	// CameraController参照
	CameraController* cameraController_;

	// エディタ有効フラグ
	bool isEditorEnabled_;

	// 表示設定
	bool showPreviewModels_;    // プレビューモデル表示
	bool showCSVEnemies_;       // CSV敵表示
	bool showActualModels_;     // 実際のモデル表示

	// 敵配置データ
	std::vector<EnemyPlacementData> enemyPlacements_;

	// 選択中のインデックス
	int selectedIndex_;

	// 新規追加用データ
	EnemyPlacementData newEnemyData_;

	// 新規追加時の設定保持
	bool preserveNewEnemySettings_;

	// CSVファイルパス
	std::string csvFilePath_;

	// 一時保存フォルダパス
	std::string temporaryFolderPath_;

	// プリセット管理
	std::vector<EnemyPreset> enemyPresets_;
	int selectedPresetIndex_;

	// クリップボード機能
	EnemyPlacementData clipboardData_;
	bool hasClipboardData_;

	/// <summary>
	/// プレビューモデルを作成
	/// </summary>
	/// <param name="placement">配置データ</param>
	/// <param name="isNewEnemy">新規追加用かどうか</param>
	void CreatePreviewModel(EnemyPlacementData& placement, bool isNewEnemy = false);

	/// <summary>
	/// プレビューモデルの色を更新
	/// </summary>
	/// <param name="placement">配置データ</param>
	void UpdatePreviewModelColor(EnemyPlacementData& placement);

	/// <summary>
	/// プレビューモデルのタイプを更新
	/// </summary>
	/// <param name="placement">配置データ</param>
	void UpdatePreviewModelType(EnemyPlacementData& placement);

	/// <summary>
	/// 新規敵データをリセット
	/// </summary>
	void ResetNewEnemyData();

	/// <summary>
	/// 全敵の選択状態を更新
	/// </summary>
	/// <param name="selectedIndex">選択されたインデックス（-1で選択解除）</param>
	void UpdateSelectionStates(int selectedIndex);

	/// <summary>
	/// 選択インデックスが有効かチェック
	/// </summary>
	/// <returns>有効かどうか</returns>
	bool IsValidSelectedIndex() const;

	/// <summary>
	/// プリセットを初期化
	/// </summary>
	void InitializePresets();

	/// <summary>
	/// プリセットを適用
	/// </summary>
	/// <param name="preset">プリセット</param>
	void ApplyPreset(const EnemyPreset& preset);

	/// <summary>
	/// 敵タイプからモデル名を取得
	/// </summary>
	/// <param name="enemyType">敵タイプ</param>
	/// <returns>モデル名</returns>
	std::string GetEnemyModelName(EnemyType enemyType) const;

	/// <summary>
	/// 敵タイプからデフォルトスケールを取得
	/// </summary>
	/// <param name="enemyType">敵タイプ</param>
	/// <returns>デフォルトスケール</returns>
	Vector3 GetEnemyDefaultScale(EnemyType enemyType) const;

	/// <summary>
	/// タイムスタンプ付きファイル名を生成
	/// </summary>
	/// <returns>ファイル名</returns>
	std::string GenerateTimestampedFilename() const;

	/// <summary>
	/// 選択時の色を取得
	/// </summary>
	/// <returns>選択時の色</returns>
	Vector4 GetSelectedColor() const { return { 0.1f, 0.1f, 0.1f, 0.9f }; }

	// ImGui描画メソッド
	void DrawFileOperations();
	void DrawPresetSelection();
	void DrawNewEnemySection();
	void DrawEnemyList();
	void DrawInlineEditor(EnemyPlacementData& placement, size_t index);
	void DrawActionButtons(EnemyPlacementData& placement, size_t index);
	void DrawDetailedEditor();
};