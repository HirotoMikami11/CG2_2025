#pragma once
#include <string>
#include "MyMath/MyFunction.h"

// 前方宣言
class BaseEnemy;
class Player;

/// <summary>
/// 敵の状態の基底クラス
/// </summary>
class BaseEnemyState {
public:
	BaseEnemyState(const std::string& name, BaseEnemy* enemy) : name_(name), enemy_(enemy) {};

	// 仮想デストラクタ
	virtual ~BaseEnemyState() = default;

	// 毎フレーム処理
	virtual void Update() = 0;

	// Getter
	const std::string& GetName() const { return name_; }

protected:
	// 状態名
	std::string name_;

	// 操作対象の敵
	BaseEnemy* enemy_ = nullptr;

	/// <summary>
	/// プレイヤーを取得するヘルパー関数
	/// </summary>
	/// <returns>プレイヤーのポインタ</returns>
	Player* GetPlayer() const;
};