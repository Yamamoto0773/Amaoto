/*
! @file movietex.cpp
! @bref	���[�r�[�e�N�X�`���N���X
! @author	Masafumi TAKAHASHI
*/

#pragma warning(disable : 4995)
#pragma warning(disable : 4996)

#include "DirectXMovie.h"

//�O���[�o��
TCHAR g_achCopy[]     = TEXT("Bitwise copy of the sample");
TCHAR g_achOffscr[]   = TEXT("Using offscreen surfaces and StretchCopy()");
TCHAR g_achDynTextr[] = TEXT("Using Dynamic Textures");
TCHAR* g_pachRenderMethod = NULL;

//-----------------------------------------------------------------------------
// Msg: Display an error message box if needed
//-----------------------------------------------------------------------------
void Msg(TCHAR *szFormat, ...) {
	TCHAR szBuffer[1024];  // Large buffer for long filenames or URLs
	const size_t NUMCHARS = sizeof(szBuffer) / sizeof(szBuffer[0]);
	const int LASTCHAR = NUMCHARS - 1;

	// Format the input string
	va_list pArgs;
	va_start(pArgs, szFormat);

	// Use a bounded buffer size to prevent buffer overruns.  Limit count to
	// character size minus one to allow for a NULL terminating character.
	_vsntprintf(szBuffer, NUMCHARS - 1, szFormat, pArgs);
	va_end(pArgs);

	// Ensure that the formatted string is NULL-terminated
	szBuffer[LASTCHAR] = TEXT('\0');

	MessageBox(NULL, szBuffer, TEXT("DirectShow CaptureTex9 Sample\0"),
		MB_OK | MB_ICONERROR);
}

//
//			CMovieTexture
//

//�R���X�g���N�^
CMovieTexture::CMovieTexture(LPUNKNOWN pUnk, HRESULT *phr)
	: CBaseVideoRenderer(__uuidof(CLSID_TextureRenderer),
		NAME("Texture Renderer"), pUnk, phr),
	m_bUseDynamicTextures(FALSE) {
	// Store and AddRef the texture for our use.
	ASSERT(phr);
	if (phr)
		*phr = S_OK;
}

//�f�X�g���N�^
CMovieTexture::~CMovieTexture() {
}

//-----------------------------------------------------------------------------
// CheckMediaType: This method forces the graph to give us an R8G8B8 video
// type, making our copy to texture memory trivial.
//-----------------------------------------------------------------------------
HRESULT CMovieTexture::CheckMediaType(const CMediaType *pmt) {
	HRESULT   hr = E_FAIL;
	VIDEOINFO *pvi=0;

	CheckPointer(pmt, E_POINTER);

	// Reject the connection if this is not a video type
	if (*pmt->FormatType() != FORMAT_VideoInfo) {
		return E_INVALIDARG;
	}

	// Only accept RGB24 video
	pvi = (VIDEOINFO *)pmt->Format();

	if (IsEqualGUID(*pmt->Type(), MEDIATYPE_Video)  &&
		IsEqualGUID(*pmt->Subtype(), MEDIASUBTYPE_RGB24)) {
		hr = S_OK;
	}

	return hr;
}

