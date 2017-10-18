/*
! 参考ソースコード
!
! @file movietex.h
! @bref	ムービーテクスチャクラス
! @author	Masafumi TAKAHASHI
*/

#ifndef MOVIETEX_H
#define MOVIETEX_H

#include <d3d9.h>
#include <d3dx9.h>

#include <atlbase.h>
#include <stdio.h>
#include <streams.h>

#include <d3d9types.h>
#include <dshow.h>

//-----------------------------------------------------------------------------
// Define GUID for Texture Renderer
// {71771540-2017-11cf-AE26-0020AFD79767}
//-----------------------------------------------------------------------------
struct __declspec(uuid("{71771540-2017-11cf-ae26-0020afd79767}")) CLSID_TextureRenderer;

//! @class CMovieTexture 
//! @brief ビデオをDirect3DTexture9に書き込む処理を行うクラス
class CMovieTexture : public CBaseVideoRenderer {
	CComPtr<IDirect3DDevice9>	m_pd3dDevice;
	CComPtr<IDirect3DTexture9>	m_pTexture;

	D3DFORMAT		m_TextureFormat;

public:
	CMovieTexture(LPUNKNOWN pUnk, HRESULT *phr);
	~CMovieTexture();

	VOID SetDevice(IDirect3DDevice9 * pd3dDevice) { m_pd3dDevice = pd3dDevice; };

	HRESULT CheckMediaType(const CMediaType *pmt);     // Format acceptable?
	HRESULT SetMediaType(const CMediaType *pmt);       // Video format notification
	HRESULT DoRenderSample(IMediaSample *pMediaSample); // New video sample

														//! @fn IDirect3DTexture9 * GetTexture()
														//! @brief ムービーテクスチャの取得
														//! @return ムービーがレンダリングされたテクスチャ
	IDirect3DTexture9 * GetTexture() { return m_pTexture; };

	//! @fn VOID GetVideoDesc(LONG* plVidWidth, LONG* plVidHeight, LONG* plVidPitch)
	//! @brief ビデオの幅、高さ、ピッチの取得
	//! @param *plVidWidth (out) ビデオの幅
	//! @param *plVidHeigh (out) ビデオの高
	//! @param *plVidPitch (out) ビデオのピッチ
	VOID GetVideoDesc(LONG* plVidWidth, LONG* plVidHeight, LONG* plVidPitch) {
		*plVidWidth = m_lVidWidth;
		*plVidHeight = m_lVidHeight;
		*plVidPitch = m_lVidPitch;
	};

	//! @param ダイナミックテクスチャを使うかどうかどうかのフラグ
	BOOL m_bUseDynamicTextures;
	//! @param ビデオの幅
	LONG m_lVidWidth;
	//! @param ビデオの高さ
	LONG m_lVidHeight;
	//! @param ビデオのピッチ
	LONG m_lVidPitch;
};







#define D3DFVF_MOVIEVERTEX (D3DFVF_XYZ | D3DFVF_TEX1)

typedef struct _MOVIEVERTEX {
	float x, y, z;
	float u, v;
} MOVIEVERTEX;


// 動画の位置を指定するときに使うフラグ
typedef enum _MOVIEPOSITIONFORMAT {
	MOVPOSFMT_CENTER,		// 動画の中央の座標を指定
	MOVPOSFMT_UPPERRIGHT,	// 動画の右上の座標を指定
}MOVIEPOSITIONFORMAT;


// 動画の拡大縮小処理を指定するときに使うフラグ
typedef enum _DRAWMOVIEOPTION {
	DRAWMOVOPT_ASPECTVARI,	// 短形領域に一致させるように縦横比を無視して動画を拡大縮小
	DRAWMOVOPT_ASPECTFIXED,	// 短形領域に一致させるように縦横比を維持して動画を拡大縮小
	DRAWMOVOPT_ZOOMDISABLED,// 短形領域に収まるように動画を縮小　※拡大はされません
}DRAWMOVIEOPTION;


//! @class CMovieToTexture
//! @brief ムービーテクスチャクラス
class DirectXMovie {
	CComPtr<IDirect3DDevice9>       m_pD3DDevice;   // Our rendering device
	CComPtr<IDirect3DTexture9>      m_pTexture;     // Our texture
	CComPtr<IDirect3DVertexBuffer9> m_pD3DVertex;

	CComPtr<IGraphBuilder>  m_pGB;          // GraphBuilder
	CComPtr<IMediaControl>  m_pMC;          // Media Control
	CComPtr<IMediaPosition> m_pMP;          // Media Position
	CComPtr<IMediaEvent>    m_pME;          // Media Event
	CComPtr<IBaseFilter>    m_pRenderer;    // our custom renderer

	int m_iDrawWidth;
	int m_iDrawHeight;

	//! @param 幅  
	LONG m_lWidth;
	//! @param 高さ
	LONG m_lHeight;
	//! @param ピッチ
	LONG m_lPitch;

