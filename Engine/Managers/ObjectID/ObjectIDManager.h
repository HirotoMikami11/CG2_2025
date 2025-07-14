#pragma once
#include <unordered_map>
#include <string>
#include <format>

/// <summary>
/// オブジェクトやスプライトのID管理を統一するクラス
/// </summary>
class ObjectIDManager {
public:
	// シングルトンパターン
	static ObjectIDManager* GetInstance();

	/// <summary>
	/// フォーマットされた名前を生成（自動でカウンターをインクリメント）
	/// </summary>
	/// <param name="baseName">ベース名（例："Triangle", "Sphere"）</param>
	/// <param name="typeName">オブジェクトタイプ名（内部管理用、通常はbaseNameと同じ）</param>
	/// <returns>フォーマット済み名前（例："Triangle_1"）</returns>
	std::string GenerateName(const std::string& baseName, const std::string& typeName = "");

	/// <summary>
	/// 現在のカウンター値を取得
	/// </summary>
	/// <param name="typeName">オブジェクトタイプ名</param>
	/// <returns>現在のカウンター値</returns>
	int GetCurrentCount(const std::string& typeName) const;

	/// <summary>
	/// 特定タイプのカウンターをリセット
	/// </summary>
	/// <param name="typeName">オブジェクトタイプ名</param>
	void ResetCounter(const std::string& typeName);

	/// <summary>
	/// 全カウンターをリセット（シーン切り替え時に使用）
	/// </summary>
	void ResetAllCounters();

	/// <summary>
	/// デバッグ情報の表示（ImGui）
	/// </summary>
	void ImGui();

private:
	ObjectIDManager() = default;
	~ObjectIDManager() = default;
	ObjectIDManager(const ObjectIDManager&) = delete;
	ObjectIDManager& operator=(const ObjectIDManager&) = delete;

	/// <summary>
	/// オブジェクトタイプを登録（内部使用）
	/// </summary>
	void RegisterType(const std::string& typeName);

	/// <summary>
	/// 次のIDを取得してカウンターをインクリメント（内部使用）
	/// </summary>
	int GetNextID(const std::string& typeName);

	// タイプ名とカウンターのマップ
	std::unordered_map<std::string, int> counters_;
};