#pragma once


#include "libfiles/CDIPro81.h"
#include "libfiles/MIDIinput.h"


class GameController {
	static const int keyCount = 25;		// ���̓L�[�̐�
	
private:
	MIDIInput mi;
	CDIPro81 di;
	

	bool bIsKeyOn[keyCount];
	bool bIsKeyPressed[keyCount];
	
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
	void GetKeyState(bool* &hold, bool* &pressed);

	// �w��ԍ��̓��̓f�o�C�X(hold)�̏�Ԃ��擾
	const bool IsKeyOn(int num) const;

	// �w��ԍ��̓��̓f�o�C�X(pressed)�̏�Ԃ��擾
	const bool IsKeyPressed(int num) const;

	// ��Еt��
	bool Clear();
	
};