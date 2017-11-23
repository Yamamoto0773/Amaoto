#pragma once

/*
EffectManagerクラス

エフェクト描画などに用いるカウンタを簡単に管理できるクラスです。
指定した時間の間、エフェクトを描画します。

作成者:山本七海
作成日時:2017/11/13
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
	float alpha;	// 透明:0.0f 不透明:1.0f
	float scale;	// 等倍表示:1.0f
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
	マネージャを生成
	changeDrawingFuncには,エフェクトをどのように変化させて描画するかを定義した関数を渡します。
	引数には、エフェクトの経過時間が0.0f-1.0fで渡されます。
	*/
	void Create(wchar_t fileName, const draweffectoption_func& changeDrawingFunc, double timeLength, unsigned int element=50);

	// エフェクト開始
	void Start(float x, float y);

	// 指定した長さでエフェクトを開始
	void Start(float x, float y, double timeLength);

	// エフェクトを描画
	bool Draw() const;

	// 全エフェクトをリセット
	void Reset();
};