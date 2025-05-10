#include "Audio.h"

Audio::Audio() : pSourceVoice(nullptr), isPlaying(false) {
	soundData = {};
}

Audio::~Audio() {
	Unload();
}

void Audio::LoadWave(const std::string& filename) {
	// ファイルを開く
	std::ifstream file;
	file.open(filename, std::ios_base::binary);
	// 失敗したら終了
	assert(file.is_open());

	// .wavデータ読み込み
	// RIFFヘッダーを読む
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	// ファイルがRIFFかどうか
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}
	// タイプがWAVEかどうか
	if (strncmp(riff.type, "WAVE", 4) != 0) {
		assert(0);
	}

	// Formatチャンク読み込み
	FormatChunk format = {};
	// チャンクヘッダーの確認
	file.read((char*)&format, sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
		assert(0);
	}
	// チャンク本体の読み込み
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt, format.chunk.size);

	// Dataチャンクの読み込み
	ChunkHeader data;
	file.read((char*)&data, sizeof(data));
	// JUNKチャンクはダミーデータなので、検出した場合はそれを読み飛ばす
	if (strncmp(data.id, "JUNK", 4) == 0) {
		// 読み取りの位置をJUNKチャンクの終わりまで進める
		file.seekg(data.size, std::ios_base::cur);
		// 再読み込み
		file.read((char*)&data, sizeof(data));
	}

	if (strncmp(data.id, "data", 4) != 0) {
		assert(0);
	}

	// Dataチャンクの波型データを読み込む
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	// ファイルを閉じる
	file.close();

	// 読み込んだ音声データを設定
	soundData.wfex = format.fmt;							// 波型フォーマット
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);	// 波型データ
	soundData.bufferSize = data.size;						// 波型データのサイズ
}

void Audio::Play(IXAudio2* xAudio2) {
	HRESULT result;

	// 既に再生中なら何もしない
	if (isPlaying) {
		return;
	}

	// 波型フォーマットをもとにSourceVoiceを生成
	result = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));

	// 再生する波型データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	// 波型データの再生
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	assert(SUCCEEDED(result));

	result = pSourceVoice->Start();
	assert(SUCCEEDED(result));

	isPlaying = true;
}

void Audio::PlayLoop(IXAudio2* xAudio2) {
	HRESULT result;

	// 既に再生中なら何もしない
	if (isPlaying) {
		return;
	}

	// 波型フォーマットをもとにSourceVoiceを生成
	result = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));

	// 再生する波型データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;
	buf.LoopCount = XAUDIO2_LOOP_INFINITE;  // 無限ループする設定

	// 波型データの再生
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	assert(SUCCEEDED(result));

	result = pSourceVoice->Start();
	assert(SUCCEEDED(result));

	isPlaying = true;
}


void Audio::SetVolume(float volume) {
	///ボリュームを入れる
	if (pSourceVoice) {
		pSourceVoice->SetVolume(volume);
	}
}

void Audio::Stop() {
	///停止して削除
	if (pSourceVoice) {
		pSourceVoice->Stop();
		pSourceVoice->FlushSourceBuffers();
		isPlaying = false;
	}
}

void Audio::Unload() {
	// 再生停止しておく一応
	Stop();

	// ソースボイスの解放
	if (pSourceVoice) {//あるなら
		pSourceVoice->DestroyVoice();
		pSourceVoice = nullptr;
	}

	// バッファのメモリを解放
	if (soundData.pBuffer) {//あるなら
		delete[] soundData.pBuffer;
		soundData.pBuffer = nullptr;
	}

	soundData.bufferSize = 0;
	soundData.wfex = {};
	isPlaying = false;
}

bool Audio::IsPlaying() const {
	return isPlaying;
}