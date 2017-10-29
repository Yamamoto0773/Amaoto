#pragma once

/*
HighPrecisionTimerクラス

概要:時間を高精度(1ms以下)で測定することができるクラスです。

実際のストップウォッチ同じ挙動をするように、プログラムを設計しました。
関数の呼び出しも、ストップウォッチと同じように行って下さい。


作成者:山本七海
作成日時:2017/10/29
参考:http://www.charatsoft.com/develop/otogema/

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

	// 高精度タイマーが有効かどうかを返す
	bool IsHighPrecisionValid() { return bIsHighPrecValid; }

	// タイマーの分解能を返す
	long long GetResolutionFreq() { return llFrequency.QuadPart; }

	// タイマーをスタート
	void Start();

	// タイマーを一時停止
	void Pause();

	// タイマーをリセット
	void Reset();

	// 時間を取得
	double GetTime();
};