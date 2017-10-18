#include "CDSPro81.h"

#define DEBUGMODE
#include "DEBUG.H"


#define SAFE_RELEASE(x)		{ if(x) { x->Release(); x=NULL; } }
#define ONOFF(x)			(x?"YES":" NO")

////////////////////////////////////////////////////////////////////////////////////
// コンストラクタ
////////////////////////////////////////////////////////////////////////////////////
CDSPro81::CDSPro81()
{
	// 初期化
	ZeroMemory( &mDSCap,sizeof(DSCAPS) );
	mDSCap.dwSize	= sizeof(DSCAPS);

	hWnd			= NULL;
	pDS				= NULL;
	bStream			= FALSE;
}

////////////////////////////////////////////////////////////////////////////////////
// デストラクタ
////////////////////////////////////////////////////////////////////////////////////
CDSPro81::~CDSPro81()
{
	Delete();
}

////////////////////////////////////////////////////////////////////////////////////
// DirectSoundの作成
////////////////////////////////////////////////////////////////////////////////////
BOOL CDSPro81::Create( HWND hwnd,DWORD samp,DWORD bit,DWORD ch,DWORD dwLevel )
{
	Delete();

	hWnd = hwnd;

	// 生成
	if( FAILED(DirectSoundCreate8(NULL,&pDS,NULL)) ) {
		DEBUG( "サウンドオブジェクト作成失敗\n" );
		return FALSE;
	}
	// 強調モード
	if( FAILED(pDS->SetCooperativeLevel(hwnd,dwLevel)) ) {
		DEBUG( "強調レベル設定失敗\n" );
		return FALSE;
	}

	// 能力取得
	pDS->GetCaps(&mDSCap);
	DEBUG( "------------------------------------[ DirectSound Infomation ]--------------------------------------------\n" );
	DEBUG("(%s) : Microsoft により試験と保証が行われている。\n",ONOFF(mDSCap.dwFlags&DSCAPS_CERTIFIED) );
	DEBUG("(%s) : dwMinSecondarySampleRate と dwMaxSecondarySampleRate の各メンバ値の間のすべてのサンプリングレートをサポートする\n",ONOFF(mDSCap.dwFlags&DSCAPS_CONTINUOUSRATE ) );
	DEBUG("(%s) : DirectSound ドライバがインストールされている\n",ONOFF(~(mDSCap.dwFlags&DSCAPS_EMULDRIVER) ) );
	DEBUG("(%s) : 16 ビットサンプリングでプライマリサウンドバッファをサポートする\n",ONOFF(mDSCap.dwFlags&DSCAPS_PRIMARY16BIT ) );
	DEBUG("(%s) :  8 ビットサンプリングでプライマリサウンドバッファをサポートする\n",ONOFF(mDSCap.dwFlags&DSCAPS_PRIMARY8BIT ) );
	DEBUG("(%s) : 単音のプライマリバッファをサポートする。\n",ONOFF(mDSCap.dwFlags&DSCAPS_PRIMARYMONO ) );
	DEBUG("(%s) : ステレオのプライマリバッファをサポートする。\n",ONOFF(mDSCap.dwFlags&DSCAPS_PRIMARYSTEREO ) );
	DEBUG("(%s) : 16 ビットサンプリングでハードウェアミキシングのセカンダリサウンドバッファをサポートする\n",ONOFF(mDSCap.dwFlags&DSCAPS_SECONDARY16BIT ) );
	DEBUG("(%s) :  8 ビットサンプリングでハードウェアミキシングのセカンダリサウンドバッファをサポートする\n",ONOFF(mDSCap.dwFlags&DSCAPS_SECONDARY8BIT ) );
	DEBUG("(%s) : ハードウェアミキシングの単音セカンダリバッファをサポートする\n",ONOFF(mDSCap.dwFlags&DSCAPS_SECONDARYMONO ) );
	DEBUG("(%s) : ハードウェアミキシングのステレオセカンダリバッファをサポートする\n",ONOFF(mDSCap.dwFlags&DSCAPS_SECONDARYSTEREO ) );
	DEBUG("(%6d) : スタティックサウンドバッファ の最大数\n",mDSCap.dwMaxHwMixingStaticBuffers );
	DEBUG("(%6d) : ストリームサウンドバッファ の最大数\n",mDSCap.dwMaxHwMixingStreamingBuffers );
	DEBUG("(%6d) : スタティックサウンドバッファ を保持するサウンドカード上のメモリ容量のサイズ\n",mDSCap.dwTotalHwMemBytes );
	DEBUG("(%6d) : サウンドカード上の空きメモリのサイズ\n",mDSCap.dwFreeHwMemBytes );
	DEBUG("(%6d) : サウンドカード上の空きメモリのうち、最大の連続ブロックのサイズ\n",mDSCap.dwMaxContigFreeHwMemBytes );
	DEBUG("(%6d) : ハードウェアスタティックサウンドバッファへのデータ転送速度\n",mDSCap.dwUnlockTransferRateHwBuffers );
	DEBUG("(%6d) : ハードウェアセカンダリサウンドバッファがサポートする最小のサンプリングレート\n",mDSCap.dwMinSecondarySampleRate  );
	DEBUG("(%6d) : ハードウェアセカンダリサウンドバッファがサポートする最大のサンプリングレート\n",mDSCap.dwMaxSecondarySampleRate );
	DEBUG("(%6d) : ハードウェアでミキシングできるバッファの合計数\n",mDSCap.dwMaxHwMixingAllBuffers );
	DEBUG( "----------------------------------------------------------------------------------------------------------\n" );

	WAVEFORMATEX fmt;
	ZeroMemory( &fmt,sizeof(fmt) );
	fmt.wFormatTag		= WAVE_FORMAT_PCM;
	fmt.nChannels		= (WORD)ch;												
	fmt.nSamplesPerSec	= samp;									
	fmt.wBitsPerSample	= (WORD)bit;									
	fmt.nBlockAlign		= fmt.nChannels * ( fmt.wBitsPerSample/8 );	
	fmt.nAvgBytesPerSec	= fmt.nBlockAlign * fmt.nSamplesPerSec;

	return mPrimary.CreateSoundBuffer( pDS,&fmt,NULL,0,TRUE,TRUE );
}

