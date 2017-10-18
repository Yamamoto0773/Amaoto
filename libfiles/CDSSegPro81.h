#ifndef _CDSSEGPRO81_H
#define _CDSSEGPRO81_H
///////////////////////////////////////////////////////////////////////////////////
// CDSSegPro81 : DirectSoundBuffer管理クラス for おとゲマ v2.00                  //
// ※このクラスはCDSPro81で使用されるため直接使用する必要はありません。          //
//                                                                               //
// このソースコードは自由に改変して使用可能です。                                //
// また商用利用も可能ですが、すべての環境で正しく動作する保障はありません。      //
//                          http://www.charatsoft.com/                           //
///////////////////////////////////////////////////////////////////////////////////
#include <Windows.h>
#include <dsound.h>

///////////////////////////////////////////////////////////////////////////////////
// ライブラリの動作定義
//   使用するライブラリの場合は1を指定(要対応クラス)
///////////////////////////////////////////////////////////////////////////////////

// CAcmを使用するか(0の場合はCWavを使用)
#define USE_ACMLIB			0

// DirectShowによるストリーム再生を利用するか(要CMp3Playライブラリ)
#define USE_DSHOWSTREAM		0

// DirectShowによるスタティク再生を利用するか(メモリデコード版)
#define USE_DSHOWSTATIC		0

// エフェクトを利用するか(要DX9ランタイム＆FXライブラリ)
#define USE_EFFECT			0





///////////////////////////////////////////////////////////////////////////////////
// ライブラリインクルード
///////////////////////////////////////////////////////////////////////////////////
#if USE_DSHOWSTREAM
#include "CMp3Play.h"
#endif

#if USE_EFFECT
#include "CDirectSoundEffect.h"
#endif

#pragma comment(lib, "WinMM.lib")


class CDSSegPro81 {
protected:
	LPDIRECTSOUNDBUFFER		lpDSB;
	LPDIRECTSOUNDBUFFER8	lpDSB8;
	DSBCAPS					dsbcap;
	DWORD					dwBaseFreq;
	BOOL					bStream;
	int						iWavSize;

protected:
#if USE_DSHOWSTREAM
	CMp3Play				mp3;
#endif

#if USE_EFFECT
	CDirectSoundEffect		ef;
#endif

public:
	CDSSegPro81();
	virtual ~CDSSegPro81();
	BOOL Create( LPDIRECTSOUND8 lpDS,const char *file,BOOL hal=TRUE );																				// ファイルからサウンドバッファを生成
	BOOL CreateSoundBuffer( LPDIRECTSOUND8 lpDS,const LPVOID buf,int size,BOOL hal=TRUE );															// バイナリWAVデータからサウンドバッファを作成
	BOOL CreateSoundBuffer( LPDIRECTSOUND8 lpDS,const LPWAVEFORMATEX fmt,const LPVOID buf=NULL,int size=0,BOOL bPrimary=FALSE,BOOL hal=TRUE );		// サウンドバッファを指定サイズで生成
	BOOL Delete( void );																															// バッファの消去
	BOOL Play( BOOL bLoop=FALSE );																													// 再生
	BOOL Stop( void );																																// 停止
	BOOL Reset( void );																																// 巻き戻し
	DWORD GetBaseFrequency( void );																													// 基本周波数の取得
	DWORD GetFrequency( void );																														// 現在の周波数の取得
	BOOL SetFrequency( DWORD freq );																												// 周波数のセット
	BOOL SetVolume( float mul=1.0f );																												// 音量の変更(0～1)
	BOOL SetPan( float pan=0 );																														// パンの位置を変更(-1～+1)
	BOOL SetCurrentPosition( DWORD pos );																											// 再生カーソルのセット
	DWORD GetCurrentPosition( void );																												// 現在の再生カーソルを取得
	inline BOOL IsStream( void ) { return bStream; }																								// mp3などのストリームか
	inline int GetBufferSize( void ) { return iWavSize; }																							// サウンドバッファのサイズを返す

public:
#if USE_EFFECT
	BOOL AddEffect( void );
	BOOL DelEffect( void );
	inline CDirectSoundEffect *GetEffect( void ) { return &ef; }
#endif

public:
	static BOOL CreateWaveFormatEx( LPWAVEFORMATEX wf,int samp,int ch,int bit );			// WAVEFORMATEX構造体を作成
};



#endif
