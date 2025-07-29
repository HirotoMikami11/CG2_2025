#pragma once
#include <string>
#include "MyMath/MyFunction.h"

// 前方宣言
class Enemy;

/// <summary>
/// 敵の状態の基底クラス
/// </summary>
class BaseEnemyState {

public:
	
	
	BaseEnemyState(const std::string& name, Enemy* enemy) : name_(name), enemy_(enemy) {};
	
	// 仮想デストラクタ(適切なデストラクタが呼ばれる？)
	virtual ~BaseEnemyState() = default;

	// 毎フレーム処理
	virtual void Update() = 0;

	// デバッグログ出力
	virtual void DebugLog();

protected:
	// 状態名
	std::string name_;

	// 捜査対象の敵
	Enemy* enemy_ = nullptr;
};
