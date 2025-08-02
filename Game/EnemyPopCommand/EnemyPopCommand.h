#pragma once
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include "MyMath/MyMath.h"
#include "GameObjects/Enemy/BaseEnemy.h"

/// <summary>
/// 敵生成コマンドの種類
/// </summary>
enum class EnemyPopCommandType {
	POP,    // 敵生成
	WAIT    // 待機
};

/// <summary>
/// 敵生成データ
/// </summary>
struct EnemyPopData {
	Vector3 position;           // 生成位置
	EnemyType enemyType;        // 敵の種類
	EnemyPattern pattern;       // 敵のパターン
};

/// <summary>
/// 待機データ
/// </summary>
struct WaitData {
	int32_t waitTime;           // 待機時間（フレーム数）
};

/// <summary>
/// エディタ用敵配置情報
/// </summary>
struct EditorEnemyInfo {
	Vector3 position;
	EnemyType enemyType;
	EnemyPattern pattern;
	int waitTime;
};

/// <summary>
/// 敵生成コマンドクラス
/// </summary>
class EnemyPopCommand {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	EnemyPopCommand();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~EnemyPopCommand();

	/// <summary>
	/// CSVファイルから敵生成データを読み込み
	/// </summary>
	/// <param name="filePath">CSVファイルのパス</param>
	/// <returns>読み込み成功フラグ</returns>
	bool LoadFromCSV(const std::string& filePath);

	/// <summary>
	/// 次のコマンドを取得
	/// </summary>
	/// <param name="commandType">コマンドの種類</param>
	/// <param name="popData">敵生成データ（POPコマンドの場合）</param>
	/// <param name="waitData">待機データ（WAITコマンドの場合）</param>
	/// <returns>コマンドが取得できたかどうか</returns>
	bool GetNextCommand(EnemyPopCommandType& commandType, EnemyPopData& popData, WaitData& waitData);

	/// <summary>
	/// コマンドストリームをリセット
	/// </summary>
	void Reset();

	/// <summary>
	/// 全てのコマンドが処理されたかどうか
	/// </summary>
	/// <returns>全て処理済みかどうか</returns>
	bool IsFinished() const;

	/// <summary>
	/// エディタ用：読み込まれた敵情報をすべて取得
	/// </summary>
	/// <returns>敵配置情報のベクター</returns>
	std::vector<EditorEnemyInfo> GetAllEnemyInfo() const;

	/// <summary>
	/// エディタ用：敵情報をCSVに保存
	/// </summary>
	/// <param name="filePath">保存先ファイルパス</param>
	/// <param name="enemyInfos">敵配置情報</param>
	/// <returns>保存成功フラグ</returns>
	static bool SaveEnemyInfoToCSV(const std::string& filePath, const std::vector<EditorEnemyInfo>& enemyInfos);

private:
	// コマンドストリーム
	std::stringstream commandStream_;

	// エディタ用：読み込まれた敵情報を保持
	std::vector<EditorEnemyInfo> loadedEnemyInfos_;

	/// <summary>
	/// 文字列をEnemyTypeに変換
	/// </summary>
	/// <param name="typeStr">文字列</param>
	/// <returns>EnemyType</returns>
	EnemyType StringToEnemyType(const std::string& typeStr);

	/// <summary>
	/// 文字列をEnemyPatternに変換
	/// </summary>
	/// <param name="patternStr">文字列</param>
	/// <returns>EnemyPattern</returns>
	EnemyPattern StringToEnemyPattern(const std::string& patternStr);

	/// <summary>
	/// EnemyTypeを文字列に変換
	/// </summary>
	/// <param name="type">敵タイプ</param>
	/// <returns>文字列</returns>
	static std::string EnemyTypeToString(EnemyType type);

	/// <summary>
	/// EnemyPatternを文字列に変換
	/// </summary>
	/// <param name="pattern">敵パターン</param>
	/// <returns>文字列</returns>
	static std::string EnemyPatternToString(EnemyPattern pattern);
};