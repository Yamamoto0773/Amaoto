#pragma warning( disable : 4996 )
#include "CDSSegPro81.h"

#if USE_ACMLIB
#include "CAcm.h"			// CAcmクラス
#else
#include "CWav.h"			// CWavクラス
#endif

#if USE_DSHOWSTATIC
	#include "CDShowGraph.h"
	#include "CDSWavDecoder.h"
	#ifdef _DEBUG
	#pragma comment(lib, "dslib/STRMBASD.LIB")
	#else
	#pragma comment(lib, "dslib/STRMBASE.LIB")
	#endif
#endif

//#define DEBUGMODE
#include "DEBUG.H"

#define SAFE_RELEASE(x)		{ if(x) { x->Release(); x=NULL; } }

//////////////////////////////////////////////////////////////
// コンストラクタ
//////////////////////////////////////////////////////////////
CDSSegPro81::CDSSegPro81()
{
	lpDSB			= NULL;
	lpDSB8			= NULL;
	ZeroMemory( &dsbcap,sizeof(dsbcap) );
	dsbcap.dwSize	= sizeof(dsbcap);
	dwBaseFreq		= 0;
	bStream			= FALSE;
	iWavSize		= 0;
}

//////////////////////////////////////////////////////////////
// デストラクタ
//////////////////////////////////////////////////////////////
CDSSegPro81::~CDSSegPro81()
{
	Delete();
}

