#pragma once

//////////////////////////////////////////////////////
//音楽ゲームで使う内部的な処理をまとめたクラスです。
//汎用性はありません。
//////////////////////////////////////////////////////


#include <dinput.h>

class MusicGame {
public:
	MusicGame();
	~MusicGame();

	double CalcKeyPosition(int DIKcode);

private:

	

};

MusicGame::MusicGame() {
}

MusicGame::~MusicGame() {
}