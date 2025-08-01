#include "AudioManager.h"
#include "Managers/ImGui/ImGuiManager.h"

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

	//MediaFoundationのCOMの初期化
	result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	assert(SUCCEEDED(result));
	Logger::Log(Logger::GetStream(), "Complete MFCOM initialized !!\n");

	//MediaFoundationのスタートアップ
	result = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
	assert(SUCCEEDED(result));
	Logger::Log(Logger::GetStream(), "Complete MF initialized !!\n");

	//DirectX初期化の末尾にXAudio2エンジンのインスタンス生成
	result = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(result));

	// マスターボイスの生成
	//音声を鳴らすとき、通る場所
	result = xAudio2->CreateMasteringVoice(&masterVoice);
	assert(SUCCEEDED(result));
	Logger::Log(Logger::GetStream(), "Complete AudioManager initialized !!\n");
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

	// MediaFoundationの終了処理
	MFShutdown();
	Logger::Log(Logger::GetStream(), "MediaFoundation shutdown\n");

	// COMの終了処理
	CoUninitialize();
	Logger::Log(Logger::GetStream(), "COM uninitialized\n");

}

void AudioManager::LoadAudio(const std::string& filename, const std::string& tagName) {
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
	//実際に読み込む（WAV/MP3自動判別して動かす）
	audio->LoadAudio(filename);
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

void AudioManager::Pause(const std::string& tagName) {
	// 指定したタグ名の音声が見つからなければ何もしない
	if (audios.find(tagName) == audios.end()) {
		return;
	}

	// 音声の一時停止
	audios[tagName]->Pause();
}

void AudioManager::Resume(const std::string& tagName) {
	// 指定したタグ名の音声が見つからなければ何もしない
	if (audios.find(tagName) == audios.end()) {
		return;
	}

	// 音声の再開
	audios[tagName]->Resume();
}

void AudioManager::Stop(const std::string& tagName) {
	// 指定したタグ名の音声が見つからなければ何もしない
	if (audios.find(tagName) == audios.end()) {
		return;
	}

	// 音声の停止
	audios[tagName]->Stop();
}

void AudioManager::SetLoop(const std::string& tagName, bool loop) {
	// 指定したタグ名の音声が見つからなければ何もしない
	if (audios.find(tagName) == audios.end()) {
		return;
	}

	// ループ設定の変更
	audios[tagName]->SetLoop(loop);
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

void AudioManager::ImGui()
{

	if (ImGui::CollapsingHeader("AudioManager")) {

		// 読み込まれている音声ファイル数を表示
		ImGui::Text("Files: %zu", audios.size());

		// 同じ行の右側に全停止ボタンを配置
		// SameLine(100)で水平位置100pxに配置
		ImGui::SameLine(100);
		if (ImGui::SmallButton("Stop All")) {
			// 全ての音声を一括停止
			StopAll();
		}

		// 音声ファイルが1つも読み込まれていない場合の処理
		if (audios.empty()) {
			// グレーアウトされたテキストで状態を表示
			ImGui::TextDisabled("No audio loaded");
			return; // 以降の処理をスキップ
		}

		// 区切り線を表示して視覚的に分離
		ImGui::Separator();


		/// 音声ファイル選択

		// 選択中の音声インデックス（静的変数で状態を保持）
		static int selectedAudio = 0;

		// 音声ファイル名のリスト（コンボボックス用）
		static std::vector<std::string> audioNames;

		// 音声名リストを毎フレーム更新
		// （動的に音声が追加/削除される可能性があるため）
		audioNames.clear();
		for (const auto& pair : audios) {
			audioNames.push_back(pair.first);
		}

		// 選択インデックスの範囲チェック
		// 音声ファイルが削除された場合などに配列外アクセスを防ぐ
		if (selectedAudio >= static_cast<int>(audioNames.size())) {
			selectedAudio = 0;
		}

		/// コンボボックスの表示部分

		// コンボボックスの幅を親ウィンドウ全体に設定
		ImGui::PushItemWidth(-1);

		// コンボボックスの表示開始
		// 現在選択されている音声名を表示、音声がない場合は"No Audio"
		if (ImGui::BeginCombo("##AudioSelect",
			audioNames.empty() ? "No Audio" : audioNames[selectedAudio].c_str())) {

			// 全ての音声ファイルをリスト表示
			for (int i = 0; i < static_cast<int>(audioNames.size()); i++) {
				bool isSelected = (selectedAudio == i);
				Audio* audio = audios[audioNames[i]];

				// 再生中の音声ファイルは緑色でハイライト表示
				if (audio && audio->IsPlaying()) {
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.8f, 0.0f, 1.0f));
				}

				// 表示テキストの作成
				std::string displayText = audioNames[i];

				// 再生状態に応じてステータステキストを追加
				if (audio) {
					if (audio->IsPlaying()) {
						if (audio->IsPaused()) {
							displayText += " [Paused]";
						} else if (audio->IsLooping()) {
							displayText += " [Looping]";
						} else {
							displayText += " [Playing]";
						}
					}
				}

				// 選択可能なアイテムとして表示
				if (ImGui::Selectable(displayText.c_str(), isSelected)) {
					selectedAudio = i; // クリックされたら選択状態を更新
				}

				// 緑色スタイルを適用していた場合は元に戻す
				if (audio && audio->IsPlaying()) {
					ImGui::PopStyleColor();
				}

				// 現在選択されているアイテムにフォーカスを設定
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		ImGui::PopItemWidth(); // 幅設定を元に戻す


		/// 選択された音声の詳細


		// 有効な音声が選択されている場合のみコントロールを表示
		if (!audioNames.empty() && selectedAudio < static_cast<int>(audioNames.size())) {
			const std::string& currentTag = audioNames[selectedAudio];
			Audio* currentAudio = audios[currentTag];

			if (currentAudio) {
				// 視覚的な間隔を追加
				ImGui::Spacing();

				/// ステータス表示部分

				// 現在の再生状態を色付きテキストで表示
				if (currentAudio->IsPlaying()) {
					if (currentAudio->IsPaused()) {
						ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Status: Paused");
					} else if (currentAudio->IsLooping()) {
						ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "Status: Looping");
					} else {
						ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.0f, 1.0f), "Status: Playing");
					}
				} else {
					// 停止中はグレー表示
					ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Status: Stopped");
				}


				/// 再生ボタン群

				// 通常再生ボタン
				if (ImGui::Button("Play", ImVec2(80, 0))) {
					Play(currentTag);
				}

				// 同じ行に配置するため SameLine() を使用
				ImGui::SameLine();

				// ループ再生ボタン
				if (ImGui::Button("Play Loop", ImVec2(80, 0))) {
					PlayLoop(currentTag);
				}

				ImGui::SameLine();

				// 一時停止/再開ボタン
				if (currentAudio->IsPaused()) {
					if (ImGui::Button("Resume", ImVec2(80, 0))) {
						Resume(currentTag);
					}
				} else {
					if (ImGui::Button("Pause", ImVec2(80, 0))) {
						Pause(currentTag);
					}
				}

				ImGui::SameLine();

				// 停止ボタン
				if (ImGui::Button("Stop", ImVec2(80, 0))) {
					Stop(currentTag);
				}

				/// ループ設定切り替え

				ImGui::Spacing();

				// 現在のループ状態を取得
				bool isLooping = currentAudio->IsLooping();

				// チェックボックスでループ設定を切り替え
				if (ImGui::Checkbox("Loop Mode", &isLooping)) {
					SetLoop(currentTag, isLooping);
				}

				// ループ設定の説明
				if (isLooping) {
					ImGui::SameLine();
					ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "(Infinite loop)");
				}


				/// 音量調整

				ImGui::Spacing();

				// 各音声の音量を保存するための静的マップ
				// キー：音声タグ名、値：音量値（0.0〜1.0）
				static std::map<std::string, float> volumes;

				// 新しい音声の場合、デフォルト音量（1.0）を設定
				if (volumes.find(currentTag) == volumes.end()) {
					volumes[currentTag] = 1.0f;
				}

				// 音量ラベル表示
				ImGui::Text("Volume:");
				ImGui::SameLine(); // 同じ行にスライダーを配置

				// スライダーの幅を150pxに設定
				ImGui::PushItemWidth(150);

				// 音量調整スライダー（0.0〜1.0の範囲）
				// スライダーが変更された場合、即座に音量を適用
				if (ImGui::SliderFloat("##Volume", &volumes[currentTag], 0.0f, 1.0f, "%.2f")) {
					SetVolume(currentTag, volumes[currentTag]);
				}
				ImGui::PopItemWidth(); // 幅設定を元に戻す


				// 25%音量ボタン
				ImGui::SameLine();
				if (ImGui::SmallButton("25%")) {
					volumes[currentTag] = 0.25f;
					SetVolume(currentTag, 0.25f);
				}

				// 50%音量ボタン
				ImGui::SameLine();
				if (ImGui::SmallButton("50%")) {
					volumes[currentTag] = 0.5f;
					SetVolume(currentTag, 0.5f);
				}

				// 100%音量ボタン（最大音量）
				ImGui::SameLine();
				if (ImGui::SmallButton("100%")) {
					volumes[currentTag] = 1.0f;
					SetVolume(currentTag, 1.0f);
				}
			}
		}
	}

}