#pragma once


#include "libfiles/CDIPro81.h"
#include "libfiles/MIDIinput.h"


#define KEYCOUNT 25		// 入力キーの数

class GameController {	
private:
	MIDIInput mi;
	CDIPro81 di;
	

	bool bIsKeyOn[KEYCOUNT];
	bool bIsKeyPressed[KEYCOUNT];
	
	// キーの位置を指定した値の範囲で返す
	bool GetKeyPosition(int DIKcode, float min, float max, double *posX=nullptr, double *posY=nullptr);	 

public:
	GameController();
	virtual ~GameController();

	// クラスの初期化
	bool Init(HWND hnd, HINSTANCE hInstance);

	// 入力状態の更新
	bool Update();

	// 入力状態の取得
	void GetKeyState(bool* hold, bool* pressed);

	// 指定番号の入力デバイス(hold)の状態を取得
	const bool IsKeyOn(int num) const;

	// 指定番号の入力デバイス(pressed)の状態を取得
	const bool IsKeyPressed(int num) const;

	// 後片付け
	bool Clear();
	
};