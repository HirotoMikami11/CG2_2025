#pragma once
#include <memory>
#include"Scenes/GameScene/GameScene.h"

//ゲームシーン

class Game
{
public:
	Game();
	~Game();

	void Initialize();
	void Update();
	void Draw();
	void Finalize();

private:

	//シーンマネージャー
	
	//ゲームシーン	gameScene_;		(実際のゲームシーン)
	std::unique_ptr<GameScene> gameScene_;
	//デモシーン		demoScene_;		(とりあえず動かすシーン)
	//デバッグシーン	debugScene_;	(デバッグ用のシーン)

};

