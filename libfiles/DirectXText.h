#ifndef _DIRECTXFONT_H
#define _DIRECTXFONT_H

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#include <windows.h>
#include <tchar.h>
#include <d3dx9.h>
#include <d3d9.h>
#include <stdio.h>

#include "DirectXTextClass.h"

#define DFONT_MAXSTRING 2048
#define DFONT_MAXCHARACTER 256


class DirectXText {
private:
	LPDIRECT3DDEVICE9		lpDev;
	IDirect3DVertexBuffer9	*lpVertexBuffer;
	ID3DXEffect				*lpEffect;			// �V�F�[�_�Ǘ��p
	IDirect3DVertexDeclaration9	*lpDecl;		// ���_�錾�쐬

	int						iDrawWidth;
	int						iDrawHeight;

	CHARDATA				*pCharData[DFONT_MAXSTRING];
	int						iFontSize[DFONT_MAXSTRING];

	WCHAR					*pSetString[DFONT_MAXSTRING];

	IDirect3DTexture9		*lpFontTex[DFONT_MAXSTRING][DFONT_MAXCHARACTER];		// �����̃e�N�X�`��


public:
	DirectXText();
	virtual ~DirectXText();

	//�@DirectXText������
	BOOL Init(LPDIRECT3DDEVICE9 dev, int drawWidth, int drawHeight);
	// �`�悷�镶���̓o�^
	BOOL SetString(int strID, FONTSTATUS *status, const char * s, ...);
	// �����`��
	BOOL Draw(int strID, int x, int y, int fontSize, int charInterval, DWORD color);
	// �w��̈���֕����`��
	BOOL DrawInRect(int strID, RECT *rect, int fontSize, int charInterval, DWORD format, DWORD color);
	// �t�H���g�e�N�X�`�����J������ (�f�X�g���N�^�Ŏ����I�ɌĂяo����܂��B�����I�ɌĂяo���K�v�͂���܂���B
	BOOL Clear(int strID);

	
	// FONTSTATUS�\���̂ɒl���i�[����@(�֐����g�킸�ɒ��ڍ\���̂ɒl���i�[���Ă��\���܂���)
	BOOL SetFontStatus(FONTSTATUS *status, int fontSize, WCHAR *fontName, int fontWeight, bool italic);
	BOOL SetFontStatusEX(FONTSTATUS *status, int fontSize, WCHAR *fontName, int fontWeight, bool italic, bool underLine, bool struckOut, const int antiAliasing = GGO_GRAY4_BITMAP);

	// RGBA��DWORD�^�ɕϊ�����֐��B�F��4�����Ŏw�肵�����Ƃ��Ɏg���܂�
	DWORD ConvertFromRGBA(int red, int green, int blue, int alpha = 255);

private:
	// �^����ꂽ�`��̈�ƃt�H�[�}�b�g����A�ŏI�I�ȕ`����W���v�Z
	int	CalcTextPosition(int strID, RECT *rect, float inScale, int charInterval, DWORD format, const WCHAR *s, POINT *pt, float *outScale);
	// ��������œK������ (�߂�l�͍œK����̕�����)
	int OptimizeString(WCHAR *dst, const WCHAR *src);


};


#endif // !_DIRECTXFONT_H