//-----------------------------------------------------------------------------
// SetMediaType: Graph connection has been made. 
//-----------------------------------------------------------------------------
HRESULT CMovieTexture::SetMediaType(const CMediaType *pmt) {
	HRESULT hr;

	UINT uintWidth = 2;
	UINT uintHeight = 2;

	D3DCAPS9 caps;
	VIDEOINFO *pviBmp;                      // �r�b�g�}�b�v�̏�񂪊i�[
	pviBmp = (VIDEOINFO *)pmt->Format();

	m_lVidWidth  = pviBmp->bmiHeader.biWidth;
	m_lVidHeight = abs(pviBmp->bmiHeader.biHeight);
	m_lVidPitch  = (m_lVidWidth * 3 + 3) & ~(3); // 24bit�J���[

												 // �e�N�X�`�����_�C�i�~�b�N�e�N�X�`�����`�F�b�N
	ZeroMemory(&caps, sizeof(D3DCAPS9));
	hr = m_pd3dDevice->GetDeviceCaps(&caps);
	if (caps.Caps2 & D3DCAPS2_DYNAMICTEXTURES) {
		// �_�C�i�~�b�N�e�N�X�`���̏ꍇ
		m_bUseDynamicTextures = TRUE;
	}

	if (caps.TextureCaps & D3DPTEXTURECAPS_POW2
		&& caps.TextureCaps & !D3DPTEXTURECAPS_NONPOW2CONDITIONAL) {
		// �e�N�X�`����2�̗ݏ�łȂ��ƍ쐬�ł��Ȃ��ꍇ
		while ((LONG)uintWidth < m_lVidWidth) {
			uintWidth = uintWidth << 1;
		}
		while ((LONG)uintHeight < m_lVidHeight) {
			uintHeight = uintHeight << 1;
		}
	}
	else {
		// �e�N�X�`������2�̗ݏ�ł悯���
		uintWidth = m_lVidWidth;
		uintHeight = m_lVidHeight;
		m_bUseDynamicTextures = FALSE;
	}

	hr = E_UNEXPECTED;
	if (m_bUseDynamicTextures) {
		// �_�C�i�~�b�N�e�N�X�`���ō쐬����
		hr = m_pd3dDevice->CreateTexture(uintWidth, uintHeight, 1, 0,
			D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT,
			&m_pTexture, NULL);
		g_pachRenderMethod = g_achDynTextr;
		if (FAILED(hr)) {
			m_bUseDynamicTextures = FALSE;
		}
	}
	if (FALSE == m_bUseDynamicTextures) {
		hr = m_pd3dDevice->CreateTexture(uintWidth, uintHeight, 1, 0,
			D3DFMT_X8R8G8B8, D3DPOOL_MANAGED,
			&m_pTexture, NULL);
		g_pachRenderMethod = g_achCopy;
	}
	if (FAILED(hr)) {
		Msg(TEXT("Could not create the D3DX texture!  hr=0x%x"), hr);
		return hr;
	}

	D3DSURFACE_DESC ddsd;
	ZeroMemory(&ddsd, sizeof(ddsd));

	if (FAILED(hr = m_pTexture->GetLevelDesc(0, &ddsd))) {
		Msg(TEXT("Could not get level Description of D3DX texture! hr = 0x%x"), hr);
		return hr;
	}


	CComPtr<IDirect3DSurface9> pSurf;

	if (SUCCEEDED(hr = m_pTexture->GetSurfaceLevel(0, &pSurf)))
		pSurf->GetDesc(&ddsd);

	// Save format info
	m_TextureFormat = ddsd.Format;

	if (m_TextureFormat != D3DFMT_X8R8G8B8 &&
		m_TextureFormat != D3DFMT_A1R5G5B5) {
		Msg(TEXT("Texture is format we can't handle! Format = 0x%x"), m_TextureFormat);
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	return S_OK;
}


//-----------------------------------------------------------------------------
// DoRenderSample: A sample has been delivered. Copy it to the texture.
//-----------------------------------------------------------------------------
HRESULT CMovieTexture::DoRenderSample(IMediaSample * pSample) {
	BYTE  *pBmpBuffer, *pTxtBuffer; // �r�b�g�}�b�v�A�e�N�X�`��
	LONG  lTxtPitch;                // Pitch of bitmap, texture

	BYTE  * pbS = NULL;
	DWORD * pdwS = NULL;
	DWORD * pdwD = NULL;
	UINT row, col, dwordWidth;

	CheckPointer(pSample, E_POINTER);
	CheckPointer(m_pTexture, E_UNEXPECTED);

	// �r�f�I����r�b�g�}�b�v�����o��
	pSample->GetPointer(&pBmpBuffer);

	// �e�N�X�`�������b�N����
	D3DLOCKED_RECT d3dlr;
	if (m_bUseDynamicTextures) {
		if (FAILED(m_pTexture->LockRect(0, &d3dlr, 0, D3DLOCK_DISCARD)))
			return E_FAIL;
	}
	else {
		if (FAILED(m_pTexture->LockRect(0, &d3dlr, 0, 0)))
			return E_FAIL;
	}
	// �e�N�X�`���̃s�b�`���̎擾
	pTxtBuffer = static_cast<byte *>(d3dlr.pBits);
	lTxtPitch = d3dlr.Pitch;


	// �R�s�[

	// 32bit�J���[�̃e�N�X�`���̏ꍇ
	if (m_TextureFormat == D3DFMT_X8R8G8B8) {
		// Instead of copying data bytewise, we use DWORD alignment here.
		// We also unroll loop by copying 4 pixels at once.
		//
		// original BYTE array is [b0][g0][r0][b1][g1][r1][b2][g2][r2][b3][g3][r3]
		//
		// aligned DWORD array is     [b1 r0 g0 b0][g2 b2 r1 g1][r3 g3 b3 r2]
		//
		// We want to transform it to [ff r0 g0 b0][ff r1 g1 b1][ff r2 g2 b2][ff r3 b3 g3]
		// below, bitwise operations do exactly this.

		dwordWidth = m_lVidWidth / 4; // aligned width of the row, in DWORDS
									  // (pixel by 3 bytes over sizeof(DWORD))

		for (row = 0; row< (UINT)m_lVidHeight; row++) {
			pdwS = (DWORD*)pBmpBuffer;
			pdwD = (DWORD*)pTxtBuffer;

			for (col = 0; col < dwordWidth; col++) {
				pdwD[0] =  pdwS[0] | 0xFF000000;
				pdwD[1] = ((pdwS[1]<<8)  | 0xFF000000) | (pdwS[0]>>24);
				pdwD[2] = ((pdwS[2]<<16) | 0xFF000000) | (pdwS[1]>>16);
				pdwD[3] = 0xFF000000 | (pdwS[2]>>8);
				pdwD +=4;
				pdwS +=3;
			}

			// we might have remaining (misaligned) bytes here
			pbS = (BYTE*)pdwS;
			for (col = 0; col < (UINT)m_lVidWidth % 4; col++) {
				*pdwD = 0xFF000000     |
					(pbS[2] << 16) |
					(pbS[1] <<  8) |
					(pbS[0]);
				pdwD++;
				pbS += 3;
			}

			pBmpBuffer  += m_lVidPitch;
			pTxtBuffer += lTxtPitch;
		}// for rows
	}

	// 16bit�̃e�N�X�`���̏ꍇ
	if (m_TextureFormat == D3DFMT_A1R5G5B5) {
		for (int y = 0; y < m_lVidHeight; y++) {
			BYTE *pBmpBufferOld = pBmpBuffer;
			BYTE *pTxtBufferOld = pTxtBuffer;

			for (int x = 0; x < m_lVidWidth; x++) {
				*(WORD *)pTxtBuffer = (WORD)
					(0x8000 +
						((pBmpBuffer[2] & 0xF8) << 7) +
						((pBmpBuffer[1] & 0xF8) << 2) +
						(pBmpBuffer[0] >> 3));

				pTxtBuffer += 2;
				pBmpBuffer += 3;
			}

			pBmpBuffer = pBmpBufferOld + m_lVidPitch;
			pTxtBuffer = pTxtBufferOld + lTxtPitch;
		}
	}

	// �A�����b�N
	if (FAILED(m_pTexture->UnlockRect(0)))
		return E_FAIL;

	return S_OK;
}

//�R���X�g���N�^
DirectXMovie::DirectXMovie() {
	m_pGB = NULL;
	m_pMC = NULL;
	m_pMP = NULL;
	m_pME = NULL;
	m_pRenderer = NULL;

	m_pTexture = NULL;
	m_pD3DVertex = NULL;

	m_iDrawWidth = 640;
	m_iDrawHeight = 480;
	m_fu = m_fv = 1.0f;
}

//�f�X�g���N�^
DirectXMovie::~DirectXMovie() {
	Clear();
}


//! @fn HRESULT InitDShowTextureRenderer(WCHAR* wFileName, const BOOL bSound)
//! @brief DirectShow����e�N�X�`���ւ̃����_�����O�ւ̏�����
//! @param *lpD3DDevice (in) Direct3D�f�o�C�X
//! @param *wFileName (in) ���[�r�[�t�@�C���̃p�X
//! @param bSound (in) �T�E���h�Đ��t���O
//! @param iDrawWidth (in) iDrawWidth ������Đ�����E�B���h�E�̉���
//! @param iDrawHeight (in) iDrawHeight ������Đ�����E�B���h�E�̏c��
//! @return �֐��̐���
HRESULT DirectXMovie::Create(IDirect3DDevice9 *lpD3DDevice, WCHAR* wFileName, const BOOL bSound, int iDrawWidth, int iDrawHeight) {
	if (lpD3DDevice == NULL)	return E_POINTER;
	if (iDrawWidth <= 0)		return E_INVALIDARG;
	if (iDrawHeight <= 0)		return E_INVALIDARG;

	Clear();

	HRESULT hr = S_OK;
	CComPtr<IBaseFilter>    pFSrc;          // Source Filter
	CComPtr<IPin>           pFSrcPinOut;    // Source Filter Output Pin   
	CMovieTexture			*pCTR=0;        // DirectShow Texture renderer

	// ����ۑ�
	m_pD3DDevice = lpD3DDevice;
	m_iDrawWidth = iDrawWidth;
	m_iDrawHeight = iDrawHeight;

	// �t�B���^�O���t�̍쐬
	if (FAILED(m_pGB.CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC)))
		return E_FAIL;

#ifdef REGISTER_FILTERGRAPH
	// Register the graph in the Running Object Table (for debug purposes)
	AddToROT(m_pGB);
#endif

	// Create the Texture Renderer object
	pCTR = new CMovieTexture(NULL, &hr);
	pCTR->SetDevice(m_pD3DDevice);
	if (FAILED(hr) || !pCTR) {
		Msg(TEXT("Could not create texture renderer object!  hr=0x%x"), hr);
		return E_FAIL;
	}

	// Get a pointer to the IBaseFilter on the TextureRenderer, add it to graph
	m_pRenderer = pCTR;
	if (FAILED(hr = m_pGB->AddFilter(m_pRenderer, L"TextureRenderer"))) {
		Msg(TEXT("Could not add renderer filter to graph!  hr=0x%x"), hr);
		return hr;
	}

	// �t�B���^�O���O�Ƀ\�[�X��ǉ�
	hr = m_pGB->AddSourceFilter(wFileName, L"SOURCE", &pFSrc);

	// If the media file was not found, inform the user.
	if (hr == VFW_E_NOT_FOUND) {
		Msg(TEXT("Could not add source filter to graph!  (hr==VFW_E_NOT_FOUND)\r\n\r\n")
			TEXT("This sample reads a media file from the DirectX SDK's media path.\r\n")
			TEXT("Please install the DirectX 9 SDK on this machine."));
		return hr;
	}
	else if (FAILED(hr)) {
		Msg(TEXT("Could not add source filter to graph!  hr=0x%x"), hr);
		return hr;
	}

	if (SUCCEEDED(hr = pFSrc->FindPin(L"Output", &pFSrcPinOut))) {
		//AVI�Ƃ�MPEG1�Ƃ�VFW�ł�������

		//�I�[�f�B�I�g���b�N���Đ����Ȃ��ꍇ�@NO_AUDIO_RENDERER�@���`
		if (!bSound) {

			// If no audio component is desired, directly connect the two video pins
			// instead of allowing the Filter Graph Manager to render all pins.

			CComPtr<IPin> pFTRPinIn;      // Texture Renderer Input Pin

										  // Find the source's output pin and the renderer's input pin
			if (FAILED(hr = pCTR->FindPin(L"In", &pFTRPinIn))) {
				Msg(TEXT("Could not find input pin!  hr=0x%x"), hr);
				return hr;
			}

			// Connect these two filters
			if (FAILED(hr = m_pGB->Connect(pFSrcPinOut, pFTRPinIn))) {
				Msg(TEXT("Could not connect pins!  hr=0x%x"), hr);
				return hr;
			}
		}
		else {

			// Render the source filter's output pin.  The Filter Graph Manager
			// will connect the video stream to the loaded CTextureRenderer
			// and will load and connect an audio renderer (if needed).

			if (FAILED(hr = m_pGB->Render(pFSrcPinOut))) {
				Msg(TEXT("Could not render source output pin!  hr=0x%x"), hr);
				return hr;
			}
		}
	}
	else {
		CComPtr<IBasicAudio> IAudio;
		hr = m_pGB.QueryInterface(&IAudio);

		//WMV�Ƃ�MPEG2�Ƃ�
		hr = m_pGB->RenderFile(wFileName, NULL);
		if (FAILED(hr))
			return hr;
		if (!bSound) {
			// ����
			hr = IAudio->put_Volume(-10000);
		}
		else {
			//���̏o��
			hr = IAudio->put_Volume(0);
		}
	}

	// Get the graph's media control, event & position interfaces
	m_pGB.QueryInterface(&m_pMC);
	m_pGB.QueryInterface(&m_pMP);
	m_pGB.QueryInterface(&m_pME);

	// Start the graph running;
	if (FAILED(hr = m_pMC->Run())) {
		Msg(TEXT("Could not run the DirectShow graph!  hr=0x%x"), hr);
		return hr;
	}

	//�e�N�X�`���̎擾
	m_pTexture = pCTR->GetTexture();
	pCTR->GetVideoDesc(&m_lWidth, &m_lHeight, &m_lPitch);

	D3DSURFACE_DESC desc;
	m_pTexture->GetLevelDesc(0, &desc);

	//�e�N�X�`����2�̗ݏ�ł����m�ۂł��Ȃ��ꍇ�ɗ]�蕪��\�����Ȃ����߂�UV�v�Z
	m_fu = (FLOAT)m_lWidth / (FLOAT)desc.Width;
	m_fv = (FLOAT)m_lHeight / (FLOAT)desc.Height;


	// ���_�o�b�t�@�쐬
	hr = m_pD3DDevice->CreateVertexBuffer(sizeof(MOVIEVERTEX)*4, 0, D3DFVF_MOVIEVERTEX, D3DPOOL_MANAGED, &m_pD3DVertex, NULL);
	if (FAILED(hr)) return hr;
	MOVIEVERTEX vtx[] ={
			{ 0.0f, -1.0f, 1.0f, 0.0f, 0.0f },
			{ 0.0f,  0.0f, 1.0f, 0.0f, m_fv },
			{ 1.0f, -1.0f, 1.0f, m_fu, 0.0f },
			{ 1.0f,  0.0f, 1.0f, m_fu, m_fv },
	};

	MOVIEVERTEX *access = NULL;
	hr = m_pD3DVertex->Lock(0, sizeof(MOVIEVERTEX)*4, (void**)&access, 0);
	if (FAILED(hr)) return hr;
	memcpy(access, vtx, sizeof(MOVIEVERTEX)*4);
	m_pD3DVertex->Unlock();
	if (FAILED(hr)) return hr;

	return S_OK;

}



