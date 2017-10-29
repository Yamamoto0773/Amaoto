#include "HighPrecisionTimer.hpp"

HighPrecisionTimer::HighPrecisionTimer() {
	llStartCount.QuadPart = 0;
	llPauseStCount.QuadPart = 0;
	llFrequency.QuadPart = 0;

	bIsPaused = true;
	bIsReset = true;
	dTime = 0.0;

	// ���g�����擾
	if (QueryPerformanceFrequency(&llFrequency)) {
		bIsHighPrecValid = true;
	}
	else {
		/*
		�����x�^�C�}�[���g�p�ł��Ȃ��ꍇ�AtimeGetTime�֐��ő�p���܂��B
		���̍ہA���x��1ms�Ŏ擾����̂Ŏ��g����1000Hz�ɐݒ肵�܂��B
		*/

		llFrequency.QuadPart = 1000LL;
	}

}


HighPrecisionTimer::~HighPrecisionTimer() {
}


void HighPrecisionTimer::Start() {

	if (bIsPaused) {
		LARGE_INTEGER now;

		// ���݂̃J�E���g���擾
		if (bIsHighPrecValid) {
			QueryPerformanceCounter(&now);
		}
		else {
			timeBeginPeriod(1);
			now.QuadPart = (long long)timeGetTime();
			timeEndPeriod(1);
		}

		// �^�C�}�[�J�n�̎��Ԃ�ύX
		if (bIsReset) {
			llStartCount.QuadPart = now.QuadPart;
		}
		else {
			// �ꎞ��~�������ԕ��������炷
			llStartCount.QuadPart += now.QuadPart - llPauseStCount.QuadPart;
		}
		

		bIsPaused = false;
		bIsReset = false;
	}

}


void HighPrecisionTimer::Pause() {

	if (bIsPaused) {
		/*
		���Ɉꎞ��~���̏ꍇ�͉������܂���
		*/
	}
	else {

		// �ꎞ��~�J�n���̃J�E���g���擾
		if (bIsHighPrecValid) {
			QueryPerformanceCounter(&llPauseStCount);
		}
		else {
			timeBeginPeriod(1);
			llPauseStCount.QuadPart = (long long)timeGetTime();
			timeEndPeriod(1);
		}

		// �ꎞ��~���̃^�C����ۑ�
		dTime = (double)(llPauseStCount.QuadPart - llStartCount.QuadPart)/llFrequency.QuadPart;

	}

	bIsPaused = true;

}


void HighPrecisionTimer::Reset() {

	if (bIsPaused) {
		dTime = 0.0;

		bIsReset = true;
	}
	else {
		/*
		�ꎞ��~���łȂ��Ɠ��삵�܂���
		*/
	}

}


double HighPrecisionTimer::GetTime() {

	if (bIsPaused == false) {

		// �^�C�����X�V
		LARGE_INTEGER now;
		if (bIsHighPrecValid) {
			QueryPerformanceCounter(&now);
		}
		else {
			timeBeginPeriod(1);
			now.QuadPart = timeGetTime();
			timeEndPeriod(1);
		}

		dTime = (double)(now.QuadPart - llStartCount.QuadPart)/llFrequency.QuadPart;
	}

	return dTime;
}
