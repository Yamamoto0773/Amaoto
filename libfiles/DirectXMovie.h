/*
! �Q�l�\�[�X�R�[�h
!
! @file movietex.h
! @bref	���[�r�[�e�N�X�`���N���X
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
//! @brief �r�f�I��Direct3DTexture9�ɏ������ޏ������s���N���X
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
														//! @brief ���[�r�[�e�N�X�`���̎擾
														//! @return ���[�r�[�������_�����O���ꂽ�e�N�X�`��
	IDirect3DTexture9 * GetTexture() { return m_pTexture; };

	//! @fn VOID GetVideoDesc(LONG* plVidWidth, LONG* plVidHeight, LONG* plVidPitch)
	//! @brief �r�f�I�̕��A�����A�s�b�`�̎擾
	//! @param *plVidWidth (out) �r�f�I�̕�
	//! @param *plVidHeigh (out) �r�f�I�̍�
	//! @param *plVidPitch (out) �r�f�I�̃s�b�`
	VOID GetVideoDesc(LONG* plVidWidth, LONG* plVidHeight, LONG* plVidPitch) {
		*plVidWidth = m_lVidWidth;
		*plVidHeight = m_lVidHeight;
		*plVidPitch = m_lVidPitch;
	};

	//! @param �_�C�i�~�b�N�e�N�X�`�����g�����ǂ����ǂ����̃t���O
	BOOL m_bUseDynamicTextures;
	//! @param �r�f�I�̕�
	LONG m_lVidWidth;
	//! @param �r�f�I�̍���
	LONG m_lVidHeight;
	//! @param �r�f�I�̃s�b�`
	LONG m_lVidPitch;
};







#define D3DFVF_MOVIEVERTEX (D3DFVF_XYZ | D3DFVF_TEX1)

typedef struct _MOVIEVERTEX {
	float x, y, z;
	float u, v;
} MOVIEVERTEX;


// ����̈ʒu���w�肷��Ƃ��Ɏg���t���O
typedef enum _MOVIEPOSITIONFORMAT {
	MOVPOSFMT_CENTER,		// ����̒����̍��W���w��
	MOVPOSFMT_UPPERRIGHT,	// ����̉E��̍��W���w��
}MOVIEPOSITIONFORMAT;


// ����̊g��k���������w�肷��Ƃ��Ɏg���t���O
typedef enum _DRAWMOVIEOPTION {
	DRAWMOVOPT_ASPECTVARI,	// �Z�`�̈�Ɉ�v������悤�ɏc����𖳎����ē�����g��k��
	DRAWMOVOPT_ASPECTFIXED,	// �Z�`�̈�Ɉ�v������悤�ɏc������ێ����ē�����g��k��
	DRAWMOVOPT_ZOOMDISABLED,// �Z�`�̈�Ɏ��܂�悤�ɓ�����k���@���g��͂���܂���
}DRAWMOVIEOPTION;


//! @class CMovieToTexture
//! @brief ���[�r�[�e�N�X�`���N���X
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

	//! @param ��  
	LONG m_lWidth;
	//! @param ����
	LONG m_lHeight;
	//! @param �s�b�`
	LONG m_lPitch;

	//! @param ���̃��[�r�[�̃T�C�Y(���A����) / �e�N�X�`���̃T�C�Y(���A����)�ŎZ�o����UV�l
	FLOAT m_fu, m_fv;

	void CheckMovieStatus(void);
	

public:
	DirectXMovie();	//�R���X�g���N�^
	~DirectXMovie();	//�f�X�g���N�^

	VOID Clear(void);

	//! @fn HRESULT InitDShowTextureRenderer(WCHAR* wFileName, const BOOL bSound)
	//! @brief DirectShow����e�N�X�`���ւ̃����_�����O�ւ̏�����
	//! @param *lpD3DDevice (in) Direct3D�f�o�C�X
	//! @param *wFileName (in) ���[�r�[�t�@�C���̃p�X
	//! @param bSound (in) �T�E���h�Đ��t���O
	//! @param iDrawWidth (in) iDrawWidth ������Đ�����E�B���h�E�̉���
	//! @param iDrawHeight (in) iDrawHeight ������Đ�����E�B���h�E�̏c��
	//! @return �֐��̐���
	HRESULT Create(IDirect3DDevice9 *lpD3DDevice, WCHAR* wFileName, const BOOL bSound, int iDrawWidth, int iDrawHeight);

	//! @fn IDirect3DTexture9 * GetTexture()
	//! @brief ���[�r�[�e�N�X�`���̎擾
	//! @return ���[�r�[�������_�����O���ꂽ�e�N�X�`��
	HRESULT GetTexture(IDirect3DTexture9** pTexture) { return m_pTexture.CopyTo(pTexture); };


	// ������e�N�X�`���Ƃ��Ďw��̈���ɕ`��
	// ���� (in) drawArea �����`�悷��̈�(�X�N���[�����W)
	// �߂�l �֐��̐���
	HRESULT DrawMovie(POINT &drawPoint, MOVIEPOSITIONFORMAT format);
	HRESULT DrawMovie(RECT &drawArea, DRAWMOVIEOPTION option);
	

	//! @fn VOID Play()
	//! @brief ���[�r�[�̍Đ�
	HRESULT Play() { return m_pMC->Run(); };

	//! @fn VOID Stop()
	//! @brief ���[�r�[�̒�~
	HRESULT Stop() { return m_pMC->Stop(); };


	//! @fn VOID SetSpeed(double time)
	//! @brief �Đ��X�s�[�h�̕ύX
	//! @param time (in) �Đ��X�s�[�h�̔{��
	HRESULT SetSpeed(double time) { return m_pMP->put_Rate(time); };

	//! @fn double GetStopTime()
	//! @brief �I�����Ԃ̎擾
	//! @return �I������
	double GetStopTime() {
		REFTIME time;
		m_pMP->get_StopTime(&time);
		return time;
	};

	//! @fn double GetDuration()
	//! @brief �X�g���[���̎��ԕ��̎擾
	//! @return �S�X�g���[����
	double GetDuration() {
		REFTIME time;
		m_pMP->get_Duration(&time);
		return time;
	}

	//! @fn double GetCurrentPosition()
	//! @brief ���݂̍Đ��ʒu�̎擾
	//! @return ���݂̍Đ��ʒu
	double GetCurrentPosition() {
		REFTIME time;
		m_pMP->get_CurrentPosition(&time);
		return time;
	}

	//! @fn VOID SetTime(double time)
	//! @brief ���݂̍Đ��ʒu���w��ʒu�ɃZ�b�g
	//! @param time (in) �Z�b�g�������Đ��ʒu
	//! @return �֐��̐���
	HRESULT SetTime(double time) { return m_pMP->put_CurrentPosition(time); };

	//! @fn VOID GetUV(FLOAT* u, FLOAT* v)
	//! @brief �Đ�����e�N�X�`���̃T�C�Y�ƃ��[�r�[�̃T�C�Y������Ȃ��ꍇ�̗]�蕪���J�b�g���邽�߂�UV�l�擾
	//! @param *u (out) �e�N�X�`�����WU
	//! @param *v (out) �e�N�X�`�����WV
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
	//! @brief ���[�r�[�̃C�x���g�擾
	//! @param lEventCode (out) �C�x���g �R�[�h���󂯎��ϐ��ւ̃|�C���^
	//! @param lParam1 (out) �� 1 �C�x���g�������󂯎��ϐ��ւ̃|�C���^�B
	//! @param lParam2 (out) �� 2 �C�x���g�������󂯎��ϐ��ւ̃|�C���^�B
	//! @param msTimeout (in) �^�C���A�E�g���� (�~���b�P��)�B�C�x���g����������܂œ�����~����ɂ́AINFINITE ���g���B
	VOID GetEvent(long* lEventCode, LONG_PTR *lParam1, LONG_PTR *lParam2, long msTimeout);
};

#endif // MOVIETEX_H