HRESULT DirectXMovie::DrawMovie(POINT &drawPoint, MOVIEPOSITIONFORMAT format) {
	if (&drawPoint == NULL) return E_POINTER;

	// �Z�`�̈���Z�o
	RECT drawArea;
	LONG w, h;
	GetMovieSize(&w, &h, NULL);
	if (format == MOVPOSFMT_CENTER) {
		drawArea.left	= drawPoint.x - w/2;
		drawArea.top	= drawPoint.y - h/2;
		drawArea.right	= drawPoint.x + w/2;
		drawArea.bottom	= drawPoint.y + h/2;
	}
	else {
		drawArea.left	= drawPoint.x;
		drawArea.top	= drawPoint.y;
		drawArea.right	= drawPoint.x + w;
		drawArea.bottom	= drawPoint.y + h;
	}

	return DrawMovie(drawArea, DRAWMOVOPT_ZOOMDISABLED);
}



HRESULT DirectXMovie::DrawMovie(RECT &drawArea, DRAWMOVIEOPTION option) {
	if (m_pD3DDevice	== NULL) return E_FAIL;
	if (m_pD3DVertex	== NULL) return E_FAIL;
	if (m_pGB			== NULL) return E_FAIL;
	if (&drawArea		== NULL) return E_POINTER;

	HRESULT hr = S_OK;
	LONG movieSizeW, movieSizeH;
	LONG drawSizeW, drawSizeH;
	LONG resultSizeW, resultSizeH;

	// �X�N���[���̍���������_�Ƃ������W�ɕϊ�
	drawArea.left -= m_iDrawWidth/2;
	drawArea.right -= m_iDrawWidth/2;
	drawArea.top -= m_iDrawHeight/2;
	drawArea.top *= -1;
	drawArea.bottom -= m_iDrawHeight/2;
	drawArea.bottom *= -1;

	// �t�H�[�}�b�g�����Ƃɓ���T�C�Y���Z�o
	drawSizeW = drawArea.right - drawArea.left;
	drawSizeH = drawArea.top - drawArea.bottom;
	GetMovieSize(&movieSizeW, &movieSizeH, NULL);
	if (option == DRAWMOVOPT_ASPECTVARI) {
		resultSizeW = drawSizeW;
		resultSizeH = drawSizeH;
	}
	else {
		// �`��̈�Ɠ���̃A�X�y�N�g����r
		if ((FLOAT)drawSizeW/(FLOAT)drawSizeH > (FLOAT)movieSizeW/(FLOAT)movieSizeH) {
			// �`��̈�̏c�̒����ɓ�������킹��
			FLOAT scale = (FLOAT)movieSizeH/(FLOAT)drawSizeH;
			if (option == DRAWMOVOPT_ZOOMDISABLED && scale < 1.0f) scale = 1.0f;

			resultSizeH = (INT)(movieSizeH/scale);
			resultSizeW = (INT)(movieSizeW/scale);
		}
		else {
			// �`��̈�̕��̒����ɓ�������킹��
			FLOAT scale = (FLOAT)movieSizeW/(FLOAT)drawSizeW;
			if (option == DRAWMOVOPT_ZOOMDISABLED && scale < 1.0f) scale = 1.0f;

			resultSizeW = (INT)(movieSizeW/scale);
			resultSizeH = (INT)(movieSizeH/scale);
		}
	}


	//-------------------�`��ݒ�---------------------------------------
	// ���C�g�̓I�t��
	m_pD3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	// ���u�����h�ݒ�
	m_pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	//�e�N�X�`���̃A���t�@�𓧖��x���g�p����ݒ�
	m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
	m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	m_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	m_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	m_pD3DDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	m_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);					// ����n

	m_pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);			// �e�N�X�`�����͂ݏo�����ɕ\�����Ȃ��ɂ���
	m_pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	m_pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	m_pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	//-------------------------------------------------------------------

	D3DXMATRIX worldOffset;						// �|���S���̔z�u���W
	D3DXMATRIX localScale;						// �|���S���̑傫�� (�g��{��
	D3DXMATRIX localOffset;						// �|���S���̌��_���W
	D3DXMATRIX localMat;
	D3DXMATRIX world;

	// ������W�n���ˉe�s���ݒ�
	D3DXMATRIX ortho;
	D3DXMatrixOrthoLH(&ortho, (float)m_iDrawWidth, (float)m_iDrawHeight, 0.0f, 1000.0f);
	m_pD3DDevice->SetTransform(D3DTS_PROJECTION, &ortho);

	// ���[���h�r���[�ˉe�ϊ��s����쐬
	D3DXMatrixScaling(&localScale, resultSizeW, resultSizeH, 1.0f);				// �|���S���𕶎��̑傫���ɂ���
	D3DXMatrixTranslation(&localOffset, -resultSizeW/2.0f, +resultSizeH/2.0f, 0.0f);
	localMat = localScale*localOffset;
	D3DXMatrixTranslation(&worldOffset, (drawArea.left + drawArea.right)/2.0f -0.5f, (drawArea.top + drawArea.bottom)/2.0f +0.5f, 0.0f);
	world = localMat * worldOffset;

	hr = m_pD3DDevice->SetTransform(D3DTS_WORLD, &world);
	if (FAILED(hr)) return hr;
	hr = m_pD3DDevice->SetStreamSource(0, m_pD3DVertex, 0, sizeof(MOVIEVERTEX));
	if (FAILED(hr)) return hr;
	hr = m_pD3DDevice->SetFVF(D3DFVF_MOVIEVERTEX);
	if (FAILED(hr)) return hr;
	hr = m_pD3DDevice->SetTexture(0, m_pTexture);
	if (FAILED(hr)) return hr;
	hr = m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	if (FAILED(hr)) return hr;

	return hr;
}



