#include "Audio.h"

Audio::Audio() : pSourceVoice(nullptr), isPlaying(false) {
	soundData = {};
}

Audio::~Audio() {
	Unload();
}
void Audio::LoadAudio(const std::string& filename) {
	// ファイル拡張子を取得して適切な読み込み方法を選択
	std::string extension = GetFileExtension(filename);//この関数のせいで多分メモリ3MBくらい増える

	if (extension == ".wav") {
		LoadWave(filename);					//Waveファイルを読みこむ
	} else if (extension == ".mp3") {
		LoadMP3(filename);					//MP3ファイルを読み込む
	} else {
		assert(false && "Not Wave Of MP3");	//知らない拡張子なら止める
	}
}

std::string Audio::GetFileExtension(const std::string& filename) {
	// ファイル名から最後のドット（.）の位置を検索
	size_t dotPos = filename.find_last_of('.');

	// ドットが見つからない場合は空文字列を返す
	if (dotPos == std::string::npos) {
		return "";
	}

	// ドット位置から末尾までの文字列を抽出（ドットも含む）
	std::string extension = filename.substr(dotPos);

	// 拡張子を小文字に変換
	// ".MP3" なら".mp3",".WAV"なら".wav"
	std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
	return extension;
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

void Audio::LoadMP3(const std::string& filename) {
	// MediaFoundationを使用してMP3をPCMに変換
	ConvertMP3ToPCM(filename);
}

void Audio::ConvertMP3ToPCM(const std::string& filename) {
	HRESULT hr = S_OK;//S_OK入れるとなぜか1MBだけメモリ軽くなる

	///*-----------------------------------------------------------------------*///
	///								ソースリーダーの作成							///
	///*-----------------------------------------------------------------------*///
	// ファイル名をワイド文字列に変換
	int wideLength = MultiByteToWideChar(CP_UTF8, 0, filename.c_str(), -1, nullptr, 0);
	//filenameをソースリーダーで読み込める形式に変更
	std::wstring wfilename(wideLength - 1, L'\0'); // null終端文字を除く?
	MultiByteToWideChar(CP_UTF8, 0, filename.c_str(), -1, &wfilename[0], wideLength);

	// ソースリーダーの実体作成
	Microsoft::WRL::ComPtr<IMFSourceReader> pMFSourceReader;
	hr = MFCreateSourceReaderFromURL(wfilename.c_str(), nullptr, &pMFSourceReader);

	///*-----------------------------------------------------------------------*///
	///								メディアタイプの取得							///
	///*-----------------------------------------------------------------------*///
	// 出力メディアタイプの設定（PCM形式）
	Microsoft::WRL::ComPtr<IMFMediaType> pMFMediaType;
	hr = MFCreateMediaType(&pMFMediaType);
	// PCMフォーマットを設定
	pMFMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	pMFMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
	hr = pMFSourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, pMFMediaType.Get());
	assert(SUCCEEDED(hr));
	//ここでReleaseして生成しなおそうとして失敗したので別のを用意してそこに代入する形で解決する

	// 実際のメディアタイプを取得
	Microsoft::WRL::ComPtr<IMFMediaType> pActualMediaType;
	hr = pMFSourceReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &pActualMediaType);
	assert(SUCCEEDED(hr));

	///*-----------------------------------------------------------------------*///
	///							オーディオデータ形式の作成							///
	///*-----------------------------------------------------------------------*///

	// WAVEFORMATEXを取得
	WAVEFORMATEX* pWaveFormat{ nullptr };
	hr = MFCreateWaveFormatExFromMFMediaType(pActualMediaType.Get(), &pWaveFormat, nullptr);
	assert(SUCCEEDED(hr));

	// WAVEFORMATEXをコピー
	soundData.wfex = *pWaveFormat;
	CoTaskMemFree(pWaveFormat);


	///*-----------------------------------------------------------------------*///
	///								データの読み込み								///
	///*-----------------------------------------------------------------------*///

	// 音声データを読み込む
	std::vector<BYTE> mediaData;
	DWORD streamIndex = 0;
	DWORD dwStreamFlags = 0;
	LONGLONG timestamp = 0;

	while (true) {
		Microsoft::WRL::ComPtr<IMFSample> pSample;

		hr = pMFSourceReader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, &streamIndex, &dwStreamFlags, &timestamp, &pSample);

		if (FAILED(hr)) {
			break;
		}

		if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM) {
			break;
		}


		Microsoft::WRL::ComPtr<IMFMediaBuffer> pMFMediaBuffer;
		hr = pSample->ConvertToContiguousBuffer(&pMFMediaBuffer);

		if (SUCCEEDED(hr)) {
			BYTE* pBuffer = nullptr;
			DWORD cbCurrentLength = 0;
			hr = pMFMediaBuffer->Lock(&pBuffer, nullptr, &cbCurrentLength);

			if (SUCCEEDED(hr)) {
				// データを追加
				size_t currentSize = mediaData.size();
				mediaData.resize(currentSize + cbCurrentLength);
				memcpy(mediaData.data() + currentSize, pBuffer, cbCurrentLength);
				pMFMediaBuffer->Unlock();
			}
		}

	}

	///*-----------------------------------------------------------------------*///
	///							データをメンバ変数に渡す							///
	///*-----------------------------------------------------------------------*///
	
	soundData.bufferSize = static_cast<unsigned int>(mediaData.size());
	soundData.pBuffer = new BYTE[soundData.bufferSize];
	memcpy(soundData.pBuffer, mediaData.data(), soundData.bufferSize);

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