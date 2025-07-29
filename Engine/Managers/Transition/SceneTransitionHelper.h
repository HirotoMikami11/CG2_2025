#pragma once
#include <string>
#include "TransitionManager.h"
#include "Managers/Scene/SceneManager.h"

/// <summary>
/// シーン遷移を簡単に行うためのヘルパークラス
/// </summary>
class SceneTransitionHelper {
public:
	/// <summary>
	/// フェードを使ってシーン遷移
	/// </summary>
	static void FadeToScene(const std::string& sceneName, float duration = 1.0f) {
		TransitionManager::GetInstance()->FadeTransition(
			duration,
			[sceneName]() {
				// フェードアウト完了時にシーンを切り替え
				SceneManager::GetInstance()->ChangeScene(sceneName);
			}
		);
	}

	/// <summary>
	/// カスタムエフェクトを使ってシーン遷移
	/// </summary>
	static void TransitionToScene(
		const std::string& sceneName,
		const std::string& effectName,
		float enterDuration,
		float exitDuration)
	{
		TransitionManager::GetInstance()->StartTransition(
			effectName,
			enterDuration,
			exitDuration,
			[sceneName]() {
				SceneManager::GetInstance()->ChangeScene(sceneName);
			}
		);
	}

	/// <summary>
	/// 即座にシーン遷移（エフェクトなし）
	/// </summary>
	static void ChangeSceneImmediate(const std::string& sceneName) {
		SceneManager::GetInstance()->ChangeScene(sceneName);
	}

	/// <summary>
	/// 次フレームでシーン遷移（エフェクトなし）
	/// </summary>
	static void SetNextScene(const std::string& sceneName) {
		SceneManager::GetInstance()->SetNextScene(sceneName);
	}

	/// <summary>
	/// トランジション中かどうか
	/// </summary>
	static bool IsTransitioning() {
		return TransitionManager::GetInstance()->IsTransitioning();
	}
};