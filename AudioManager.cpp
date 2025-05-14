#include "AudioManager.h"

// シングルトンインスタンス
AudioManager* AudioManager::GetInstance() {
	static AudioManager instance;
	return &instance;
}

AudioManager::AudioManager() : masterVoice(nullptr) {
}

AudioManager::~AudioManager() {
	Finalize();
}

void AudioManager::Initialize() {
	HRESULT result;

	//DirectX初期化の末尾にXAudio2エンジンのインスタンス生成
	result = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(result));

	// マスターボイスの生成
	//音声を鳴らすとき、通る場所
	result = xAudio2->CreateMasteringVoice(&masterVoice);
	assert(SUCCEEDED(result));
}

void AudioManager::Finalize() {
	// 全ての音声を解放
	for (auto& pair : audios) {
		if (pair.second) {
			pair.second->Unload();
			delete pair.second;
		}
	}
	audios.clear();

	// マスターボイスの解放
	if (masterVoice) {
		masterVoice->DestroyVoice();
		masterVoice = nullptr;
	}

	// XAudio2の解放
	xAudio2.Reset();
}

void AudioManager::LoadWave(const std::string& filename, const std::string& tagName) {
	// 既に同じタグ名で登録されていた場合は古いものを解放
	if (audios.find(tagName) != audios.end()) {
		if (audios[tagName]) {
			audios[tagName]->Unload();
			delete audios[tagName];
		}
		audios.erase(tagName);
	}

	// 新しい音声データを作成
	Audio* audio = new Audio();
	//実際に読み込む
	audio->LoadWave(filename);
	audios[tagName] = audio;
}

void AudioManager::Play(const std::string& tagName) {
	// 指定したタグ名の音声が見つからなければ何もしない
	if (audios.find(tagName) == audios.end()) {
		return;
	}

	// 音声の再生
	audios[tagName]->Play(xAudio2.Get());
}

void AudioManager::PlayLoop(const std::string& tagName) {
	// 指定したタグ名の音声が見つからなければ何もしない
	if (audios.find(tagName) == audios.end()) {
		return;
	}

	// 音声の再生
	audios[tagName]->PlayLoop(xAudio2.Get());
}


void AudioManager::Stop(const std::string& tagName) {
	// 指定したタグ名の音声が見つからなければ何もしない
	if (audios.find(tagName) == audios.end()) {
		return;
	}

	// 音声の停止
	audios[tagName]->Stop();
}

void AudioManager::SetVolume(const std::string& tagName, float volume) {
	// 指定したタグ名の音声が見つからなければ何もしない
	if (audios.find(tagName) == audios.end()) {
		return;
	}

	// 音量の設定
	audios[tagName]->SetVolume(volume);
}

void AudioManager::StopAll() {
	// 全ての音声を停止
	for (auto& pair : audios) {
		if (pair.second) {
			pair.second->Stop();
		}
	}
}