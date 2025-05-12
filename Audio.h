#pragma once

#include <string>
#include <xaudio2.h>
#include <wrl.h>
#include <cstdint>

#include <cassert>
#include <fstream>

/// <summary>
/// 音声データ構造体
/// </summary>
struct SoundData {
	// 波形フォーマット
	WAVEFORMATEX wfex;
	// バッファの先頭アドレス
	BYTE* pBuffer;
	// バッファサイズ
	unsigned int bufferSize;
};

/// <summary>
/// チャンクヘッダ
/// </summary>
struct ChunkHeader {
	char id[4];			// チャンクごとのid
	int32_t size;		// チャンクサイズ
};

/// <summary>
/// RIFFヘッダチャンク
/// </summary>
struct RiffHeader {
	ChunkHeader chunk;	// RIFF
	char type[4];		// WAVE
};

/// <summary>
/// FMTチャンク
/// </summary>
struct FormatChunk {
	ChunkHeader chunk;	// fmt
	WAVEFORMATEX fmt;	// 波型フォーマット(18byteまで対応)
};

/// <summary>
/// 音声クラス
/// </summary>
class Audio {
public:
	// コンストラクタ
	Audio();
	// デストラクタ
	~Audio();


	/// <summary>
	/// 音声データの読み込み
	/// </summary>
	/// <param name="filename">ファイルパス</param>
	/// <returns></returns>
	void LoadWave(const std::string& filename);

	/// <summary>
	/// 音声の再生（ループなし）
	/// </summary>
	/// <param name="xAudio2"></param>
	void Play(IXAudio2* xAudio2);

	/// <summary>
	/// 音声の再生（ループあり）
	/// </summary>
	/// <param name="xAudio2"></param>
	void PlayLoop(IXAudio2* xAudio2);

	/// <summary>
	/// 音量設定
	/// </summary>
	/// <param name="volume"></param>
	void SetVolume(float volume);

	/// <summary>
	/// 音声の停止
	/// </summary>
	void Stop();

	/// <summary>
	/// 音声データの解放
	/// </summary>
	void Unload();

	/// <summary>
	/// 再生中かどうか
	/// </summary>
	/// <returns></returns>
	bool IsPlaying() const;

private:
	// 音声データ
	SoundData soundData;
	// ソースボイス
	IXAudio2SourceVoice* pSourceVoice;
	// 再生中フラグ
	bool isPlaying;
};