#pragma once

/*
Counterクラス

エフェクト描画などに用いるカウンタを簡単に管理できるクラスです。
カウント値は動作スピードに関係なく経過時間で変化します。

作成者:山本七海
作成日時:2017/11/13
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
	// 全てのカウンタを更新
	void Update();

public:
	EffectCounter();
	~EffectCounter();

	// カウンタ生成
	void Create(unsigned int element, unsigned long maxCount, int updateFps=60);

	// カウンタリセット
	void Reset();

	// カウンタ開始
	void CountStart(float x, float y);

	// カウンタを指定値で開始
	void CountStart(float x, float y, int val);

	const std::vector<EFFECTCOUNT>::const_iterator GetCount();

	void Delete();
};
