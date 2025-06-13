#include "Audio.h"

Audio::Audio() : pSourceVoice(nullptr), isPlaying(false) {
	soundData = {};
}

Audio::~Audio() {
	Unload();
}
void Audio::LoadAudio(const std::string& filename) {
	HRESULT hr = S_OK;//S_OK入れるとなぜか1MBだけメモリ軽くなる??

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