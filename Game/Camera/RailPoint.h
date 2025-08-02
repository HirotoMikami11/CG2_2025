#pragma once
#include "MyMath/MyFunction.h"
#include <string>

/// <summary>
/// レールカメラの制御点
/// </summary>
class RailPoint {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="position">座標</param>
	/// <param name="name">名前（オプション）</param>
	RailPoint(const Vector3& position = { 0.0f, 0.0f, 0.0f }, const std::string& name = "");

	/// <summary>
	/// デストラクタ
	/// </summary>
	~RailPoint() = default;

	// Getter
	const Vector3& GetPosition() const { return position_; }
	const std::string& GetName() const { return name_; }
	int GetId() const { return id_; }

	// Setter
	void SetPosition(const Vector3& position) { position_ = position; }
	void SetName(const std::string& name) { name_ = name; }

private:
	Vector3 position_;          // 座標
	std::string name_;          // 名前
	int id_;                    // 一意ID

	static int nextId_;         // 次のID
};