////////////////////////////////////////////////////////////////////////////////////
// 明示的開放
////////////////////////////////////////////////////////////////////////////////////
BOOL CDSPro81::Delete( void )
{
	Clear();					// セカンダリバッファの全開放

	mPrimary.Delete();			// プライマリバッファの開放
	SAFE_RELEASE( pDS );		// DirectSound開放
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////
// ファイルの追加
////////////////////////////////////////////////////////////////////////////////////
BOOL CDSPro81::AddSound( int id,const char *file,BOOL hal )
{
	if( id<0 || id>CDSPRO_MAXSOUND-1 )
		return FALSE;

	if( !pDS )
		return FALSE;

	DEBUG( "Load [%d] %s\n",id,file );

	if( !mDSB[id].Create(pDS,file,hal) ) {
		DEBUG( "     NG\n" );
		return FALSE;
	}

	if( mDSB[id].IsStream() )
		bStream = TRUE;

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////
// WAVバイナリデータを追加(CAcm使用)
////////////////////////////////////////////////////////////////////////////////////
BOOL CDSPro81::AddSound( int id,const LPVOID buf,int size,BOOL hal )
{
	if( id<0 || id>CDSPRO_MAXSOUND-1 )
		return FALSE;

	return mDSB[id].CreateSoundBuffer( pDS,buf,size,hal );
}

////////////////////////////////////////////////////////////////////////////////////
// WAVバイナリデータを追加(直指定)
////////////////////////////////////////////////////////////////////////////////////
BOOL CDSPro81::AddSound( int id,const LPVOID buf,int size,DWORD samp,DWORD bit,DWORD ch,BOOL hal )
{
	if( id<0 || id>CDSPRO_MAXSOUND-1 )
		return FALSE;

	// 指定の周波数でバッファを作成
	WAVEFORMATEX fmt;
	CDSSegPro81::CreateWaveFormatEx( &fmt,samp,ch,bit );
	return mDSB[id].CreateSoundBuffer( pDS,&fmt,buf,size,FALSE,hal );
}

////////////////////////////////////////////////////////////////////////////////////
// ファイルの明示的開放
////////////////////////////////////////////////////////////////////////////////////
BOOL CDSPro81::DelSound( int id )
{
	if( id<0 || id>CDSPRO_MAXSOUND-1 )
		return FALSE;

	BOOL ret = mDSB[id].Delete();

	// ストリームが１つでも存在するか
	bStream = FALSE;
	for( int i=0;i<CDSPRO_MAXSOUND;i++ ) {
		if( mDSB[i].IsStream() ) {
			bStream = TRUE;
			break;
		}
	}

	return ret;
}

////////////////////////////////////////////////////////////////////////////////////
// ファイルの全開放
////////////////////////////////////////////////////////////////////////////////////
BOOL CDSPro81::Clear( void )
{
	int i;

	bStream = FALSE;

	if( !pDS )
		return FALSE;

	for( i=0;i<CDSPRO_MAXSOUND;i++ )
		mDSB[i].Delete();
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////
// 再生
////////////////////////////////////////////////////////////////////////////////////
BOOL CDSPro81::Play( int id,BOOL bLoop )
{
	if( !pDS )
		return FALSE;
	if( id<0 || id>CDSPRO_MAXSOUND-1 )
		return FALSE;
	return mDSB[id].Play( bLoop );
}

////////////////////////////////////////////////////////////////////////////////////
// 停止
////////////////////////////////////////////////////////////////////////////////////
BOOL CDSPro81::Stop( int id )
{
	if( !pDS )
		return FALSE;
	if( id<0 || id>CDSPRO_MAXSOUND-1 )
		return FALSE;
	return mDSB[id].Stop();
}

////////////////////////////////////////////////////////////////////////////////////
// 巻き戻し
////////////////////////////////////////////////////////////////////////////////////
BOOL CDSPro81::Reset( int id )
{
	if( !pDS )
		return FALSE;
	if( id<0 || id>CDSPRO_MAXSOUND-1 )
		return FALSE;
	return mDSB[id].Reset();
}

////////////////////////////////////////////////////////////////////////////////////
// 周波数の変更
////////////////////////////////////////////////////////////////////////////////////
BOOL CDSPro81::SetFrequency( int id,DWORD freq )
{
	if( !pDS )
		return FALSE;
	if( id<0 || id>CDSPRO_MAXSOUND-1 )
		return FALSE;
	return mDSB[id].SetFrequency( freq );
}

////////////////////////////////////////////////////////////////////////////////////
// 周波数の変更(倍率指定)
////////////////////////////////////////////////////////////////////////////////////
BOOL CDSPro81::SetFrequency( int id,float mul )
{
	if( !pDS )
		return FALSE;
	if( id<0 || id>CDSPRO_MAXSOUND-1 )
		return FALSE;

	float freq = (float)mDSB[id].GetBaseFrequency() * mul;
	if( freq<100 )
		return FALSE;

	return mDSB[id].SetFrequency( (DWORD)freq );
}

////////////////////////////////////////////////////////////////////////////////////
// 基本の周波数を返す
////////////////////////////////////////////////////////////////////////////////////
DWORD CDSPro81::GetBaseFrequency( int id )
{
	if( !pDS )
		return FALSE;
	if( id<0 || id>CDSPRO_MAXSOUND-1 )
		return FALSE;
	return mDSB[id].GetBaseFrequency();
}

////////////////////////////////////////////////////////////////////////////////////
// 基本の周波数を返す(音量の変更)
////////////////////////////////////////////////////////////////////////////////////
BOOL CDSPro81::SetVolume( int id,float mul )
{
	if( !pDS )
		return FALSE;
	if( id<0 || id>CDSPRO_MAXSOUND-1 )
		return FALSE;
	return mDSB[id].SetVolume( mul );
}

////////////////////////////////////////////////////////////////////////////////////
// パンの位置を変更
////////////////////////////////////////////////////////////////////////////////////
BOOL CDSPro81::SetPan( int id,float pan )
{
	if( !pDS )
		return FALSE;
	if( id<0 || id>CDSPRO_MAXSOUND-1 )
		return FALSE;
	return mDSB[id].SetPan( pan );
}
