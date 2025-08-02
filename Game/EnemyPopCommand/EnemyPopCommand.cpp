#include "EnemyPopCommand.h"
#include "BaseSystem/Logger/Logger.h"

EnemyPopCommand::EnemyPopCommand() {
}

EnemyPopCommand::~EnemyPopCommand() {
}

bool EnemyPopCommand::LoadFromCSV(const std::string& filePath) {
	// ファイルを開く
	std::ifstream file(filePath);
	if (!file.is_open()) {
		Logger::Log(Logger::GetStream(), "EnemyPopCommand: Failed to open file: " + filePath + "\n");
		return false;
	}

	// ストリームをクリア
	commandStream_.clear();
	commandStream_.str("");

	// ファイルの内容を丸ごと文字列ストリームにコピー
	commandStream_ << file.rdbuf();

	// ファイルを閉じる
	file.close();

	Logger::Log(Logger::GetStream(), "EnemyPopCommand: Successfully loaded file: " + filePath + "\n");
	return true;
}

bool EnemyPopCommand::GetNextCommand(EnemyPopCommandType& commandType, EnemyPopData& popData, WaitData& waitData) {
	std::string line;

	// コマンド実行ループ（コマンドは一行単位なので一行づつ取り出す）
	while (std::getline(commandStream_, line)) {
		// 一行分の文字列をストリームに変換して解析しやすくする
		std::istringstream lineStream(line);

		// 一行の中から[,]が現れるまでwordに入れる
		std::string word;
		// カンマ区切りで行の先頭文字列を取得
		std::getline(lineStream, word, ',');

		// "//"から始まる行はコメントなのでスキップ
		if (word.find("//") == 0) {
			continue;
		}

		// 空行をスキップ
		if (word.empty()) {
			continue;
		}

		// POPコマンド
		if (word.find("POP") == 0) {
			commandType = EnemyPopCommandType::POP;

			// x座標
			std::getline(lineStream, word, ',');
			popData.position.x = static_cast<float>(std::atof(word.c_str()));

			// y座標
			std::getline(lineStream, word, ',');
			popData.position.y = static_cast<float>(std::atof(word.c_str()));

			// z座標
			std::getline(lineStream, word, ',');
			popData.position.z = static_cast<float>(std::atof(word.c_str()));

			// 敵の種類
			std::getline(lineStream, word, ',');
			popData.enemyType = StringToEnemyType(word);

			// パターンの指定
			std::getline(lineStream, word, ',');
			popData.pattern = StringToEnemyPattern(word);

			return true;
		}
		// WAITコマンド
		else if (word.find("WAIT") == 0) {
			commandType = EnemyPopCommandType::WAIT;

			std::getline(lineStream, word, ',');
			// 待ち時間
			waitData.waitTime = atoi(word.c_str());

			return true;
		}
	}

	// コマンドが見つからない場合
	return false;
}

void EnemyPopCommand::Reset() {
	// ストリームの位置を先頭に戻す
	commandStream_.clear();
	commandStream_.seekg(0);
}

bool EnemyPopCommand::IsFinished() const {
	// ストリームの終端に達しているかチェック
	return commandStream_.eof();
}

EnemyType EnemyPopCommand::StringToEnemyType(const std::string& typeStr) {
	if (typeStr == "Normal" || typeStr == "0") {
		return EnemyType::Normal;
	} else if (typeStr == "RushingFish" || typeStr == "1") {
		return EnemyType::RushingFish;
	}

	// デフォルトは通常の敵
	return EnemyType::Normal;
}

EnemyPattern EnemyPopCommand::StringToEnemyPattern(const std::string& patternStr) {
	if (patternStr == "Straight" || patternStr == "0") {
		return EnemyPattern::Straight;
	} else if (patternStr == "LeaveLeft" || patternStr == "1") {
		return EnemyPattern::LeaveLeft;
	} else if (patternStr == "LeaveRight" || patternStr == "2") {
		return EnemyPattern::LeaveRight;
	} else if (patternStr == "Homing" || patternStr == "3") {
		return EnemyPattern::Homing;
	}

	// デフォルトは直進
	return EnemyPattern::Straight;
}