//////////////////////////////////////////////////////////////
// ファイルからサウンドバッファを生成
//////////////////////////////////////////////////////////////
BOOL CDSSegPro81::Create( LPDIRECTSOUND8 lpDS,const char *file,BOOL hal )
{
	// 既に存在する場合は消去
	Delete();

	//DEBUG( "LOAD [%s]\n",file );

	if( strnicmp(&file[strlen(file)-4],".WAV",4)!=0 ) {
#if USE_DSHOWSTREAM

		if( !mp3.Init(file,TRUE) )
			return FALSE;
		bStream = TRUE;

#elif USE_DSHOWSTATIC
		HRESULT ret;
		WCHAR file2[MAX_PATH];
		MultiByteToWideChar( CP_ACP,0,file,-1,file2,MAX_PATH-1 );
		CDShowGraph ds;
		ds.Create();
		ds.DisableClock();
		if( !ds.SetSourceFile( file2 ) ) {
			return FALSE;
		}

		CDSWavDecoder *wd = new CDSWavDecoder( &ret );
		ds.SetRenderFilter( wd );
		if( !ds.Connect() ) {
			DEBUG( "フィルタ接続失敗\n" );
			return FALSE;
		}

		// デコード開始
		ds.Play();

		// 終了するまで待機
		if( !ds.WaitForCompletion() ) {
			DEBUG( "デコードエラー\n" );
			ds.Delete();
			return FALSE;
		}

//		wd->SavePCM(L"WAVDEC.PCM");

		// サウンドバッファの生成
		if( !CreateSoundBuffer(lpDS,wd->GetWaveFormat(),wd->GetData(),wd->GetDataSize(),FALSE,hal) ) {
			DEBUG( "サウンドバッファ生成エラー\n" );
			ds.Delete();
			return FALSE;
		}

		ds.Delete();
#else
		return FALSE;
#endif
	} else {

		// スタティックバッファ
#if USE_ACMLIB
		CAcm acm;
		if( !acm.Load(file) ) {
			DEBUG( "ACMロードエラー [%s]\n",file );
			return FALSE;
		}

		// サウンドバッファの生成
		if( !CreateSoundBuffer(lpDS,acm.GetWaveFormatEx(),acm.GetBuffer(),acm.GetBufferSize(),FALSE,hal) ) {
			DEBUG( "サウンドバッファ生成エラー\n" );
			return FALSE;
		}
		acm.Flash();
#else
		CWav wav;
		if( !wav.Load(file,CWAVMODE_READ_STATIC) ) {
			DEBUG( "WAVロードエラー [%s]\n",file );
			return FALSE;
		}
		// サウンドバッファの生成
		if( !CreateSoundBuffer(lpDS,wav.GetWaveFormatEx(),wav.GetBuffer(),wav.GetBufferSize(),FALSE,hal) ) {
			DEBUG( "サウンドバッファ生成エラー\n" );
			return FALSE;
		}
		wav.Close();
#endif
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////
// バイナリWAVデータからサウンドバッファを作成
//////////////////////////////////////////////////////////////
BOOL CDSSegPro81::CreateSoundBuffer( LPDIRECTSOUND8 lpDS,const LPVOID buf,int size,BOOL hal )
{
	Delete();

	// ACM経由でファイルをロード
#if USE_ACMLIB
	CAcm acm;
	if( !acm.Load(buf,size) ) {
		DEBUG( "ACMバイナリロードエラー\n" );
		return FALSE;
	}

	// サウンドバッファの作成
	if( !CreateSoundBuffer(lpDS,acm.GetWaveFormatEx(),acm.GetBuffer(),acm.GetBufferSize(),FALSE,hal) )
		return FALSE;
#else
	// ACM以外はCWav経由でロード
	CWav wav;
	if( !wav.Load(buf,size) ) {
		DEBUG( "WAVロードエラー [%s]\n",buf );
		return FALSE;
	}
	// サウンドバッファの生成
	if( !CreateSoundBuffer(lpDS,wav.GetWaveFormatEx(),wav.GetBuffer(),wav.GetBufferSize(),FALSE,hal) ) {
		DEBUG( "サウンドバッファ生成エラー\n" );
		return FALSE;
	}
#endif

	return TRUE;
}

//////////////////////////////////////////////////////////////
// サウンドバッファを指定サイズで生成
//////////////////////////////////////////////////////////////
BOOL CDSSegPro81::CreateSoundBuffer( LPDIRECTSOUND8 lpDS,const LPWAVEFORMATEX fmt,const LPVOID buf,int size,BOOL bPrimary,BOOL hal )
{
	HRESULT ret;
	LPBYTE p = (LPBYTE)buf;

	// すでに存在する場合エラー
	if( lpDSB8 )
		return FALSE;

	// バッファをつくる
	DSBUFFERDESC dsdesc;
	ZeroMemory(&dsdesc,sizeof(DSBUFFERDESC));
	dsdesc.dwSize = sizeof(DSBUFFERDESC);

	DEBUG( "チャンネル数      [%d]\n",fmt->nChannels );
	DEBUG( "サンプリングレート[%d]\n",fmt->nSamplesPerSec );
	DEBUG( "ビットレート      [%d]\n",fmt->wBitsPerSample );
	DEBUG( "バッファサイズ    [%d]\n",size );/**/


	if( bPrimary ) {
		DEBUG( "プライマリバッファの作成\n" );
		dsdesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D;
		dsdesc.dwBufferBytes = 0;
		dsdesc.lpwfxFormat = NULL;

		if( FAILED(lpDS->CreateSoundBuffer(&dsdesc,&lpDSB,NULL)) ) {
			DEBUG("バッファ作成失敗\n");
			return FALSE;
		}

		if( FAILED(lpDSB->SetFormat(fmt)) ) {
			DEBUG( "サウンドフォーマットの指定に失敗\n" );
			return FALSE;
		}

		DEBUG( "プライマリバッファ作成OK\n" );

	} else {

		DEBUG( "スタティックバッファの作成\n" );
		if( fmt->nChannels>2 ) {
			// マルチチャンネルの場合は特殊操作は不可能
			DEBUG( "マルチチャンネル\n" );
			dsdesc.dwFlags = DSBCAPS_STATIC | DSBCAPS_LOCDEFER | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_GLOBALFOCUS;
		} else {
#if USE_EFFECT
			dsdesc.dwFlags = DSBCAPS_LOCSOFTWARE | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLFREQUENCY | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_CTRLFX;
#else
			dsdesc.dwFlags = DSBCAPS_LOCSOFTWARE | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLFREQUENCY | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN;
#endif
		}

		fmt->wFormatTag			= WAVE_FORMAT_PCM;
		dsdesc.dwBufferBytes	= size;
		dsdesc.lpwfxFormat		= fmt;
		dsdesc.guid3DAlgorithm	= GUID_NULL;

		float m_sec = (float)size / (float)(fmt->nSamplesPerSec * fmt->wBitsPerSample/8 * fmt->nChannels ) * 1000;
		DEBUG( "データ時間        [%f]ms\n",m_sec );
		if( m_sec<DSBSIZE_FX_MIN ) {
			DEBUG( "%dミリ秒以下\n",DSBSIZE_FX_MIN );
		}

		// バッファの確保先
/*		if( hal ) {
			dsdesc.dwFlags |= DSBCAPS_LOCDEFER;
		} else {
			dsdesc.dwFlags |= DSBCAPS_LOCSOFTWARE;
		}/**/

		ret = lpDS->CreateSoundBuffer( &dsdesc,&lpDSB,NULL );
		if( FAILED(ret) ) {
			DEBUG( "バッファ作成失敗1 [%08X]\n",ret );
			if( ret!=DSERR_BUFFERTOOSMALL )
				return FALSE;
			// サイズを変更して実験(64K)
			size = 0x10000;
			dsdesc.dwBufferBytes = size;
			HRESULT ret = lpDS->CreateSoundBuffer( &dsdesc,&lpDSB,NULL );
			if( FAILED(ret) ) {
				DEBUG("バッファ作成失敗2 [%08X]\n",ret );
				if( ret!=DSERR_BUFFERTOOSMALL )
					return FALSE;
				// サイズを変更して実験2(128K)
				size = 0x20000;
				dsdesc.dwBufferBytes = size;
				ret = lpDS->CreateSoundBuffer( &dsdesc,&lpDSB,NULL );	// バッファサイズ4倍でもだめか
				if( FAILED(ret) ) {
					DEBUG("バッファ作成失敗 [%08X]\n",ret );
					return FALSE;
				}
			}
			DEBUG( "サイズ変更で作成\n" );
		}

		// WAVサイズを保存
		iWavSize = size;

		// ベース周波数の取得
		dwBaseFreq = GetFrequency();
		if( dwBaseFreq==0 ) {
			DEBUG( "ベース周波数の取得に失敗\n" );
		}

		LPVOID pMem1,pMem2;
		DWORD dwSize1,dwSize2;
		if( FAILED(lpDSB->Lock(0,size,&pMem1,&dwSize1,&pMem2,&dwSize2,0)) ) {
			DEBUG( "ロック失敗\n" );
			return FALSE;
		}

		// バッファのクリア
		ZeroMemory( pMem1,dwSize1 );		// 第１ブロック
		ZeroMemory( pMem2,dwSize2 );		// 第２ブロック

		// データが指定されている場合はバッファにコピー
		if( p ) {
			memcpy( pMem1,p,dwSize1 );
			memcpy( pMem2,&p[dwSize1],dwSize2 );
		}

		lpDSB->Unlock( pMem1,dwSize1,pMem2,dwSize2 );

		// DirectSoundBuffer8を取得
		ret = lpDSB->QueryInterface( IID_IDirectSoundBuffer8,(LPVOID*)&lpDSB8 );

#if USE_EFFECT
		if( SUCCEEDED(ret) ) {
			if( fmt->nChannels<=2 ) {
				if( !ef.Create(lpDSB8) ) {
					DEBUG( "エフェクトエラー\n" );
				}
			}
		}
#endif

		DEBUG( "スタティックバッファ作成OK\n" );

	}



	// サウンドバッファの情報を取得
/*	lpDSB->GetCaps( &dsbcap );
	DEBUG("バッファサイズ = %d\n",dsbcap.dwBufferBytes );
	DEBUG("オンボードハードウェアに%s\n",(dsbcap.dwFlags&DSBCAPS_STATIC)?"ある":"ない" );
	DEBUG("%sバッファ\n\n",(dsbcap.dwFlags&DSBCAPS_PRIMARYBUFFER )?"プライマリ":"セカンダリ" );
	DEBUG( "ベース周波数 %d\n",dwBaseFreq );
/**/
	return TRUE;
}

//////////////////////////////////////////////////////////////
// バッファの消去
//////////////////////////////////////////////////////////////
BOOL CDSSegPro81::Delete( void )
{
	Stop();		// 強制停止

#if USE_DSHOWSTREAM
	mp3.Exit();
	bStream = FALSE;
#endif

#if USE_EFFECT
	ef.Delete();
#endif

	SAFE_RELEASE( lpDSB8 );
	SAFE_RELEASE( lpDSB );

	ZeroMemory( &dsbcap,sizeof(dsbcap) );
	dsbcap.dwSize	= sizeof(dsbcap);
	dwBaseFreq		= 0;
	iWavSize		= 0;

	return TRUE;
}


//////////////////////////////////////////////////////////////
// 再生
//////////////////////////////////////////////////////////////
BOOL CDSSegPro81::Play( BOOL bLoop )
{
#if USE_DSHOWSTREAM
	if( bStream ) {
		return mp3.Play( bLoop );
	}
#endif

	if( !lpDSB )
		return FALSE;

	DWORD flag = 0x00000000;

	if( bLoop || (dsbcap.dwFlags&DSBCAPS_PRIMARYBUFFER) )
		flag |= DSBPLAY_LOOPING;

	if( FAILED(lpDSB->Play( 0,0,flag )) )
		return FALSE;

	return TRUE;
}

//////////////////////////////////////////////////////////////
// 停止
//////////////////////////////////////////////////////////////
BOOL CDSSegPro81::Stop( void )
{
#if USE_DSHOWSTREAM
	if( bStream ) {
		return mp3.Stop();
	}
#endif

	if( !lpDSB )
		return FALSE;

	if( dsbcap.dwFlags&DSBCAPS_PRIMARYBUFFER )		// プライマリバッファは無視
		return FALSE;

	if( FAILED(lpDSB->Stop()) )
		return FALSE;

	return TRUE;
}

//////////////////////////////////////////////////////////////
// 巻き戻し
//////////////////////////////////////////////////////////////
BOOL CDSSegPro81::Reset( void )
{
#if USE_DSHOWSTREAM
	if( bStream ) {
		mp3.Pause();
		return mp3.Seek();
	}
#endif

	if( !lpDSB )
		return FALSE;


	if( dsbcap.dwFlags&DSBCAPS_PRIMARYBUFFER )		// プライマリバッファは無視
		return FALSE;

	if( FAILED(lpDSB->Stop()) )
		return FALSE;

	if( FAILED(lpDSB->SetCurrentPosition(0)) )
		return FALSE;

	return TRUE;
}

DWORD CDSSegPro81::GetBaseFrequency( void )
{
	return dwBaseFreq;
}

DWORD CDSSegPro81::GetFrequency( void )
{
	if( !lpDSB )
		return FALSE;

	DWORD freq = 0;
	lpDSB->GetFrequency( &freq );
	return freq;
}

BOOL CDSSegPro81::SetFrequency( DWORD freq )
{
	if( !lpDSB )
		return FALSE;

	if( dwBaseFreq==0 )
		return FALSE;

	if( FAILED(lpDSB->SetFrequency(freq)) )
		return FALSE;

	return TRUE;
}

BOOL CDSSegPro81::SetVolume( float mul )
{
	if( !lpDSB )
		return FALSE;

	long v = (long)( (5000.0f * mul) - 5000.0f );
	if( v<=-5000 )
		v = DSBVOLUME_MIN;
	if( FAILED(lpDSB->SetVolume(v)) )
		return FALSE;

	return TRUE;
}

BOOL CDSSegPro81::SetPan( float pan )
{
	if( !lpDSB )
		return FALSE;

	float pan2 = pan / 2 + 0.5f;
	float pan3 = (float)(DSBPAN_RIGHT-DSBPAN_LEFT) * pan2 + DSBPAN_LEFT;
	if( FAILED(lpDSB->SetPan((LONG)pan3)) )
		return FALSE;

	return TRUE;
}

BOOL CDSSegPro81::SetCurrentPosition( DWORD pos )
{
	if( !lpDSB )
		return FALSE;
	if( FAILED(lpDSB->SetCurrentPosition(pos)) )
		return FALSE;

	return TRUE;
}

DWORD CDSSegPro81::GetCurrentPosition( void )
{
	if( !lpDSB )
		return 0;

	DWORD dwPlay = 0;
	DWORD dwWrite = 0;
	lpDSB->GetCurrentPosition( &dwPlay,&dwWrite );

	return dwPlay;
}

#if USE_EFFECT
BOOL CDSSegPro81::AddEffect( void )
{
	if( !lpDSB8 )
		return FALSE;
	lpDSB8->Stop();
	ef.Create( lpDSB8 );
	return ef.SetNoEffect();
}

BOOL CDSSegPro81::DelEffect( void )
{
	return ef.Delete();
}
#endif

// WAVEFORMATEXの作成
BOOL CDSSegPro81::CreateWaveFormatEx( LPWAVEFORMATEX wf,int samp,int ch,int bit )
{
	ZeroMemory( wf,sizeof(WAVEFORMATEX) );
	wf->cbSize = sizeof(WAVEFORMATEX);
	wf->wFormatTag = WAVE_FORMAT_PCM;
	wf->nChannels = ch;
	wf->nSamplesPerSec = samp;
	wf->wBitsPerSample = bit;
	wf->nBlockAlign = wf->nChannels * wf->wBitsPerSample / 8;
	wf->nAvgBytesPerSec = wf->nSamplesPerSec * wf->nBlockAlign;
	return TRUE;
}
