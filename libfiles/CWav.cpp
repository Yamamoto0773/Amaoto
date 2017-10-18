#pragma warning( disable : 4996 )
#include "CWav.h"

//#define DEBUGMODE
#include "debug.h"

#define SAFE_FREE(x)		{ if(x) { free(x); x=NULL; } }

CWav::CWav()
{
	dwMode	= CWAVMODE_NO;
	ZeroMemory( &mHead,sizeof(mHead) );
	ZeroMemory( &mChunc,sizeof(mChunc) );
	ZeroMemory( &mData,sizeof(mData) );
	ZeroMemory( &mWaveFmt,sizeof(mWaveFmt) );
	fp		= NULL;

	pBuf	= NULL;
	iBuf	= 0;
	iAddr	= 0;
}

CWav::~CWav()
{
	Delete();
}

BOOL CWav::Create( const char *file,int samp,int bit,int ch )
{
	Delete();

	memcpy( &mHead.mRiff,"RIFF",4 );
	mHead.dwFileSize	= sizeof(CWAVHEADER) + sizeof(CWAVCHUNC) + sizeof(CWAVFMTCHUNC);
	memcpy( &mHead.mWave,"WAVE",4 );

	memcpy( &mChunc.mName,"fmt ",4 );
	mChunc.dwSize = sizeof(CWAVFMTCHUNC)-sizeof(CWAVCHUNC);		// fmtチャンクサイズ
	mChunc.wFormatID	= 1;
	mChunc.wChannel		= ch;
	mChunc.dwSample		= samp;
	mChunc.wBitrate		= bit;
	mChunc.dwBytePerSec	= mChunc.wChannel * mChunc.dwSample * mChunc.wBitrate / 8;
	mChunc.wBlockSize	= mChunc.wChannel * mChunc.wBitrate / 8;

	memcpy( &mData.mName,"data",4 );
	mData.dwSize		= 0;

	// WAVEFORMATEXの生成
	MakeWaveFormatEx( &mWaveFmt,mChunc.dwSample,mChunc.wBitrate,mChunc.wChannel );

	fp = fopen( file,"wb" );
	if( !fp ) {
		Close();
		return FALSE;
	}

	fwrite( &mHead,sizeof(mHead),1,fp );
	fwrite( &mChunc,sizeof(mChunc),1,fp );
	fwrite( &mData,sizeof(mData),1,fp );

	dwMode = CWAVMODE_WRITE_STREAM;
	return TRUE;
}

BOOL CWav::Create( const char *file,const LPWAVEFORMATEX fmt )
{
	return Create( file,fmt->nSamplesPerSec,fmt->wBitsPerSample,fmt->nChannels );
}

void CWav::Delete( void )
{
	Close();

	SAFE_FREE( pBuf );
	iBuf	= 0;
	iAddr	= 0;
}

