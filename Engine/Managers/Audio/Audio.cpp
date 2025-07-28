#include "Audio.h"

Audio::Audio() : pSourceVoice(nullptr), isPlaying(false), isPaused(false), isLooping(false), pausedSamplesPlayed(0) {
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
	if (isPlaying && !isPaused) {
		return;
	}

	// 一時停止中の場合は再開
	if (isPaused) {
		Resume();
		return;
	}

	// 既存のSourceVoiceがある場合は削除
	if (pSourceVoice) {
		pSourceVoice->DestroyVoice();
		pSourceVoice = nullptr;
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
	isPaused = false;
	isLooping = false;
	pausedSamplesPlayed = 0;
}

void Audio::PlayLoop(IXAudio2* xAudio2) {
	HRESULT result;

	// 既に再生中なら何もしない
	if (isPlaying && !isPaused) {
		return;
	}

	// 一時停止中の場合は再開
	if (isPaused) {
		Resume();
		return;
	}

	// 既存のSourceVoiceがある場合は削除
	if (pSourceVoice) {
		pSourceVoice->DestroyVoice();
		pSourceVoice = nullptr;
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
	isPaused = false;
	isLooping = true;
	pausedSamplesPlayed = 0;
}


void Audio::SetVolume(float volume) {
	///ボリュームを入れる
	if (pSourceVoice) {
		pSourceVoice->SetVolume(volume);
	}
}

void Audio::Pause() {
	if (pSourceVoice && isPlaying && !isPaused) {
		// 現在の再生位置を保存
		XAUDIO2_VOICE_STATE state;
		pSourceVoice->GetState(&state);
		pausedSamplesPlayed = state.SamplesPlayed;

		// 一時停止
		pSourceVoice->Stop();
		isPaused = true;
	}
}

void Audio::Resume() {
	if (pSourceVoice && isPlaying && isPaused) {
		// 一時停止位置から再開
		pSourceVoice->Start();
		isPaused = false;
	}
}

void Audio::Stop() {
	///停止して削除
	if (pSourceVoice) {
		pSourceVoice->Stop();
		pSourceVoice->FlushSourceBuffers();
		isPlaying = false;
		isPaused = false;
		pausedSamplesPlayed = 0;
	}
}

void Audio::SetLoop(bool loop) {
	if (pSourceVoice && isPlaying) {
		// 再生中の場合、一旦停止して再設定
		XAUDIO2_VOICE_STATE state;
		pSourceVoice->GetState(&state);
		UINT64 currentSample = state.SamplesPlayed;

		// 現在の音量を保存
		float currentVolume = 1.0f;
		pSourceVoice->GetVolume(&currentVolume);

		// 一旦停止
		pSourceVoice->Stop();
		pSourceVoice->FlushSourceBuffers();

		// 新しい設定でバッファを再設定
		XAUDIO2_BUFFER buf{};
		buf.pAudioData = soundData.pBuffer;
		buf.AudioBytes = soundData.bufferSize;
		buf.Flags = XAUDIO2_END_OF_STREAM;

		if (loop) {
			buf.LoopCount = XAUDIO2_LOOP_INFINITE;
		}

		// 再生位置を調整（可能な範囲で）
		UINT32 bytesPerSample = soundData.wfex.wBitsPerSample / 8 * soundData.wfex.nChannels;
		UINT32 playBegin = static_cast<UINT32>((currentSample % (soundData.bufferSize / bytesPerSample)) * bytesPerSample);

		if (playBegin < soundData.bufferSize) {
			buf.PlayBegin = playBegin / bytesPerSample;
		}

		// バッファを再送信
		pSourceVoice->SubmitSourceBuffer(&buf);

		// 音量を復元
		pSourceVoice->SetVolume(currentVolume);

		// 再生再開
		pSourceVoice->Start();

		isLooping = loop;
	} else {
		// 再生していない場合は単にフラグを設定
		isLooping = loop;
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
	isPaused = false;
	isLooping = false;
	pausedSamplesPlayed = 0;
}

bool Audio::IsPlaying() {
	UpdatePlayState();
	return isPlaying;
}

bool Audio::IsPaused() const {
	return isPaused;
}

bool Audio::IsLooping() const {
	return isLooping;
}

void Audio::UpdatePlayState() {
	if (pSourceVoice && isPlaying && !isPaused) {
		XAUDIO2_VOICE_STATE state;
		pSourceVoice->GetState(&state);

		// バッファが空で、キューに何もない場合は再生終了
		if (state.BuffersQueued == 0 && !isLooping) {
			isPlaying = false;
		}
	}
}