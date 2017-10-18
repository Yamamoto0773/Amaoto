#ifndef _CWAV_H
#define _CWAV_H
///////////////////////////////////////////////////////////////////////////////////
// CWav : 標準WAVEファイル管理クラス v1.09                                       //
//                                                                               //
// このソースコードは自由に改変して使用可能です。                                //
// また商用利用も可能ですが、すべての環境で正しく動作する保障はありません。      //
//                          http://www.charatsoft.com/                           //
///////////////////////////////////////////////////////////////////////////////////
#include <Windows.h>
#include <stdio.h>

// モード
#define CWAVMODE_NO				0x00000000		// 未定義
#define CWAVMODE_READ_STATIC	0x00000001		// Openにて全データをロード済み
#define CWAVMODE_READ_STREAM	0x00000002		// Openにてデータをストリーミングロード
#define CWAVMODE_WRITE_STREAM	0x00000003		// Createにてデータをストリーミングライト


// 1バイト境界とする
#pragma pack(push)
#pragma pack(1)

typedef struct _CWAVHEADER {
	char	mRiff[4];							// 'RIFF'
	DWORD	dwFileSize;							// ファイルサイズ
	char	mWave[4];							// 'WAVE'
} CWAVHEADER,*LPCWAVHEADER;

typedef struct _CWAVCHUNC {
	char	mName[4];							// '****'チャンク名
	DWORD	dwSize;								// チャンクサイズ
} CWAVCHUNC,*LPCWAVCHUNC;

typedef struct _CWAVFMTCHUNC : CWAVCHUNC {
	WORD	wFormatID;							// フォーマットＩＤ(PCMなら1)
	WORD	wChannel;							// チャンネル数
	DWORD	dwSample;							// サンプリングレート
	DWORD	dwBytePerSec;						// 秒間のバイト数(wChannel*dwSample*wBitrate )
	WORD	wBlockSize;							// １つのWAVデータサイズ(wChannel*wBitrate)
	WORD	wBitrate;							// サンプリングビット値(8bit/16bit)
} CWAVFMTCHUNC,*LPCWAVFMTCHUNC;

typedef struct _CWAVFACT : CWAVCHUNC {
	DWORD	dwFactData;							// factチャンクに含まれる情報
} CWAVFACTCHUNK,*LPCWAVFACTCHUNK;

#pragma pack(pop)



class CWav {
	DWORD			dwMode;		// ストリームか
	CWAVHEADER		mHead;		// WAVヘッダ
	CWAVFMTCHUNC	mChunc;		// fmtチャンク情報
	CWAVCHUNC		mData;		// DATAチャンク情報
	WAVEFORMATEX	mWaveFmt;	// WAVEFORMATEX構造体
	FILE			*fp;

	LPBYTE			pBuf;		// データ部
	int				iBuf;		// データ数
	int				iAddr;		// ストリーム時のデータポインタ

public:
	CWav();
	virtual ~CWav();
	BOOL Create( const char *file,int samp=44100,int bit=16,int ch=2 );			// WAVデータの新規作成
	BOOL Create( const char *file,const LPWAVEFORMATEX fmt );					// WAVデータの新規作成(fmtのsamp,bit,chを正しくセットすること)
	void Delete( void );														// 終了処理
	BOOL Load( const char *file,DWORD mode=CWAVMODE_READ_STATIC );				// ファイルのロード(modeによりストリームロード可)
	BOOL Load( const LPVOID data,int size );									// メモリバッファからロード(常にCWAVMODE_READ_STATICとなる)
	BOOL Close( void );															// ストリームモードでファイルを閉じる
	BOOL Write( const LPVOID buf,int size );									// ストリームライト時に指定のバッファを書き込む
	BOOL Read( LPVOID buf,int *size );											// ストリームリード時に指定のバッファ分をロード(sizeには取得したサイズを入れて呼び出すことで実際にロードしたサイズを返す)
public:
	static BOOL MakeWaveFormatEx( LPWAVEFORMATEX fmt,int samp,int bit,int ch );
public:
	inline const LPBYTE GetBuffer( void ) { return pBuf; }
	inline int GetBufferSize( void ) { return iBuf; }
	inline const LPWAVEFORMATEX GetWaveFormatEx( void ) { return &mWaveFmt; }
};

#endif
