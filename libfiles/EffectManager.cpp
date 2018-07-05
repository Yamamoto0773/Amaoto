#include "EffectManager.hpp"


EffectManager::EffectManager() {
	ulTimeLength = 0;
	uiIndex = 0;
}

EffectManager::~EffectManager() {
	effect.erase(effect.cbegin(), effect.cend());
}

void EffectManager::Reset() {
	effect.assign(effect.size(), { 0ul, 0.0f, 0.0f });
	ulTimeLength = 0;
	uiIndex = 0;
}

void EffectManager::Create(const draweffectoption_func& changeDrawingFunc, double timeLength, unsigned int element) {
	effect.resize(element);
	effect.assign(effect.size(), { 0ul, 0.0f, 0.0f });

	ulTimeLength = timeLength;
	uiIndex = 0;
}

bool SetEffectImg(IDirect3DDevice9 *lpDev, wchar_t *ImgFileName, int x, int y, int w, int h) {
	
}

void EffectManager::Start(float x, float y) {
	Start(x, y, ulTimeLength);
}

void EffectManager::Start(float x, float y, double timeLength) {
	effect[uiIndex].startTimeCnt = timer._GetNowCount();
	effect[uiIndex].length = timeLength;
	effect[uiIndex].x = x;
	effect[uiIndex].y = y;

	uiIndex++;
	if (uiIndex >= effect.size()) uiIndex = 0;
}

bool EffectManager::Draw() const {

	return true;
}

CreateTexture(wchar_t *fileName){
	// テクスチャメモリを解放
	if (lpTex != nullptr) {
		lpTex->Release();
		lpTex = nullptr;
	}

	// 対応画像か
	if( strnicmp(&file[strlen(file)-4],".BMP",4)==0 ||
		strnicmp(&file[strlen(file)-4],".TGA",4)==0 ||
		strnicmp(&file[strlen(file)-4],".DDS",4)==0 ||
		strnicmp(&file[strlen(file)-4],".DIB",4)==0 ||
		strnicmp(&file[strlen(file)-4],".JPG",4)==0 ||
		strnicmp(&file[strlen(file)-4],".PNG",4)==0 ) {

		D3DXIMAGE_INFO imgInfo;
		ZeroMemory( &imgInfo,sizeof(D3DXIMAGE_INFO) );
		HRESULT ret = D3DXCreateTextureFromFileExA(
				dev,
				file,
				D3DX_DEFAULT,
				D3DX_DEFAULT,
				1,
				0,
				D3DFMT_UNKNOWN,
				D3DPOOL_MANAGED,
				D3DX_FILTER_POINT,
				D3DX_FILTER_NONE,
				0,
				&imgInfo,
				NULL,
				&lpTex );
		if( FAILED(ret) ) {
			DEBUG( "[%s] 画像ロード失敗\n",file );
			return FALSE;
		}

		// サイズを取得
		mTexSize.w = imgInfo.Width;
		mTexSize.h = imgInfo.Height;
		
}



DRAWEFFECTPARAM EffectManager::drawEffectDefFunc(float timePos) {
	DRAWEFFECTPARAM param;
	param.alpha = timePos;
	param.offsetX = 0.0f;
	param.offsetY = 0.0;
	param.scale = 1.0f;

	return param;
}