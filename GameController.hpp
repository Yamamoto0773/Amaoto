#pragma once


#include "libfiles/CDIPro81.h"
#include "libfiles/MIDIinput.h"


#define KEYCOUNT 25		// ���̓L�[�̐�

class GameController {	
private:
	MIDIInput mi;
	CDIPro81 di;
	

	bool bIsKeyOn[KEYCOUNT];
	bool bIsKeyPressed[KEYCOUNT];
	
	// �L�[�̈ʒu���w�肵���l�͈̔͂ŕԂ�
	bool GetKeyPosition(int DIKcode, float min, float max, double *posX=nullptr, double *posY=nullptr);	 

public:
	GameController();
	virtual ~GameController();

	// �N���X�̏�����
	bool Init(HWND hnd, HINSTANCE hInstance);

	// ���͏�Ԃ̍X�V
	bool Update();

	// ���͏�Ԃ̎擾
	void GetKeyState(bool* hold, bool* pressed);

	// �w��ԍ��̓��̓f�o�C�X(hold)�̏�Ԃ��擾
	const bool IsKeyOn(int num) const;

	// �w��ԍ��̓��̓f�o�C�X(pressed)�̏�Ԃ��擾
	const bool IsKeyPressed(int num) const;

	// ��Еt��
	bool Clear();
	
};