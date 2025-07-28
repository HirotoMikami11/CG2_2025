#pragma once
#include <xaudio2.h>
#include <wrl.h>
#include <map>
#include <string>
#include <cassert>

#include "Managers/Audio/Audio.h"
#include "BaseSystem/Logger/Logger.h"


/// <summary>
/// 音声を管理する管理クラス
/// </summary>
class AudioManager {
public:
	// ゲームアプリケーション全体で一つのインスタンスを使う（シングルトン）
	static AudioManager* GetInstance();

	// 初期化
	void Initialize();
	// 終了処理
	void Finalize();

	/// <summary>
	/// 音声データの読み込み（WAV/MP3対応）
	/// </summary>
	/// <param name="filename"></param>
	/// <param name="tagName"></param>
	void LoadAudio(const std::string& filename, const std::string& tagName);

	/// <summary>
	/// 音声の再生(ループなし)
	/// </summary>
	/// <param name="tagName"></param>
	void Play(const std::string& tagName);

	/// <summary>
	/// 音声の再生(ループあり)
	/// </summary>
	/// <param name="tagName"></param>
	void PlayLoop(const std::string& tagName);

	/// <summary>
	/// 音声の一時停止
	/// </summary>
	/// <param name="tagName"></param>
	void Pause(const std::string& tagName);

	/// <summary>
	/// 音声の再開（一時停止位置から）
	/// </summary>
	/// <param name="tagName"></param>
	void Resume(const std::string& tagName);

	/// <summary>
	/// 音声の完全停止
	/// </summary>
	/// <param name="tagName"></param>
	void Stop(const std::string& tagName);

	/// <summary>
	/// ループ設定の変更
	/// </summary>
	/// <param name="tagName"></param>
	/// <param name="loop"></param>
	void SetLoop(const std::string& tagName, bool loop);

	/// <summary>
	/// 音量の設定
	/// </summary>
	/// <param name="tagName"></param>
	/// <param name="volume"></param>
	void SetVolume(const std::string& tagName, float volume);

	/// <summary>
	/// 全ての音声を停止
	/// </summary>
	void StopAll();

	/// <summary>
	/// ImGui
	/// </summary>
	void ImGui();

private:
	// コンストラクタ
	AudioManager();
	// デストラクタ
	~AudioManager();

	// コピーを禁止
	AudioManager(const AudioManager&) = delete;
	AudioManager& operator=(const AudioManager&) = delete;

	// XAudio2のインスタンス
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2;
	// マスターボイス
	IXAudio2MasteringVoice* masterVoice;
	// audio
	std::map<std::string, Audio*> audios;

};