#ifndef _CDSPRO81_H
#define _CDSPRO81_H
///////////////////////////////////////////////////////////////////////////////////
// CDSPro81 : DirectSound管理クラス for おとゲマ v2.01                           //
//                                                                               //
// このソースコードは自由に改変して使用可能です。                                //
// また商用利用も可能ですが、すべての環境で正しく動作する保障はありません。      //
//                          http://www.charatsoft.com/                           //
///////////////////////////////////////////////////////////////////////////////////
#include "CDSSegPro81.h"

////////////////////////////////////////////////////////////////////////////////////
// 定義
////////////////////////////////////////////////////////////////////////////////////
#define CDSPRO_MAXSOUND			2048		// AddSound可能なサウンド数

////////////////////////////////////////////////////////////////////////////////////
// DirectSoundライブラリ
////////////////////////////////////////////////////////////////////////////////////
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")

////////////////////////////////////////////////////////////////////////////////////
// クラス定義
////////////////////////////////////////////////////////////////////////////////////
class CDSPro81 {
protected:
	HWND			hWnd;					// ウインドウのハンドル
	LPDIRECTSOUND8	pDS;					// DirectSound8オブジェクト
	DSCAPS			mDSCap;					// ハードウェア能力
	CDSSegPro81		mPrimary;				// プライマリバッファ
	CDSSegPro81		mDSB[CDSPRO_MAXSOUND];	// セカンダリサウンドバッファ
	BOOL			bStream;				// １つでもストリームが存在するか
public:
	CDSPro81();
	virtual ~CDSPro81();
	BOOL Create( HWND hwnd,DWORD samp=44100,DWORD bit=16,DWORD ch=2,DWORD dwLevel=DSSCL_PRIORITY );				// DirectSound8の生成
	BOOL Delete( void );																						// 明示的開放
	BOOL AddSound( int id,const char *file,BOOL hal=TRUE );														// ファイルの追加
	BOOL AddSound( int id,const LPVOID buf,int size,BOOL hal=TRUE );											// WAVバイナリデータを追加(ファイルイメージ用)
	BOOL AddSound( int id,const LPVOID buf,int size,DWORD samp=44100,DWORD bit=16,DWORD ch=2,BOOL hal=TRUE );	// WAVバイナリデータを追加(RAWデータ用)
	BOOL DelSound( int id );																					// ファイルの明示的開放
	BOOL Clear( void );																							// ファイルの全開放
	BOOL Play( int id,BOOL bLoop=FALSE );																		// 再生
	BOOL Stop( int id );																						// 停止
	BOOL Reset( int id );																						// 巻き戻し
	BOOL SetFrequency( int id,DWORD freq );																		// 周波数の変更
	BOOL SetFrequency( int id,float mul=1.0f );																	// 周波数の変更(倍率指定)
	BOOL SetVolume( int id,float mul=1.0f );																	// 音量の変更(0～1)
	BOOL SetPan( int id,float pan=0 );																			// パンの位置を変更(-1～+1)
	DWORD GetBaseFrequency( int id );																			// 基本の周波数を返す
public:
	inline BOOL IsStream( void ) { return bStream; }															// １つでもストリーム再生しているか
	inline int GetBufferSize( int id ) { return mDSB[id].GetBufferSize(); }										// 指定のサウンドのWAVサイズを返す
	inline CDSSegPro81 *GetSegment( int id ) { return &mDSB[id]; }												// サウンドセグメントクラスを返す
	inline LPDIRECTSOUND8 GetDirectSound( void ) { return pDS; }												// DirectSoundオブジェクトを返す
};

typedef CDSPro81 CDSPro;

#endif
