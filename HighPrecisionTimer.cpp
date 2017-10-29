#include "HighPrecisionTimer.hpp"

HighPrecisionTimer::HighPrecisionTimer() {
	llStartCount.QuadPart = 0;
	llPauseStCount.QuadPart = 0;
	llFrequency.QuadPart = 0;

	bIsPaused = true;
	bIsReset = true;
	dTime = 0.0;

	// 周波数を取得
	if (QueryPerformanceFrequency(&llFrequency)) {
		bIsHighPrecValid = true;
	}
	else {
		/*
		高精度タイマーが使用できない場合、timeGetTime関数で代用します。
		その際、精度を1msで取得するので周波数を1000Hzに設定します。
		*/

		llFrequency.QuadPart = 1000LL;
	}

}


HighPrecisionTimer::~HighPrecisionTimer() {
}


void HighPrecisionTimer::Start() {

	if (bIsPaused) {
		LARGE_INTEGER now;

		// 現在のカウントを取得
		if (bIsHighPrecValid) {
			QueryPerformanceCounter(&now);
		}
		else {
			timeBeginPeriod(1);
			now.QuadPart = (long long)timeGetTime();
			timeEndPeriod(1);
		}

		// タイマー開始の時間を変更
		if (bIsReset) {
			llStartCount.QuadPart = now.QuadPart;
		}
		else {
			// 一時停止した時間分だけずらす
			llStartCount.QuadPart += now.QuadPart - llPauseStCount.QuadPart;
		}
		

		bIsPaused = false;
		bIsReset = false;
	}

}


void HighPrecisionTimer::Pause() {

	if (bIsPaused) {
		/*
		既に一時停止中の場合は何もしません
		*/
	}
	else {

		// 一時停止開始時のカウントを取得
		if (bIsHighPrecValid) {
			QueryPerformanceCounter(&llPauseStCount);
		}
		else {
			timeBeginPeriod(1);
			llPauseStCount.QuadPart = (long long)timeGetTime();
			timeEndPeriod(1);
		}

		// 一時停止時のタイムを保存
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
		一時停止中でないと動作しません
		*/
	}

}


double HighPrecisionTimer::GetTime() {

	if (bIsPaused == false) {

		// タイムを更新
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