//-----------------------------------------------------------------------------
// CheckMovieStatus: If the movie has ended, rewind to beginning
//-----------------------------------------------------------------------------
void DirectXMovie::CheckMovieStatus(void) {
	long lEventCode;
	long lParam1;
	long lParam2;
	HRESULT hr;

	if (!m_pME)
		return;

	// Check for completion events
	hr = m_pME->GetEvent(&lEventCode, (LONG_PTR *)&lParam1, (LONG_PTR *)&lParam2, 0);
	if (SUCCEEDED(hr)) {
		// If we have reached the end of the media file, reset to beginning
		if (EC_COMPLETE == lEventCode) {
			hr = m_pMP->put_CurrentPosition(0);
		}

		// Free any memory associated with this event
		hr = m_pME->FreeEventParams(lEventCode, lParam1, lParam2);
	}
}


//-----------------------------------------------------------------------------
// CleanupDShow
//-----------------------------------------------------------------------------
void DirectXMovie::Clear(void) {
#ifdef REGISTER_FILTERGRAPH
	// Pull graph from Running Object Table (Debug)
	RemoveFromROT();
#endif

	// Shut down the graph
	if (!(!m_pMC)) m_pMC->Stop();

	//�e��I�u�W�F�N�g�̉��
	if (!(!m_pMC)) m_pMC.Release();
	if (!(!m_pME)) m_pME.Release();
	if (!(!m_pMP)) m_pMP.Release();
	if (!(!m_pGB)) m_pGB.Release();
	if (!(!m_pRenderer)) m_pRenderer.Release();

	if (m_pD3DVertex != NULL) m_pD3DVertex.Release();
}

//���[�r�[�̃C�x���g���擾
VOID DirectXMovie::GetEvent(long* lEventCode, LONG_PTR *lParam1, LONG_PTR *lParam2, long msTimeout) {
	m_pME->GetEvent(lEventCode, lParam1, lParam2, msTimeout);
}