BOOL CWav::Load( const char *file,DWORD mode )
{
	Close();

	fp = fopen( file,"rb" );
	if( !fp ) {
		Close();
		return FALSE;
	}

	fread( &mHead,sizeof(CWAVHEADER),1,fp );
	if( strnicmp(mHead.mRiff,"RIFF",4)!=0 ) {
		DEBUG( "RIFFフォーマットエラー\n" );
		Close();
		return FALSE;
	}

	if( strnicmp(mHead.mWave,"WAVE",4)!=0 ) {
		DEBUG( "WAVチャンクが見つからない\n" );
		Close();
		return FALSE;
	}

	// 次からアドレスが任意に変更する可能性がある
	long pos = ftell( fp );

	fread( &mChunc,sizeof(mChunc),1,fp );
	if( strnicmp(mChunc.mName,"fmt ",4)!=0 ) {
		DEBUG( "フォーマットチャンクが見つからない\n" );
		Close();
		return FALSE;
	}
	DEBUG( "チャンクサイズ     : %dbyte\n",mChunc.dwSize );
	DEBUG( "フォーマットID     : %d\n",mChunc.wFormatID );
	DEBUG( "チャンネル数       : %d\n",mChunc.wChannel );
	DEBUG( "ビット数           : %dbit\n",mChunc.wBitrate );
	DEBUG( "サンプリング周波数 : %dHz\n",mChunc.dwSample );

	// WAVEFORMATEXの生成
	MakeWaveFormatEx( &mWaveFmt,mChunc.dwSample,mChunc.wBitrate,mChunc.wChannel );

	// factチャンクのチェック
	pos += mChunc.dwSize + sizeof(CWAVCHUNC);
	fseek( fp,pos,SEEK_SET );

	// factチャンクの検索
	CWAVFACTCHUNK mChunk;
	ZeroMemory( &mChunk,sizeof(mChunk) );
	fread( &mChunk,sizeof(mChunk),1,fp );
	if( strnicmp(mChunk.mName,"fact",4)!=0 ) {
		// factチャンクが無ければ元に戻す
		DEBUG( "○factチャンクは存在しない\n" );
		mChunk.dwFactData = 0x00000000;
		fseek( fp,pos,SEEK_SET );
	} else {
		DEBUG( "factチャンク内容   = %08X\n",mChunk.dwFactData );
	}

	// dataチャンクの検索
	while(1) {
		if( fread(&mData,sizeof(mData),1,fp)<1 ) {
			DEBUG( "dataチャンクが見つからない\n" );
			Close();
			return FALSE;
		}
/*
#ifdef DEBUGMODE
		DEBUG( "ポインタ %08X\n",ftell(fp) );
		char chunk[8];
		ZeroMemory( chunk,sizeof(chunk) );
		memcpy( chunk,mData.mName,4 );
		DEBUG( "検索されたチャンク    [%s]\n",chunk );
#endif/**/

		// dataチャンクなら抜ける
		if( strnicmp(mData.mName,"data",4)==0 ) {
			break;
		}

		// 次のチャンクポインタへ移動
		fseek( fp,mData.dwSize,SEEK_CUR );
	}
	DEBUG( "データサイズ       = %dbyte\n",mData.dwSize );


	// モード記憶
	dwMode = mode;

	switch( dwMode )
	{
	case CWAVMODE_READ_STATIC:
		iBuf = mData.dwSize;
		pBuf = (LPBYTE)realloc( pBuf,iBuf );
		if( !pBuf ) {
			DEBUG( "メモリ確保エラー\n" );
			return FALSE;
		}
		fread( pBuf,iBuf,1,fp );
		break;
	case CWAVMODE_READ_STREAM:
		break;
	default:
		Close();
		return FALSE;
	}

	return TRUE;
}

