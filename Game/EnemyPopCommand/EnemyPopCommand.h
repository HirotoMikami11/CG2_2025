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

private:
	// コマンドストリーム
	std::stringstream commandStream_;

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
};