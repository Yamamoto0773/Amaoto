#pragma once

/*
HighPrecisionTimer�N���X

�T�v:���Ԃ������x(1ms�ȉ�)�ő��肷�邱�Ƃ��ł���N���X�ł��B

���ۂ̃X�g�b�v�E�H�b�`��������������悤�ɁA�v���O������݌v���܂����B
�֐��̌Ăяo�����A�X�g�b�v�E�H�b�`�Ɠ����悤�ɍs���ĉ������B


�쐬��:�R�{���C
�쐬����:2017/10/29
�Q�l:http://www.charatsoft.com/develop/otogema/

*/



#include <Windows.h>

class HighPrecisionTimer {
private:
	LARGE_INTEGER llFrequency;

	LARGE_INTEGER llStartCount;
	LARGE_INTEGER llPauseStCount;
	
	double dTime;

	bool bIsHighPrecValid;
	bool bIsPaused;
	bool bIsReset;


public:
	HighPrecisionTimer();
	~HighPrecisionTimer();

	// �����x�^�C�}�[���L�����ǂ�����Ԃ�
	bool IsHighPrecisionValid() { return bIsHighPrecValid; }

	// �^�C�}�[�̕���\��Ԃ�
	long long GetResolutionFreq() { return llFrequency.QuadPart; }

	// �^�C�}�[���X�^�[�g
	void Start();

	// �^�C�}�[���ꎞ��~
	void Pause();

	// �^�C�}�[�����Z�b�g
	void Reset();

	// ���Ԃ��擾
	double GetTime();
};