BOOL CWav::Load( const LPVOID data,int size )
{
	Close();


	LPBYTE p = (LPBYTE)data;
	int addr = 0;

	// ヘッダ解析
	if( size<(int)(addr+sizeof(CWAVHEADER)) )
		return FALSE;
	memcpy( &mHead,&p[addr],sizeof(CWAVHEADER) );
	addr += sizeof(CWAVHEADER);

	if( strnicmp(mHead.mRiff,"RIFF",4)!=0 ) {
		DEBUG( "RIFFフォーマットエラー\n" );
		Close();
		return FALSE;
	}

	if( strnicmp(mHead.mWave,"WAVE",4)!=0 ) {
		DEBUG( "WAVチャンクが見つからない\n" );
		Close();
		return FALSE;
	}

	// 次からアドレスが任意に変更する可能性がある
	memcpy( &mChunc,&p[addr],sizeof(mChunc) );
	addr += sizeof(mChunc);
	if( strnicmp(mChunc.mName,"fmt ",4)!=0 ) {
		DEBUG( "フォーマットチャンクが見つからない\n" );
		Close();
		return FALSE;
	}

	DEBUG( "チャンクサイズ     : %dbyte\n",mChunc.dwSize );
	DEBUG( "フォーマットID     : %d\n",mChunc.wFormatID );
	DEBUG( "チャンネル数       : %d\n",mChunc.wChannel );
	DEBUG( "ビット数           : %dbit\n",mChunc.wBitrate );
	DEBUG( "サンプリング周波数 : %dHz\n",mChunc.dwSample );

	// WAVEFORMATEXの生成
	MakeWaveFormatEx( &mWaveFmt,mChunc.dwSample,mChunc.wBitrate,mChunc.wChannel );

	// dataチャンクの検索
	while( (int)(addr+sizeof(CWAVCHUNC))<size ) {
		memcpy( &mData,&p[addr],sizeof(CWAVCHUNC) );
		addr += sizeof(CWAVCHUNC);

		// dataチャンクなら抜ける
		if( strnicmp(mData.mName,"data",4)==0 ) {
			break;
		}

		// 次のチャンクポインタへ移動
		addr += mData.dwSize;
	}
	DEBUG( "データサイズ       = %dbyte\n",mData.dwSize );

	// データ分を足したサイズがオーバーするなら失敗
	if( (int)(addr+mData.dwSize)>size )
		return FALSE;

	// 常にSTATICモード
	dwMode = CWAVMODE_READ_STATIC;

	// メモリバッファ確保
	iBuf = mData.dwSize;
	pBuf = (LPBYTE)realloc( pBuf,iBuf );
	if( !pBuf ) {
		DEBUG( "メモリ確保エラー\n" );
		return FALSE;
	}

	// データ部をコピー
	memcpy( pBuf,&p[addr],mData.dwSize );

	return TRUE;
}

BOOL CWav::Close( void )
{
	if( fp ) {
		fclose( fp );
		fp = NULL;
//		DEBUG( "CLOSE OK\n" );
	}
	dwMode	= CWAVMODE_NO;
	return TRUE;
}

BOOL CWav::Write( const LPVOID buf,int size )
{
	// ストリーミングライトじゃなければ
	if( dwMode!=CWAVMODE_WRITE_STREAM )
		return FALSE;

	if( !fp )
		return FALSE;

	// データ書き込み先
	fwrite( buf,size,1,fp );
	long pos = ftell( fp );

	// ヘッダの更新
	mHead.dwFileSize += size;
	mData.dwSize += size;
	fseek( fp,0,SEEK_SET );
	fwrite( &mHead,sizeof(mHead),1,fp );
	fwrite( &mChunc,sizeof(mChunc),1,fp );
	fwrite( &mData,sizeof(mData),1,fp );

	fseek( fp,pos,SEEK_SET );

	return TRUE;
}

BOOL CWav::Read( LPVOID buf,int *size )
{
	if( dwMode!=CWAVMODE_READ_STREAM ) {
		DEBUG( "ストリームモードではない\n" );
		return FALSE;
	}

	// 最大値のチェック
	int read = *size;
	if( read+iAddr>(int)mData.dwSize )
		read = mData.dwSize - iAddr;

	*size = read;

	iAddr += read;

	if( read<1 ) {
		DEBUG( "ロード終了\n" );
		return FALSE;
	}

	fread( buf,read,1,fp );
	return TRUE;
}

BOOL CWav::MakeWaveFormatEx( LPWAVEFORMATEX fmt,int samp,int bit,int ch )
{
	ZeroMemory( fmt,sizeof(WAVEFORMATEX) );
	fmt->cbSize = sizeof(WAVEFORMATEX);
	fmt->wFormatTag = WAVE_FORMAT_PCM;
	fmt->nChannels = ch;
	fmt->nSamplesPerSec = samp;
	fmt->wBitsPerSample = bit;
	fmt->nBlockAlign = fmt->nChannels * fmt->wBitsPerSample / 8;
	fmt->nAvgBytesPerSec = fmt->nSamplesPerSec * fmt->nBlockAlign;
	return TRUE;
}