	//! @param 元のムービーのサイズ(幅、高さ) / テクスチャのサイズ(幅、高さ)で算出するUV値
	FLOAT m_fu, m_fv;

	void CheckMovieStatus(void);
	

public:
	DirectXMovie();	//コンストラクタ
	~DirectXMovie();	//デストラクタ

	VOID Clear(void);

	//! @fn HRESULT InitDShowTextureRenderer(WCHAR* wFileName, const BOOL bSound)
	//! @brief DirectShowからテクスチャへのレンダリングへの初期化
	//! @param *lpD3DDevice (in) Direct3Dデバイス
	//! @param *wFileName (in) ムービーファイルのパス
	//! @param bSound (in) サウンド再生フラグ
	//! @param iDrawWidth (in) iDrawWidth 動画を再生するウィンドウの横幅
	//! @param iDrawHeight (in) iDrawHeight 動画を再生するウィンドウの縦幅
	//! @return 関数の成否
	HRESULT Create(IDirect3DDevice9 *lpD3DDevice, WCHAR* wFileName, const BOOL bSound, int iDrawWidth, int iDrawHeight);

	//! @fn IDirect3DTexture9 * GetTexture()
	//! @brief ムービーテクスチャの取得
	//! @return ムービーがレンダリングされたテクスチャ
	HRESULT GetTexture(IDirect3DTexture9** pTexture) { return m_pTexture.CopyTo(pTexture); };


	// 動画をテクスチャとして指定領域内に描画
	// 引数 (in) drawArea 動画を描画する領域(スクリーン座標)
	// 戻り値 関数の成否
	HRESULT DrawMovie(POINT &drawPoint, MOVIEPOSITIONFORMAT format);
	HRESULT DrawMovie(RECT &drawArea, DRAWMOVIEOPTION option);
	

	//! @fn VOID Play()
	//! @brief ムービーの再生
	HRESULT Play() { return m_pMC->Run(); };

	//! @fn VOID Stop()
	//! @brief ムービーの停止
	HRESULT Stop() { return m_pMC->Stop(); };


	//! @fn VOID SetSpeed(double time)
	//! @brief 再生スピードの変更
	//! @param time (in) 再生スピードの倍率
	HRESULT SetSpeed(double time) { return m_pMP->put_Rate(time); };

	//! @fn double GetStopTime()
	//! @brief 終了時間の取得
	//! @return 終了時間
	double GetStopTime() {
		REFTIME time;
		m_pMP->get_StopTime(&time);
		return time;
	};

	//! @fn double GetDuration()
	//! @brief ストリームの時間幅の取得
	//! @return 全ストリーム長
	double GetDuration() {
		REFTIME time;
		m_pMP->get_Duration(&time);
		return time;
	}

	//! @fn double GetCurrentPosition()
	//! @brief 現在の再生位置の取得
	//! @return 現在の再生位置
	double GetCurrentPosition() {
		REFTIME time;
		m_pMP->get_CurrentPosition(&time);
		return time;
	}

	//! @fn VOID SetTime(double time)
	//! @brief 現在の再生位置を指定位置にセット
	//! @param time (in) セットしたい再生位置
	//! @return 関数の成否
	HRESULT SetTime(double time) { return m_pMP->put_CurrentPosition(time); };

	//! @fn VOID GetUV(FLOAT* u, FLOAT* v)
	//! @brief 再生するテクスチャのサイズとムービーのサイズが合わない場合の余剰分をカットするためのUV値取得
	//! @param *u (out) テクスチャ座標U
	//! @param *v (out) テクスチャ座標V
	HRESULT GetUV(FLOAT* u, FLOAT* v) {
		if (u == NULL) return E_POINTER;
		if (v == NULL) return E_POINTER;

		*u = m_fu;
		*v = m_fv;

		return S_OK;
	}


	VOID GetMovieSize(LONG *plWidth, LONG *plHeight, LONG *plPitch) {
		if (plWidth != NULL) *plWidth = m_lWidth;
		if (plHeight != NULL) *plHeight = m_lHeight;
		if (plPitch != NULL) *plPitch = m_lPitch;
	}


	//! @fn VOID GetEvent(long* lEventCode, long* lParam1, long* lParam2, long msTimeout)
	//! @brief ムービーのイベント取得
	//! @param lEventCode (out) イベント コードを受け取る変数へのポインタ
	//! @param lParam1 (out) 第 1 イベント引数を受け取る変数へのポインタ。
	//! @param lParam2 (out) 第 2 イベント引数を受け取る変数へのポインタ。
	//! @param msTimeout (in) タイムアウト時間 (ミリ秒単位)。イベントが到着するまで動作を停止するには、INFINITE を使う。
	VOID GetEvent(long* lEventCode, LONG_PTR *lParam1, LONG_PTR *lParam2, long msTimeout);
};

#endif // MOVIETEX_H