#pragma once

/*
EffectManager�N���X

�G�t�F�N�g�`��Ȃǂɗp����J�E���^���ȒP�ɊǗ��ł���N���X�ł��B
�w�肵�����Ԃ̊ԁA�G�t�F�N�g��`�悵�܂��B

�쐬��:�R�{���C
�쐬����:2017/11/13
*/


#include <vector>
#include <functional>
#include <Windows.h>
#include "HighPrecisionTimer.hpp"


struct EFFECTTIME {
	unsigned long startTimeCnt;
	double length;
	float x;
	float y;
};

struct DRAWEFFECTPARAM {
	float alpha;	// ����:0.0f �s����:1.0f
	float scale;	// ���{�\��:1.0f
	float offsetX;
	float offsetY;
};

typedef std::function<DRAWEFFECTPARAM(float)> draweffectoption_func;


class EffectManager {
private:
	HighPrecisionTimer timer;

	std::vector<EFFECTTIME> effect;
	unsigned long dTimeLength;
	unsigned int uiIndex;

public:
	EffectManager();
	~EffectManager();


	static DRAWEFFECTPARAM drawEffectDefFunc(float timePos);
	

	/*
	�}�l�[�W���𐶐�
	changeDrawingFunc�ɂ�,�G�t�F�N�g���ǂ̂悤�ɕω������ĕ`�悷�邩���`�����֐���n���܂��B
	�����ɂ́A�G�t�F�N�g�̌o�ߎ��Ԃ�0.0f-1.0f�œn����܂��B
	*/
	void Create(wchar_t fileName, const draweffectoption_func& changeDrawingFunc, double timeLength, unsigned int element=50);

	// �G�t�F�N�g�J�n
	void Start(float x, float y);

	// �w�肵�������ŃG�t�F�N�g���J�n
	void Start(float x, float y, double timeLength);

	// �G�t�F�N�g��`��
	bool Draw() const;

	// �S�G�t�F�N�g�����Z�b�g
	void Reset();
};