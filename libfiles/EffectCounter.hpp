#pragma once

/*
Counter�N���X

�G�t�F�N�g�`��Ȃǂɗp����J�E���^���ȒP�ɊǗ��ł���N���X�ł��B
�J�E���g�l�͓���X�s�[�h�Ɋ֌W�Ȃ��o�ߎ��Ԃŕω����܂��B

�쐬��:�R�{���C
�쐬����:2017/11/13
*/


#include "CTimer.h"
#include <vector>


struct EFFECTCOUNT {
	unsigned long count;
	float x;
	float y;
};


class EffectCounter {
private:
	CTimer timer;

	std::vector<EFFECTCOUNT> counter;
	unsigned long ulMaxCount;
	unsigned int uiIndex;


private:
	// �S�ẴJ�E���^���X�V
	void Update();

public:
	EffectCounter();
	~EffectCounter();

	// �J�E���^����
	void Create(unsigned int element, unsigned long maxCount, int updateFps=60);

	// �J�E���^���Z�b�g
	void Reset();

	// �J�E���^�J�n
	void CountStart(float x, float y);

	// �J�E���^���w��l�ŊJ�n
	void CountStart(float x, float y, int val);

	const std::vector<EFFECTCOUNT>::const_iterator GetCount();

	void Delete();
};
