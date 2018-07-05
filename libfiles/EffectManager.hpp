#pragma once

/*
EffectManagerクラス

エフェクト描画などに用いるカウンタを簡単に管理できるクラスです。
指定した時間の間、エフェクトを描画します。

作成者:山本七海
作成日時:2017/11/13
*/

#include <d3d9.h>
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
	typedef struct _POINT {
		int x, y;
	}POINT;

	typedef struct _SIZE {
		int w, h;
	}SIZE;
	
	typedef struct _PUTTEXINFO {
		int x, y;
		int w, h;
	}PUTTEXINFO;

private:
	IDirect3DDevice9 *lpDev;
	IDirect3DTexture9 *lpTex;

	HighPrecisionTimer timer;

	std::vector<EFFECTTIME> effect;
	unsigned long ulTimeLength;
	unsigned int uiIndex;

	SIZE mTexSize;
	PUTTEXINFO mPutTexInfo;



public:
	EffectManager();
	~EffectManager();


	static DRAWEFFECTPARAM drawEffectDefFunc(float timePos);
	

	/*
	マネージャを生成
	changeDrawingFuncには,エフェクトをどのように変化させて描画するかを定義した関数を渡します。
	引数には、エフェクトの経過時間が0.0f-1.0fで渡されます。
	*/
	void Create(
		const draweffectoption_func& changeDrawingFunc,
		double timeLength,
		unsigned int element=50
	);

	// エフェクト画像を登録
	bool SetEffectImg(
		IDirect3DDevice9 *lpDev,
		wchar_t *ImgFileName,
		int x,
		int y,
		int w,
		int h
	);


	// エフェクト開始
	void Start(float x, float y);

	// 指定した長さでエフェクトを開始
	void Start(float x, float y, double timeLength);

	// エフェクトを描画
	bool Draw() const;

	// 全エフェクトをリセット
	void Reset();


private:
	CreateTexture(wchar_t *fileName);

};