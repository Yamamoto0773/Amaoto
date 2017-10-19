#pragma warning( disable : 4995 )				// 警告を無視
#pragma warning( disable : 4996 )				// 警告を無視
#include "CGame.h"
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>
#include "resource.h"

#define DEBUGMODE
#include "libfiles/DEBUG.H"

#define STR(var) #var

/////////////////////////////////////////////////////////////////////////
// コンストラクタ
/////////////////////////////////////////////////////////////////////////
CGame::CGame() {
	int i;
	bLostDevice		= FALSE;

	eState			= G_INIT;
	fScrMulti		= 1.0f;
	llStartTime		= 0;
	llGlobalFreq	= 0;
	dElapsedTime	= 0;
	iCursor1		= 0;
	iCursor2		= 0;
	iLoadMusicCnt	= 0;
	iBackMediaIndex	= -1;
	iRippleEffectIndex = 0;
	bFlag			= FALSE;
	ZeroMemory(iRippleEffectCount, sizeof(iRippleEffectCount));
	bIsBackMedia	= FALSE;
	ZeroMemory(&iStartNum, sizeof(iStartNum));

	ZeroMemory(&bOnVirtualKey, sizeof(bOnVirtualKey));
	ZeroMemory(&iNoteEffectIndex, sizeof(iNoteEffectIndex));
	ZeroMemory(&iNoteEffectCount, sizeof(iNoteEffectCount));
	ZeroMemory(&dGrooveRateCoef, sizeof(dGrooveRateCoef));
	for (i = 0; i < BMS_MAXNOTELANE; i++)
		dGrooveRateCoef[i] = 1.0;
	for (i=0; i<VIRTUALKEYCNT; i++)
		lHoldNote[i] = -1;

}


/////////////////////////////////////////////////////////////////////////
// デストラクタ
/////////////////////////////////////////////////////////////////////////
CGame::~CGame() {
	Clear();
}


///////////////////////////////////////////////////////
// 初期化＆ゲーム形成
///////////////////////////////////////////////////////
BOOL CGame::Init(HINSTANCE hinst) {
	// ウインドウ生成
	win.SetWindowStyle(WS_OVERLAPPEDWINDOW);					// 枠無しウインドウ(フルスクリーン時はWS_POPUPのみ、ウィンドウモード時はさらにWS_CAPTION|WS_SYSMENUなどを付ける)
	win.SetIcon(MAKEINTRESOURCEW(IDI_ICON1));		// アイコン設定
	if (!win.Create(hinst, L"Amaoto", 1, 1280, 720)) {	// ウィンドウサイズは1280*720
		DEBUG("Window create error\n");
		return FALSE;
	}
	ImmAssociateContext(win.hWnd, NULL);			// IMEを出さないようにする

	// Direct3D生成
	// フルスクリーンの1920*1080の32bitカラーにセットする。
	// ※2つ目の引数をFALSEにするとウインドウモードに出来る
	if (!dd.Create(win.hWnd, FALSE, 1920, 1080, 32, 0, TRUE)) {
		DEBUG("Direct3D create error\n");
		return FALSE;
	}

	// DirectXText初期化
	if (!dty.Init(dd.GetD3DDevice(), 1920, 1080)) {
		DEBUG("DirextXFont Init error\n");
	}

	// DXTextANSI初期化
	if (!dtc.Init(dd.GetD3DDevice(), 1920, 1080)) {
		DEBUG("DXTextANSI Init error\n");
	}
	if (!dtcs.Init(dd.GetD3DDevice(), 1920, 1080)) {
		DEBUG("DXTextANSI Init error\n");
	}
	if (!dts.Init(dd.GetD3DDevice(), 1920, 1080)) {
		DEBUG("DXTextANSI Init error\n");
	}
	// DirectXFigure初期化
	if (!df.Init(dd.GetD3DDevice()))
		DEBUG("DirectXFigure Init error\n");

	// DirectSound生成
	if (!ds.Create(win.hWnd)) {
		DEBUG("DirectAudio create error\n");
		return FALSE;
	}


	// DirectInput生成
	if (!di.Create(win.hWnd, win.hInstance)) {
		DEBUG("DirectInput生成失敗\n");
		return FALSE;
	}

	// キーボードを使う
	if (!di.CreateKeyboard()) {
		DEBUG("キーボード使用不可\n");
		return FALSE;
	}

	// ジョイスティックを使う
	if (!di.CreateJoystick()) {
		DEBUG("ジョイスティック使用不可\n");
		//		return FALSE;				// ジョイスティックが使用できなくても起動可能とする
	}


	// MIDIキーボード入力の準備
	if (!mi.Create()) {
		DEBUG("MIDIキーボード接続失敗\n");
		//return FALSE;
	}


	// 画像ロード
	if (!dd.AddTexture(TEXFILE_IMAGE0, "FILES\\image0.png")) {
		DEBUG("image.png load error\n");
		return FALSE;
	}
	if (!dd.AddTexture(TEXFILE_IMAGE1, "FILES\\image1.png")) {
		DEBUG("image1.png load error\n");
		return FALSE;
	}
	if (!dd.AddTexture(TEXFILE_IMAGE2, "FILES\\image2.png")) {
		DEBUG("image2.png load error\n");
		return FALSE;
	}
	if (!dd.AddTexture(TEXFILE_IMAGE3, "FILES\\image3.png")) {
		DEBUG("image3.png load error\n");
		return FALSE;
	}
	if (!dd.AddTexture(TEXFILE_IMAGE4, "FILES\\image4.png")) {
		DEBUG("image4.png load error\n");
	}
	if (!dd.AddTexture(TEXFILE_IMAGE5, "FILES\\image5.png")) {
		DEBUG("image5.png load error\n");
	}
	if (!dd.AddTexture(TEXFILE_IMAGE6, "FILES\\image6.png")) {
		DEBUG("image6.png load error\n");
	}
	if (!dd.AddTexture(TEXFILE_IMAGE7, "FILES\\image7.png")) {
		DEBUG("image7.png load error\n");
	}
	if (!dd.AddTexture(TEXFILE_IMAGE8, "FILES\\image8.png")) {
		DEBUG("image8.png load error\n");
	}


	// 判定ライン
	dd.SetPutRange(TEX_JUDGELINE, TEXFILE_IMAGE0, 0, 992, 1940, 15);

	// 判定ラインのエフェクト
	dd.SetPutRange(TEX_JUDGELINEEFFECT, TEXFILE_IMAGE0, 0, 899, 1940, 76);

	//// ノートレーン
	//dd.SetPutRange(2, 0, 72, 838, 1768, 8);

	// ノート
	dd.SetPutRange(TEX_NOTE, TEXFILE_IMAGE0, 62, 475, 150, 227);
	dd.SetPutStatus(TEX_NOTE, 1.0f, 0.3f, 0.0f);

	// 判定文字
	dd.SetPutRange(TEX_JUDGESTR_EARLY2, TEXFILE_IMAGE1, 0, 0, 227, 123);
	dd.SetPutRange(TEX_JUDGESTR_EARLY, TEXFILE_IMAGE1, 0, 130, 227, 123);
	dd.SetPutRange(TEX_JUDGESTR_JUSTE, TEXFILE_IMAGE1, 0, 260, 180, 123);
	dd.SetPutRange(TEX_JUDGESTR_JUST, TEXFILE_IMAGE1, 0, 390, 180, 123);
	dd.SetPutRange(TEX_JUDGESTR_JUSTL, TEXFILE_IMAGE1, 0, 520, 180, 123);
	dd.SetPutRange(TEX_JUDGESTR_LATE, TEXFILE_IMAGE1, 0, 650, 211, 123);
	dd.SetPutRange(TEX_JUDGESTR_LATE2, TEXFILE_IMAGE1, 0, 780, 211, 123);
	dd.SetPutRange(TEX_JUDGESTR_MISS, TEXFILE_IMAGE1, 0, 910, 191, 123);

	dd.SetPutRange(TEX_JUDGEEFFECT_JUST, TEXFILE_IMAGE1, 180, 390, 180, 123);


	// 難易度選択ボックス
	dd.SetPutRange(TEX_DIFFBOX_ELEMENTARY, TEXFILE_IMAGE3, 7, 8, 564, 618);
	dd.SetPutRange(TEX_DIFFBOX_INTERMEDIATE, TEXFILE_IMAGE3, 607, 8, 564, 618);
	dd.SetPutRange(TEX_DIFFBOX_ADVANCED, TEXFILE_IMAGE3, 1207, 8, 564, 618);
	dd.SetPutRange(TEX_DIFFBOX_MASTER, TEXFILE_IMAGE3, 7, 658, 564, 618);
	dd.SetPutRange(TEX_DIFFBOX_PROFESSIONAL, TEXFILE_IMAGE3, 607, 658, 564, 618);
	dd.SetPutRange(TEX_DIFFBOX_SHADE, TEXFILE_IMAGE3, 1207, 658, 564, 618);

	// ジャケットボックス系
	dd.SetPutRange(TEX_JACKETBOX, TEXFILE_IMAGE4, 15, 11, 514, 672);
	dd.SetPutRange(TEX_SCOREBOX, TEXFILE_IMAGE4, 553, 14, 1088, 537);
	dd.SetPutRange(TEX_BIGJACKETBOX, TEXFILE_IMAGE4, 12, 714, 1698, 590);
	dd.SetPutRange(TEX_RESULTJACKETBOX, TEXFILE_IMAGE4, 13, 1929, 514, 766);
	dd.SetPutRange(TEX_JACKETBOX_SHADE, TEXFILE_IMAGE4, 1166, 1952, 514, 672);
	dd.SetPutRange(TEX_BIGJACKETBOX_SHADE, TEXFILE_IMAGE4, 8, 1326, 1698, 590);
	dd.SetPutRange(TEX_RESULTJACKETBOX_SHADE, TEXFILE_IMAGE4, 626, 1946, 514, 766);

	// 波紋
	dd.SetPutRange(TEX_RIPPLEGLAY, TEXFILE_IMAGE0, 336, 35, 399, 399);
	dd.SetPutRange(TEX_RIPPLEWHITE, TEXFILE_IMAGE0, 1152, 60, 600, 600);

	// 背景画像
	dd.SetPutRange(TEX_BACKGND_MUSICSELECTION, TEXFILE_IMAGE5, 0, 0, 1920, 1080);
	dd.SetPutRange(TEX_BACKGND_TITLE, TEXFILE_IMAGE7, 0, 0, 1920, 1080);
	dd.SetPutRange(TEX_LOGO_AMAOTO, TEXFILE_IMAGE6, 14, 202, 1561, 183);
	dd.SetPutRange(TEX_LOGO_AMATSUBU, TEXFILE_IMAGE6, 1661, 122, 191, 302);
	dd.SetPutRange(TEX_BACKGND_WHITE, TEXFILE_IMAGE8, 0, 0, 1920, 1080);

	dd.SetPutRange(TEX_BUTTONEFFECT, TEXFILE_IMAGE0, 586, 536, 439, 168);


	ShowWindow(win.hWnd, SW_SHOW);


	ds.AddSound(SND_AMAOTO, "FILES/water-drop3.wav");


	//// 無音サウンドのロード
	//ds.AddSound(CDSPRO_MAXSOUND-1, "FILES\\NULL.WAV");
	//// ループ再生してデバイスを活性化
	//ds.Play(CDSPRO_MAXSOUND-1, TRUE);


	// マシンの周波数を取得　　　　　　　
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	llGlobalFreq = freq.QuadPart;

	// タイマー初期化
	tm.Start(60);

	srand((unsigned int)time(NULL));

	return TRUE;
}


///////////////////////////////////////////////////////
// ロード済みデータの全開放
///////////////////////////////////////////////////////
BOOL CGame::Clear(void) {
	int i;

	ds.Clear();
	dd.Clear();
	mi.Clear();
	for (i=0; i<MAXMUSICCNT; i++) {
		bms[i].ClearAll();
	}

	return TRUE;
}


//////////////////////////////////////////////////////////
// 音楽ゲームの初期化
//////////////////////////////////////////////////////////
BOOL CGame::InitGame() {
	int i, j;

	// ヘッダーデータを全ロード
	if (!LoadAllBmsHeader("LoadScoreFile.txt")) {
		//DEBUG("譜面データの読み込みに失敗しました。\n");
		return FALSE;
	}

	/////////////////////////////////////////////////////
	// 表示する文字列の登録

	dtc.Create(72, 500, L"Century Gothic", false);
	dtcs.Create(30, 500, L"Century Gothic", false);
	dts.Create(14, 500, L"Microsoft Sans Serif", false);

	char buf[MAX_PATH];
	char buf2[MAX_PATH];
	int cnt;

	FONTSTATUS fs;
	dty.SetFontStatus(&fs, 64, L"游ゴシック", 900, false);

	for (i=0; i<iLoadMusicCnt; i++) {
		LPBMSHEADER h = bms[i].GetHeader();

		// 曲情報文字登録
		dty.SetString(TEXT_MUSICTITLE00+i, &fs, "%s", h->mTitle);
		dty.SetString(TEXT_MUSICARTIST00+i, &fs, "%s", h->mArtist);
		dty.SetString(TEXT_MUSICSUBTITLEA00+i, &fs, "%s", h->mSubTitle[0]);
		dty.SetString(TEXT_MUSICSUBTITLEB00+i, &fs, "%s", h->mSubTitle[1]);

		// ジャケット写真のロード
		ZeroMemory(buf, sizeof(buf));
		ZeroMemory(buf, sizeof(buf2));
		cnt = 0;
		for (j=0; j<strlen(h->mDataFilePath); j++) {
			if (h->mDataFilePath[j] == '/') cnt++;
			if (cnt == 2) break;
		}
		strncpy(buf, h->mDataFilePath, j);		// 譜面データのあるディレクトリのパスを生成

		dd.DrawBegin(FALSE);				// 描画キャッシュ用に描画を開始する(クリアはせずに前回描画された状態を残す)
		if (strlen(h->mJacket)>0) {
			// 画像ファイル名の生成
			sprintf(buf2, "%s/%s", buf, h->mJacket);
			// テクスチャロード
			dd.AddTexture(i+TEXFILE_JACKET00, buf2);
			// テクスチャクラスを取得
			CDDTexPro90 *tex = dd.GetTexClass(i+TEXFILE_JACKET00);
			// 画像のサイズで切り抜き
			dd.SetPutRange(TEX_JACKET00+i, i+TEXFILE_JACKET00, 0, 0, tex->GetWidth(), tex->GetHeight());
			// 描画キャッシュ
			dd.SetPutStatus(TEX_JACKET00+i, 0.0f);		// 透明にする
			dd.Put(TEX_JACKET00+i, 0, 0);
			dd.SetPutStatus(TEX_JACKET00+i, 1.0f);		// 元に戻す
		}
		dd.DrawEnd();						// 描画キャッシュ終了(透明で描画したので実際には前回の描画画像のまま)

	}


	// midiキーボード入力受付開始
	mi.StartInput();



	return TRUE;
}



///////////////////////////////////////////////////////
// 演奏モード初期化関数
///////////////////////////////////////////////////////
BOOL CGame::InitPlayMusicMode(LONG musicID, DIFFICULTY diff) {
	char diffStr[13];
	switch (diff) {
		case DIFF_ELEMENTARY: strcpy(diffStr, "ELEMENTARY"); break;
		case DIFF_INTERMEDIATE: strcpy(diffStr, "INTERMEDIATE"); break;
		case DIFF_ADVANCED: strcpy(diffStr, "ADVANCED"); break;
		case DIFF_MASTER: strcpy(diffStr, "MASTER"); break;
		case DIFF_NONE: return FALSE;
	}


	// BMSロード
	LPBMSHEADER h = bms[musicID].GetHeader();
	if (!bms[musicID].LoadObjeData(diff)) {
		DEBUG("\"%s\" obje data load ...failed\n", h->mDataFilePath);
		return FALSE;
	}


	// サウンドデータのロード
	int i;
	char buf[MAX_PATH];
	char buf2[MAX_PATH];
	WCHAR buf3[MAX_PATH];
	int cnt=0;
	ZeroMemory(buf, sizeof(buf));
	ZeroMemory(buf2, sizeof(buf2));
	for (i=0; i<strlen(h->mDataFilePath); i++) {
		if (h->mDataFilePath[i] == '/') cnt++;
		if (cnt == 2) break;
	}
	strncpy(buf, h->mDataFilePath, i);		// 譜面データのあるディレクトリのパスを生成

	for (i=0; i<BMS_MAXINDEX; i++) {
		// WAVファイル名が存在するなら
		if (strlen(bms[musicID].GetWavFile(i))>0) {
			// テクスチャファイル名の生成
			sprintf(buf2, "%s/%s/%s", buf, diffStr, bms[musicID].GetWavFile(i));
			// サウンドロード
			ds.AddSound(i, buf2);
			// 再生してキャッシング
			ds.SetVolume(i, 0);		// 音量を0にする
			ds.Play(i);
			Sleep(1);
			ds.Stop(i);
			ds.SetVolume(i, 1);		// 音量を戻す
		}
	}


	// 背景用動画/画像ファイルの読み込み
	setlocale(LC_CTYPE, "japanese");
	const D3DPRESENT_PARAMETERS *param = dd.GetD3DPRESENT_PARAMETERS();
	for (i=0; i<BMS_MAXINDEX; i++) {
		ZeroMemory(buf2, sizeof(buf2));
		BACKMEDIA *media = bms[musicID].GetBackMedia(i);
		if (media->type == BKMEDIATYPE_NONE) continue;

		sprintf(buf2, "%s/%s", buf, media->mFileName);
		if (media->type == BKMEDIATYPE_IMAGE) {
			if (!dd.AddTexture(TEXFILE_BKGNDIMAGE00+i, buf2)) {
				// ロードエラー時
				bms[musicID].ResetBkmediaType(i);
			}
			else {
				CDDTexPro90 *tex = dd.GetTexClass(TEXFILE_BKGNDIMAGE00+i);
				dd.SetPutRange(TEX_BKGND00+i, TEXFILE_BKGNDIMAGE00+i, 0, 0, tex->GetWidth(), tex->GetHeight());

				// 表示倍率の設定
				float scale = 1.0f;
				if (tex->GetWidth()/tex->GetHeight() > param->BackBufferWidth/param->BackBufferHeight) {
					scale = (float)param->BackBufferWidth/(float)tex->GetWidth();
				}
				else {
					scale = (float)param->BackBufferHeight/(float)tex->GetHeight();
				}
				dd.SetPutStatus(TEX_BKGND00+i, 1.0f, scale, 0.0f);
			}
		}
		else if (media->type == BKMEDIATYPE_MOVIE) {
			mbstowcs(buf3, buf2, sizeof(buf3));
			if (!dm.Create(dd.GetD3DDevice(), buf3, FALSE, param->BackBufferWidth, param->BackBufferHeight)) {
				bms[musicID].ResetBkmediaType(i);
			}
			break;
		}
	}

	bIsBackMedia = bms[musicID].CheckIsBackMedia();
	if (!bIsBackMedia) {
		// 背景メディアが存在しないならジャケットを表示

		// 表示倍率の設定
		float scale = 1.0f;
		CDDTexPro90 *tex = dd.GetTexClass(TEXFILE_JACKET00+musicID);
		if (tex->GetWidth()/tex->GetHeight() > param->BackBufferWidth/param->BackBufferHeight) {
			scale = (float)param->BackBufferWidth/(float)tex->GetWidth();
		}
		else {
			scale = (float)param->BackBufferHeight/(float)tex->GetHeight();
		}
		dd.SetPutStatus(TEX_JACKET00+musicID, 0.5f, scale, 0.0f);
	}


	// スコア計算の準備
	mg.InitCalcPlayStatus(bms[musicID].GetBmsNoteJudgeCnt(), TRUE);

	// ゲーム用変数の初期化
	fScrMulti		= 1.0f;
	dElapsedTime	= 0;
	iBackMediaIndex	= -1;
	ZeroMemory(&iStartNum, sizeof(iStartNum));
	ZeroMemory(&bOnVirtualKey, sizeof(bOnVirtualKey));
	ZeroMemory(&iNoteEffectIndex, sizeof(iNoteEffectIndex));
	ZeroMemory(&iNoteEffectCount, sizeof(iNoteEffectCount));
	ZeroMemory(&dGrooveRateCoef, sizeof(dGrooveRateCoef));
	for (int i = 0; i < BMS_MAXNOTELANE; i++)
		dGrooveRateCoef[i] = 1.0;
	ZeroMemory(&lHoldNote, sizeof(lHoldNote));
	for (i=0; i<VIRTUALKEYCNT; i++)
		lHoldNote[i] = -1;

	// 背景色を黒に設定
	dd.SetBackColor(0x000000);

	return TRUE;
}

BOOL CGame::InitTitle() {

	// 波紋エフェクトセット
	iRippleEffectIndex = 0;
	ZeroMemory(iRippleEffectCount, sizeof(iRippleEffectCount));

	iRippleEffectCount[0].count	= RIPPLEEFFECTCNT;
	iRippleEffectCount[0].x		= 1420;
	iRippleEffectCount[0].y		= 600;
	iRippleEffectCount[1].count	= RIPPLEEFFECTCNT/1.5;
	iRippleEffectCount[1].x		= 1420;
	iRippleEffectCount[1].y		= 600;
	iRippleEffectCount[2].count	= RIPPLEEFFECTCNT/2;
	iRippleEffectCount[2].x		= 1420;
	iRippleEffectCount[2].y		= 600;
	iRippleEffectIndex = 3;

	/*for (int i=3; i<ELEMENTSIZE(iRippleEffectCount)/5; i++) {
		iRippleEffectCount[iRippleEffectIndex].count	= RIPPLEEFFECTCNT;
		iRippleEffectCount[iRippleEffectIndex].x		= rand()%2000;
		iRippleEffectCount[iRippleEffectIndex].y		= rand()%1200;
		iRippleEffectIndex++;
		if (iRippleEffectIndex == ELEMENTSIZE(iRippleEffectCount)) {
			iRippleEffectIndex = 0;
		}
	}*/

	dd.SetBackColor(0xffffff);

	bFlag = TRUE;

	return TRUE;
}

int CGame::RunTitle() {
	int i, j, k;

	// 開始時から経過した時間を算出
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	dElapsedTime = (double)(li.QuadPart - llStartTime) / llGlobalFreq;


	///////////////////////////////////////////////////////////////////////////////////////
	// 入力処理関係
	///////////////////////////////////////////////////////////////////////////////////////
	// キーボード入力
	BYTE key[256];
	di.GetKeyboard(key);

	// midiキーボード入力
	MIDIKEYSTATE midiKey[MIDIKEYCNT];
	BOOL getMidiKeyFlag = mi.GetKeyState(midiKey);

	// 仮想入力キーへ変換
	BOOL pressVirtualKey[VIRTUALKEYCNT];
	ZeroMemory(pressVirtualKey, sizeof(pressVirtualKey));
	if (dElapsedTime > 0.5) {
		InputVirtualKey(pressVirtualKey, midiKey, key, getMidiKeyFlag);
	}

	// ESCキーで終了
	if (key[DIK_Q]&0x80) return -1;


	// カーソル処理
	if (pressVirtualKey[0] || pressVirtualKey[1] ||
		pressVirtualKey[2] || pressVirtualKey[3] ||
		pressVirtualKey[4] || pressVirtualKey[5] ||
		pressVirtualKey[6] || pressVirtualKey[7] ||
		pressVirtualKey[8] || pressVirtualKey[9] ||
		pressVirtualKey[10] || pressVirtualKey[11] ||
		pressVirtualKey[12] || pressVirtualKey[13] ||
		pressVirtualKey[14] || pressVirtualKey[15] ||
		pressVirtualKey[16] || pressVirtualKey[17] ||
		pressVirtualKey[18] || pressVirtualKey[19] ||
		pressVirtualKey[20] || pressVirtualKey[21] ||
		pressVirtualKey[22] || pressVirtualKey[23] ||
		pressVirtualKey[24]) {
		// タイトル画面終了

		return 0;
	}


	int lp = tm.Run();
	if (dElapsedTime > 3.0) {
		// 60FPSでのデータ操作
		for (k=0; k<lp; k++) {
			for (i=0; i<ELEMENTSIZE(iRippleEffectCount); i++) {
				if (iRippleEffectCount[i].count > 0)
					iRippleEffectCount[i].count -= 1;
			}
		}

		// 波紋エフェクトの処理
		if (rand()%10 == 0) {
			iRippleEffectCount[iRippleEffectIndex].count	= RIPPLEEFFECTCNT;
			iRippleEffectCount[iRippleEffectIndex].x		= rand()%2000 - 20;
			iRippleEffectCount[iRippleEffectIndex].y		= rand()%1200 - 20;
			iRippleEffectIndex++;
			if (iRippleEffectIndex == ELEMENTSIZE(iRippleEffectCount)) {
				iRippleEffectIndex = 0;
			}
		}
	}


	// 効果音再生
	if (dElapsedTime > 3.0 && bFlag) {
		ds.SetVolume(SND_AMAOTO, 1.0f);
		ds.Reset(SND_AMAOTO);
		ds.Play(SND_AMAOTO, 0);
		bFlag = FALSE;
	}



	////////////////////////////////////////////////////////////////////////////////////
	// デバイスロストチェック(フルスクリーン時にALT+TABを押した場合など)
	// ※復帰時は内部で管理しているテクスチャは自動的にリストアされるが、
	//   MANAGEDではない頂点バッファやテクスチャを使用している場合は、
	//   自分でロスト＆リストア処理を行う
	////////////////////////////////////////////////////////////////////////////////////
	if (!dd.CheckDevice()) {
		// ロスト中なら
		if (!bLostDevice) {
			// ロスト直後ならここで開放処理を行う
			DEBUG("デバイスがロストした\n");

			bLostDevice = TRUE;
		}

		// 描画せずに抜ける
		return 1;
	}

	if (bLostDevice) {
		// リストア直後ならここで再構築を行う
		DEBUG("リストアされた\n");

		bLostDevice = FALSE;
	}



	//////////////////////////////////////////////////////
	// 描画処理
	//////////////////////////////////////////////////////
	dd.DrawBegin();

	// 背景
	float alpha = (float)((dElapsedTime - 4.1)*2);
	if (alpha < 0.0f) alpha = 0.0f;
	else if (alpha > 1.0f) alpha = 1.0f;
	dd.SetPutStatus(TEX_BACKGND_TITLE, alpha, 1.0f, 0.0f);
	dd.Put(TEX_BACKGND_TITLE, 0, 0);


	// 波紋
	if (dElapsedTime > 3.0) {
		for (i=0; i<ELEMENTSIZE(iRippleEffectCount); i++) {
			if (iRippleEffectCount[i].count > 0) {
				dd.SetPutStatus(TEX_RIPPLEGLAY, (float)iRippleEffectCount[i].count/RIPPLEEFFECTCNT, (RIPPLEEFFECTCNT - iRippleEffectCount[i].count)/(float)JUDGEEFFECTCNT, 0.0f);
				dd.Put2(TEX_RIPPLEGLAY, iRippleEffectCount[i].x, iRippleEffectCount[i].y);
			}
		}
	}

	// 雨粒
	alpha = (float)((dElapsedTime - 2.9)*3);
	if (alpha < 0.0f) alpha = 0.0f;
	else if (alpha > 1.0f) alpha = 1.0f;
	dd.SetPutStatus(TEX_LOGO_AMATSUBU, alpha, 0.5f, 0.0f);
	dd.Put2(TEX_LOGO_AMATSUBU, 1420, 540);



	// タイトル文字
	dd.SetPutStatus(TEX_LOGO_AMAOTO, 0.9f, 0.5f, 0.0f);
	dd.Put2(TEX_LOGO_AMAOTO, 860, 540);


	if (dElapsedTime > 5.0) {
		RECT textArea;
		textArea.left = 710;
		textArea.top  = 900;
		textArea.right= 1210;
		textArea.bottom = 1000;
		dtc.DrawInRect(&textArea, 56, 5, TEXTALIGN_CENTERXY|TEXTSCALE_AUTOXY, dtc.ConvertFromRGBA(0, 0, 0, (int)((dElapsedTime-5.0)*100)%200+50), "Press the button");
	}



	//dtc.Draw(100, 100, 32, 0, 0xff000000, "%f", dElapsedTime);
	/*dtc.Draw(0, 130, 32, 0, 0xffff0000, "pressed");
	dtc.Draw(0, 160, 32, 0, 0xffff0000, "hold");

	for (i=0; i<VIRTUALKEYCNT; i++) {
		dtc.Draw(i*60+20+100, 100, 32, 0, 0xffff0000, "%d", i);
		if (pressVirtualKey[i]) dtc.Draw(i*60+100, 130, 32, 0, 0xffff0000, "ON");
		else dtc.Draw(i*60+100, 130, 32, 0, 0x3f000000, "OFF");

		if (bOnVirtualKey[i]) dtc.Draw(i*60+100, 160, 32, 0, 0xffff0000, "ON");
		else dtc.Draw(i*60+100, 160, 32, 0, 0x3f000000, "OFF");
	}*/


	dd.DrawEnd();

	return 1;
}




///////////////////////////////////////////////////////
// 曲演奏モード実行関数
///////////////////////////////////////////////////////
int CGame::RunPlayMusicMode(LONG musicID, BOOL demo) {

	// 開始時から経過した時間を算出
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	dElapsedTime = (double)(li.QuadPart - llStartTime) / llGlobalFreq;

	// テンポラリ変数
	int i, j, k;



	///////////////////////////////////////////////////////////////////////////////////////
	// 入力処理関係
	///////////////////////////////////////////////////////////////////////////////////////
	// キーボード入力
	BYTE key[256];
	di.GetKeyboard(key);

	// midiキーボード入力
	MIDIKEYSTATE midiKey[MIDIKEYCNT];
	BOOL getMidiKeyFlag = mi.GetKeyState(midiKey);

	// 仮想入力キーへ変換
	BOOL pressVirtualKey[VIRTUALKEYCNT];
	ZeroMemory(pressVirtualKey, sizeof(pressVirtualKey));
	InputVirtualKey(pressVirtualKey, midiKey, key, getMidiKeyFlag);

	// ESCキーで終了
	if (key[DIK_Q]&0x80) return -1;


	// 60FPSでのデータ操作
	int lp = tm.Run();
	for (k=0; k<lp; k++) {
		// フラッシュ部
		for (j=0; j<BMS_MAXNOTELANE; j++) {
			for (i=0; i<30; i++) {
				if (iNoteEffectCount[j][i]>0)
					iNoteEffectCount[j][i] -= 1;
			}
		}

		//// スクロール幅変更
		//if (key[DIK_ADD]&0x80) {
		//	// テンキーの＋
		//	fScrMulti += 1/fScrMulti;
		//	if (fScrMulti > 10.0f)
		//		fScrMulti = 10.0f;
		//}
		//if (key[DIK_SUBTRACT]&0x80) {
		//	// テンキーの－
		//	fScrMulti -= 1/fScrMulti;
		//	if (fScrMulti < 0.2f)
		//		fScrMulti = 0.2f;
		//}
	}

	// 経過した時間から進んだBMSカウント値を算出
	LONG now_count = bms[musicID].GetCountFromTime(dElapsedTime);

	// BMSカウンタが曲の最大カウント+1小節を超えたら終了
	if (bms[musicID].GetMaxCount()+BMS_RESOLUTION<=now_count) {
		//mt.Stop();
		return 1;
	}

	// BGMをタイミングにあわせて再生する
	BMSHEADER *h = bms[musicID].GetHeader();
	for (i=iStartNum[BMS_BACKMUSIC]; i<bms[musicID].GetObjeNum(BMS_BACKMUSIC); i++) {
		LPBMSDATA b = bms[musicID].GetObje(BMS_BACKMUSIC, i);
		if (now_count<b->lTime)
			break;
		if (b->bFlag) {
			if (now_count>=b->lTime) {
				b->bFlag = FALSE;
				ds.SetVolume(b->lData, h->fWavVol);
				ds.Reset(b->lData);
				ds.Play(b->lData);
				iStartNum[BMS_BACKMUSIC] = i + 1;
			}
		}
	}


	// 背景の描画処理
	if (bIsBackMedia) {
		// 背景メディアが存在するなら

		for (i=iStartNum[BMS_BACKMEDIA]; i<bms[musicID].GetObjeNum(BMS_BACKMEDIA); i++) {
			LPBMSDATA b = bms[musicID].GetObje(BMS_BACKMEDIA, i);
			if (b->bFlag && now_count > b->lTime) {
				LPBACKMEDIA m = bms[musicID].GetBackMedia(b->lData);

				switch (m->type) {
					case BKMEDIATYPE_IMAGE:
						iBackMediaIndex = b->lData;
						break;
					case BKMEDIATYPE_MOVIE:
						iBackMediaIndex = b->lData;
						dm.SetTime(0.0);
						dm.Play();
						break;
				}

				b->bFlag = FALSE;
				iStartNum[BMS_BACKMEDIA] = i+1;
			}
		}
	}


	//// アニメーション画像のセット
	//for( i=iStartNum[BMS_BACKANIME];i<bms[musicID].GetObjeNum(BMS_BACKANIME);i++ ) {
	//	LPBMSDATA b = bms[musicID].GetObje( BMS_BACKANIME,i );
	//	if( b->bFlag ) {
	//		if( now_count>=b->lTime ) {
	//			b->bFlag = FALSE;
	//			iImageID = b->lData;
	//			iStartNum[BMS_BACKANIME] = i + 1;
	//		}
	//	}
	//}


	double td = 0.0, ts = 0.0f;

	if (demo) {
		/////////////////////////////////////////////////////////////////////////////////////
		// コンピュータプレイ
		/////////////////////////////////////////////////////////////////////////////////////

		for (i=iStartNum[BMS_NOTE]; i < bms[musicID].GetBmsNoteCnt(); i++) {
			LPBMSNOTE b = bms[musicID].GetBmsNote(i);
			LPNOTEDEFDATA n = bms[musicID].GetNoteDefData(b->lIndex);

			if (now_count < b->lTime1)
				break;

			if (b->bFlag1) {
				if (now_count >= b->lTime1) {
					// 当たり
					b->bFlag1 = FALSE;
					// そのオブジェの音を再生
					ds.SetVolume(n->lSound, n->fVol);
					ds.Reset(n->lSound);
					ds.Play(n->lSound);
					//判定結果を保存
					mg.SaveJudgeResult(now_count, -1, i, JUST, b->iLane);
					// ノートエフェクト開始
					iNoteEffectCount[b->iLane][iNoteEffectIndex[b->iLane]] = JUDGEEFFECTCNT;

					// 次のインデックスへ
					iNoteEffectIndex[b->iLane]++;
					if (iNoteEffectIndex[b->iLane] >= 30)
						iNoteEffectIndex[b->iLane] = 0;


					if (b->eType == NOTETYPE::HOLD)
						lHoldNote[0]++;

					if (lHoldNote[0] < 0)
						iStartNum[BMS_NOTE] = i + 1;	// ホールドノートの判定中は更新しない
				}
			}
			else if (b->bFlag2) {
				// ホールド終点ノート
				if (now_count >= b->lTime2) {
					// 当たり
					b->bFlag2 = FALSE;
					lHoldNote[0]--;
					//判定結果を保存
					mg.SaveJudgeResult(now_count, -1, i, JUST, b->iLane);
					// ノートエフェクト開始
					iNoteEffectCount[b->iLane][iNoteEffectIndex[b->iLane]] = JUDGEEFFECTCNT;

					// 次のインデックスへ
					iNoteEffectIndex[b->iLane]++;
					if (iNoteEffectIndex[b->iLane] >= 30)
						iNoteEffectIndex[b->iLane] = 0;

					if (lHoldNote[0] < 0)
						iStartNum[BMS_NOTE] = i + 1;	// ホールドノートの判定中は更新しない
				}
			}

		}

	}
	else {
		/////////////////////////////////////////////////////////////////////////////////////
		// 人間プレイ
		/////////////////////////////////////////////////////////////////////////////////////

		// 入力判定
		float nowBpm = h->fBpm;	// 現在のbpmを取得
		for (int i=bms[musicID].GetObjeNum(BMS_BPMINDEX)-1; i>=0; i--) {
			LPBMSDATA b = bms[musicID].GetObje(BMS_BPMINDEX, i);
			if (b->lTime < now_count) {
				nowBpm = h->fBpmIndex[b->lData];
				break;
			}
		}

		const LONG JUST_RANGE		= (long)(BMS_RESOLUTION / 36*nowBpm/120);		// Justと判定する中心からの範囲
		const LONG JUSTLE_RANGE		= (long)(BMS_RESOLUTION / 16*nowBpm/120);		// JustLEと判定する中心からの範囲
		const LONG LATEEARLY_RANGE	= (long)(BMS_RESOLUTION / 8*nowBpm/120);		// Late/Earlyと判定する中心からの範囲
		const LONG LATE2EARLY2_RANGE= (long)(BMS_RESOLUTION / 4*nowBpm/120);		// Late2/Early2と判定する中心からの範囲
		const LONG MISS_RANGE		= (long)(BMS_RESOLUTION / 4*nowBpm/120);		// Miss判定する中心からの範囲

		// まずは見逃しmissの処理から
		for (i=iStartNum[BMS_NOTE]; i < bms[musicID].GetBmsNoteCnt(); i++) {
			LPBMSNOTE b = bms[musicID].GetBmsNote(i);

			// オブジェがmiss判定にならないなら抜ける
			if (now_count < (b->lTime1 + LATE2EARLY2_RANGE / 2)) {
				break;
			}
			else {

				if (b->bFlag1) {
					b->bFlag1 = FALSE;						// オブジェを消す
															//判定結果を保存
					mg.SaveJudgeResult(now_count, -1, i, MISS_L, b->iLane);

					if (b->eType == HOLD) {
						// ホールド始点のノートだったら
						b->bFlag2 = FALSE;					// ホールド終点のノートも消す
															// 判定結果を保存
						mg.SaveJudgeResult(now_count, -1, i, HOLDBREAK, b->iLane);
					}

				}
				iStartNum[BMS_NOTE] = i + 1;
				// 次のオブジェをチェック
			}
		}


		// 入力判定の処理
		for (j = 0; j < VIRTUALKEYCNT; j++) {
			// 登録されている全てのキーを判定
			if (lHoldNote[j] >= 0) {
				// ホールドノート判定中なら
				LPBMSNOTE b = bms[musicID].GetBmsNote(lHoldNote[j]);
				LPNOTEDEFDATA n = bms[musicID].GetNoteDefData(b->lIndex);

				LONG sub = now_count - b->lTime2;		// ノートとの差を計算 (負の値は入力が基準より早かったことを表す
				JUDGE judge;
				if (sub < -LATEEARLY_RANGE*3 / 2) {
					judge = HOLDBREAK;						// 早ミス
				}
				else if (sub < -JUSTLE_RANGE*3 / 2) {
					judge = EARLY2;							// EARLY2
				}
				else if (sub < -JUST_RANGE*3 / 2) {
					judge = EARLY;							// EARLY
				}
				else if (sub < 0) {
					judge = JUST_E;							// JUST E
				}
				else if (sub >= 0) {
					judge = JUST;							// JUST
				}

				if (judge == JUST || !bOnVirtualKey[j]) {
					mg.SaveJudgeResult(now_count, j, lHoldNote[j], judge, b->iLane);		// 判定結果を保存

					lHoldNote[j] = -1;					// ホールド判定中のフラグを外す
					b->bFlag2 = FALSE;					// ホールド終点のノートも消す
					if (judge != JUST)
						ds.Stop(n->lSound);				// 音を停止

					if (judge != HOLDBREAK) {
						// ノートエフェクト開始
						iNoteEffectCount[b->iLane][iNoteEffectIndex[b->iLane]] = JUDGEEFFECTCNT;
						// 次のインデックスへ
						iNoteEffectIndex[b->iLane]++;
						if (iNoteEffectIndex[b->iLane] >= 30)
							iNoteEffectIndex[b->iLane] = 0;
					}
				}
				else {
					// ホールド中の処理

				}
			}
			else if (pressVirtualKey[j]) {
				// キーが押された瞬間なら

				// -----タイミングの同じノートが来た時に、押したキーに一番近い位置のノートを選択する-----
				bool keyFlag = false;
				int noteNum;
				float gapMin = 1.00f;
				LONG time;
				k = 0;

				for (i = iStartNum[BMS_NOTE]; i < bms[musicID].GetBmsNoteCnt(); i++) {
					LPBMSNOTE b = bms[musicID].GetBmsNote(i);

					if (!b->bFlag1)
						continue;	// ノートの判定処理が既に終わっていたら、次のノートをチェック
					if (now_count < (b->lTime1 - MISS_RANGE / 2))
						break;		// ノートが判定外なら探索終了
					if (k >= 1 && b->lTime1 != time)
						break;			// タイミングのズレたノートなら探索終了

					// ノートが判定範囲なら
					float gap = fabsf(bms[musicID].GetLane(b->iLane)->fBottomX - ((double)(j)/(MIDIKEYCNT-1)));	// キーとノートの位置のズレを求める
					if (gap <= 0.15f) {
						// キーとノートの位置のズレが許容範囲なら
						k++;
						keyFlag = true;		// 正しいキー入力とみなす

						if (gap < gapMin) {
							gapMin = gap;						// ズレの最小値を保存
							noteNum = i;						// ズレが一番小さいノート番号を保存
							time = b->lTime1;					// ズレが一番小さいノートのbmsカウントを保存
						}
					}
				}
				// -----------------------------------------------------------------------------------


				if (keyFlag) {
					// キーを押した瞬間なら精度判定

					// -------判定範囲に存在するノートの中で良い判定のノートを探索する-------
					int laneNum = bms[musicID].GetBmsNote(noteNum)->iLane;	// 探索するノートレーン番号の取得

					JUDGE best;
					JUDGE judge;
					LONG sub;
					LONG sub2;
					int num[20];
					int cnt = 0;
					for (i = noteNum; i< bms[musicID].GetBmsNoteCnt(); i++) {
						LPBMSNOTE b = bms[musicID].GetBmsNote(i);
						LPNOTEDEFDATA n = bms[musicID].GetNoteDefData(b->lIndex);
						if (!b->bFlag1)
							continue;	// ノートの判定処理が既に終わっていたら、次のノートをチェック
						if (now_count < (b->lTime1 - MISS_RANGE / 2))
							break;		// 判定外なら探索終了
						if (b->iLane != laneNum)
							continue;	// 探索するノートレーンでない場合はスキップ


						sub = now_count - b->lTime1;		// オブジェとの差を計算 (負の値は入力が基準より早かったことを表す
						if (sub < -LATE2EARLY2_RANGE / 2) {
							judge = MISS_E;							// 早ミス
						}
						else if (sub < -LATEEARLY_RANGE / 2) {
							judge = EARLY2;							// EARLY2
						}
						else if (sub < -JUSTLE_RANGE / 2) {
							judge = EARLY;							// EARLY
						}
						else if (sub < -JUST_RANGE / 2) {
							judge = JUST_E;							// JUST E
						}
						else if (sub <= JUST_RANGE / 2) {
							judge = JUST;							// JUST
						}
						else if (sub <= JUSTLE_RANGE / 2) {
							judge = JUST_L;							// JUST L
						}
						else if (sub <= LATEEARLY_RANGE / 2) {
							judge = LATE;							// LATE
						}
						else if (sub <= LATE2EARLY2_RANGE / 2) {
							judge = LATE2;							// LATE2
						}
						else if (sub <= MISS_RANGE / 2) {
							judge = MISS_L;							// 遅ミス
						}


						if (JUST_L == judge || judge == JUST) {
							// 判定がjustL or justのとき
							best = judge;	// 判定結果を更新	
							noteNum = i;

							// 追い越したノートを判定済みにする
							while (--cnt >= 0) {
								LPBMSNOTE b = bms[musicID].GetBmsNote(num[cnt]);
								b->bFlag1 = FALSE;			// 判定済みとする
															//判定結果を保存
								mg.SaveJudgeResult(now_count, j, num[cnt], MISS_L, b->iLane);
							}

							break;
						}

						if (i == noteNum) {
							// 初回ループ時
							best = judge;	// 判定結果を保存
							sub2 = sub;		// 今回のズレを保存
						}
						else {
							// 2回目ループ以降
							if (abs(sub) > abs(sub2)) {
								// 前のノーツズレより今回のノーツのズレの方が大きいなら
								break;
							}
						}

						num[cnt++] = i;
					}
					// ---------------------------------------------------------------------------

					LPBMSNOTE b = bms[musicID].GetBmsNote(noteNum);
					LPNOTEDEFDATA n = bms[musicID].GetNoteDefData(b->lIndex);

					if (b->eType == NOTETYPE::HOLD && best == MISS_E)
						continue;	// HOLDノートは早ミスにならない

									// オブジェを消す
					b->bFlag1 = FALSE;
					// そのオブジェの音を再生
					ds.SetVolume(n->lSound, n->fVol);
					ds.Reset(n->lSound);
					ds.Play(n->lSound);
					//判定結果を保存
					mg.SaveJudgeResult(now_count, j, noteNum, best, b->iLane);

					if (b->eType == NOTETYPE::HOLD) {
						// ホールド始点ノートIDを保存
						lHoldNote[j] = noteNum;
					}

					if (best != MISS_E && best != MISS_L) {
						// ノートエフェクト開始
						iNoteEffectCount[b->iLane][iNoteEffectIndex[b->iLane]] = JUDGEEFFECTCNT;
						// 次のインデックスへ
						iNoteEffectIndex[b->iLane]++;
						if (iNoteEffectIndex[b->iLane] >= 30)
							iNoteEffectIndex[b->iLane] = 0;
					}
				}
			}
		}


	}




	////////////////////////////////////////////////////////////////////////////////////
	// デバイスロストチェック(フルスクリーン時にALT+TABを押した場合など)
	// ※復帰時は内部で管理しているテクスチャは自動的にリストアされるが、
	//   MANAGEDではない頂点バッファやテクスチャを使用している場合は、
	//   自分でロスト＆リストア処理を行う
	////////////////////////////////////////////////////////////////////////////////////
	if (!dd.CheckDevice()) {
		// ロスト中なら
		if (!bLostDevice) {
			// ロスト直後ならここで開放処理を行う
			DEBUG("デバイスがロストした\n");

			bLostDevice = TRUE;
		}

		// 描画せずに抜ける
		return 1;
	}

	if (bLostDevice) {
		// リストア直後ならここで再構築を行う
		DEBUG("リストアされた\n");

		bLostDevice = FALSE;
	}





	////////////////////////////////////////////////////////////////////////////////////
	// 描画処理
	////////////////////////////////////////////////////////////////////////////////////
	dd.DrawBegin();


	// 背景の表示
	const D3DPRESENT_PARAMETERS *param = dd.GetD3DPRESENT_PARAMETERS();
	if (bIsBackMedia) {
		// 背景メディアが存在するなら

		if (iBackMediaIndex >= 0) {
			RECT rect ={ 0, 0, param->BackBufferWidth, param->BackBufferHeight };
			LPBACKMEDIA m = bms[musicID].GetBackMedia(iBackMediaIndex);
			switch (m->type) {
				case BKMEDIATYPE_IMAGE:
					dd.Put2(TEX_BKGND00+iBackMediaIndex, param->BackBufferWidth/2.0f, param->BackBufferHeight/2.0f);
					break;
				case BKMEDIATYPE_MOVIE:
					dm.DrawMovie(rect, DRAWMOVOPT_ASPECTFIXED);
					break;
			}
		}
	}
	else {
		//dd.Put2(TEX_JACKET00+musicID, param->BackBufferWidth/2.0f, param->BackBufferHeight/2.0f);
	}


	// ボタンエフェクト
	dd.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);			// 加算合成
	for (int i=0; i<VIRTUALKEYCNT; i++) {
		if (bOnVirtualKey[i]) {
			dd.SetPutStatus(TEX_BUTTONEFFECT, 1.0f, 0.9f, 0.0f);
			dd.Put2(TEX_BUTTONEFFECT, i*1920/(VIRTUALKEYCNT-1), 888);
		}
	}



	// 判定ラインの表示
	dd.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);	// 通常合成
	dd.SetPutStatus(0, 1.0f, 1.0f, 0.0f);
	dd.Put2(TEX_JUDGELINE, 960, 960);

	// 判定ラインのエフェクト
	float cnt = (float)(now_count%(BMS_RESOLUTION/4))/(BMS_RESOLUTION/4);		// 現在の時間が4分音符間のどの位置にあたるか計算(0.0<= cnt <1.0)
	dd.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);			// 加算合成
	dd.SetPutStatus(TEX_JUDGELINEEFFECT, (float)(1.0+sin(M_PI+ cnt/2* M_PI)/2.0), 1.0f, 0.0f);
	dd.Put2(TEX_JUDGELINEEFFECT, 960, 961);


	// ノート＆ノートレーン
	bool flag[BMS_MAXNOTELANE] ={ false };
	BOOL bJudgeHoldNote = FALSE;			// ホールドノートが現在判定中か調べる
	for (i=0; i<VIRTUALKEYCNT; i++) {
		if (lHoldNote[i] >= 0) {
			bJudgeHoldNote = TRUE;
			break;
		}
	}

	dd.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);	// 通常合成
	for (i=iStartNum[BMS_MAXINDEX]; i < bms[musicID].GetBmsNoteCnt(); i++) {
		LPBMSNOTE b = bms[musicID].GetBmsNote(i);
		LPNOTEDEFDATA n = bms[musicID].GetNoteDefData(b->lIndex);
		LPLANE l = bms[musicID].GetLane(b->iLane);
		float off_y = (float)((double)(b->lTime1 - now_count) / BMS_RESOLUTION * (fScrMulti * 576));
		float off_x = (float)(((l->fTopX-l->fBottomX)*1920/967.0) *off_y);
		float off_y2 = (float)((double)(b->lTime2 - now_count) / BMS_RESOLUTION * (fScrMulti * 576));
		float off_x2 = (float)(((l->fTopX-l->fBottomX)*1920/967.0) *off_y2);

		if (off_y2 < -120) {
			// ノートが画面外へ出たら、次回はその次のノートから描画開始する
			if (!bJudgeHoldNote) {
				// ホールドノート判定中の場合は更新しない
				iStartNum[BMS_MAXINDEX] = i+1;
			}
		}

		// 判定済みのノートは表示せずスキップ
		if (!b->bFlag1 && !b->bFlag2)
			continue;

		// ヒットノートが画面の上より外ならばその先は全て描画スキップ
		if (off_y > 960 + 20)
			break;

		// ノートレーンの描画
		if (!flag[b->iLane]) {
			df.strokeWidth(1.0f);
			df.stroke(l->iColor[0], l->iColor[1], l->iColor[2], l->iColor[3]);
			df.line(l->fBottomX*1920, 960, l->fTopX*1920, 0);
			RECT area ={ l->fTopX*1920+10, 10, l->fTopX*1920+210, 60 };
			dtc.DrawInRect(&area, 32, 0, TEXTALIGN_LEFT|TEXTSCALE_NONE, dtc.ConvertFromRGBA(l->iColor[0], l->iColor[1], l->iColor[2], l->iColor[3]), "%s", l->mName);
			flag[b->iLane] = TRUE;
		}

		if (b->eType == HOLD) {
			// ホールドノートなら
			if (off_y < 0)
				off_y = 0, off_x = 0;
			df.stroke(0, 100, 255, 200);
			df.strokeWidth(10.0f);
			if (b->bFlag1) {
				// ホールド始点がまだ判定されていないなら
				df.line(l->fBottomX*1920.0f + off_x, 960 + 7 - off_y, l->fBottomX*1920.0f + off_x2, 960 + 7 - off_y2);
				dd.Put2(TEX_NOTE, l->fBottomX*1920.0f + off_x2, 960 + 7 - off_y2);				// ホールド終点ノート表示
				dd.Put2(TEX_NOTE, l->fBottomX*1920.0f + off_x, 960 + 7 - off_y);				// ホールド始点ノート表示
			}
			else {
				// ホールド始点が判定済みなら
				df.line(l->fBottomX*1920.0f, 960+7, l->fBottomX*1920.0f  + off_x2, 960 + 7 - off_y2);
				dd.Put2(TEX_NOTE, l->fBottomX*1920.0f + off_x2, 960 + 7 - off_y2);				// ホールド終点ノート表示
			}

		}
		else {
			dd.Put2(TEX_NOTE, l->fBottomX*1920.0f+ off_x, 960 + 7 - off_y);				// ノート表示
		}
	}


	// 判定文字表示
	for (i=mg.GetSavedNoteCnt()-1; i>=0; i--) {
		LPNOTEJUDGE nj = mg.GetJudgeResult(i);

		cnt = BMS_RESOLUTION/4 - (now_count - nj->lTime);  // 演出用カウント値を求める　(2400 → 0)
		if (cnt > 0) {
			// 1/4小節の間表示
			LPLANE n = bms[musicID].GetLane(nj->iLane);

			dd.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);	// 通常合成
			switch (nj->eJudge) {
				case MISS_E:
				case HOLDBREAK:
					dd.SetPutStatus(TEX_JUDGESTR_MISS, cnt*4/BMS_RESOLUTION, 0.5f, 0.0f);
					dd.Put2(TEX_JUDGESTR_MISS, n->fBottomX*1920.0f, 960-250+ cnt*200/BMS_RESOLUTION);
					break;
				case JUST:
					dd.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);			// 加算合成
					dd.SetPutStatus(TEX_JUDGEEFFECT_JUST, cnt / BMS_RESOLUTION*4, 0.5f, 0.0f);
					dd.Put2(TEX_JUDGEEFFECT_JUST, n->fBottomX*1920.0f, 960 - 250 + cnt* 200/BMS_RESOLUTION);
					dd.SetPutStatus(TEX_JUDGESTR_JUST, cnt / BMS_RESOLUTION * 4, 0.5f, 0.0f);
					dd.Put2(TEX_JUDGESTR_JUST, n->fBottomX*1920.0f, 960 - 250 + cnt * 200/ BMS_RESOLUTION);
					break;
				default:
					dd.SetPutStatus(nj->eJudge + TEX_JUDGESTR_JUST, cnt*4/BMS_RESOLUTION, 0.5f, 0.0f);
					dd.Put2(nj->eJudge + TEX_JUDGESTR_JUST, n->fBottomX*1920.0f, 960 - 250+ cnt / BMS_RESOLUTION * 200);
					break;
			}
		}
		else {
			break;
		}
	}

	// ノートエフェクト
	dd.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);			// 加算合成
	int size;
	int offsetX, offsetY;
	int alpha;
	for (j = 0; j < BMS_MAXNOTELANE; j++) {
		LPLANE n = bms[musicID].GetLane(j);
		if (n->fBottomX == -1.0f && n->fTopX == -1.0f)
			// データが初期値なら
			continue;

		for (i = 0; i < 30; i++) {
			if (iNoteEffectCount[j][i] > 0) {
				// 演出が存在する場合のみ表示
				size = (JUDGEEFFECTCNT-iNoteEffectCount[j][i])*3;
				offsetX = 0;
				offsetY = 0;
				offsetY = 0;

				//alpha = iNoteEffectCount[j][i]*3;
				//*df.noFill();
				//df.strokeWidth(1.0f);
				//df.stroke(255, 255, 255, alpha-50);
				//df.ellipse(n->fBottomX*1920+offsetX, 960+offsetY, size+1, size+1);
				//df.ellipse(n->fBottomX*1920+offsetX, 960+offsetY, size-1, size-1);
				//df.strokeWidth(1.0f);
				//df.stroke(255, 255, 255, alpha);
				//df.ellipse(n->fBottomX*1920+offsetX, 960+offsetY, size, size);*/

				dd.SetPutStatus(TEX_RIPPLEWHITE, (float)iNoteEffectCount[j][i]/JUDGEEFFECTCNT, (float)(JUDGEEFFECTCNT-iNoteEffectCount[j][i])/JUDGEEFFECTCNT, 0.0f);
				dd.Put2(TEX_RIPPLEWHITE, n->fBottomX*1920, 960);

			}
		}
	}


	


	//////////////////////////////////////////////////////////////
	// 演奏状態表示
	//////////////////////////////////////////////////////////////
	double groove;
	double score;
	mg.GetPlayStatus(&score, &groove);

	if (100.0 <= groove && groove < 110.0)
		groove = 100.0;
	else if (groove >= 110.0)
		groove -= 10.0;

	dtc.Draw(10, 100, 30, 0, dtc.ConvertFromRGBA(255, 255, 255, 200), "GROOVE");
	dtc.Draw(127, 92, 40, 0, dtc.ConvertFromRGBA(255, 255, 255, 200), "%5.1f %%", groove);
	df.noFill();
	df.strokeWidth(1.0f);
	df.stroke(255, 255, 255, 150);
	df.rect(10, 80, 500, 14);
	df.fill(255, 255, 255, 150);
	df.noStroke();
	if (groove > 100.0)
		groove = 100.0;
	df.rect(10, 80, groove*5, 12);

	//スコア表示
	dtc.Draw(1530, 103, 28, 0, dtc.ConvertFromRGBA(255, 255, 255, 200), "SCORE");
	dtc.Draw(1630, 85, 60, 4, dtc.ConvertFromRGBA(255, 255, 255, 200), "%7.0f", score);

	// レベル表示
	df.noFill();
	df.stroke(255, 255, 255, 100);
	RECT textArea;
	char str[15];
	switch (bms[musicID].GetStoredDataKind()) {
		case DIFF_ELEMENTARY:	strcpy(str, "Elementary"); break;
		case DIFF_INTERMEDIATE: strcpy(str, "Intermediate"); break;
		case DIFF_ADVANCED:		strcpy(str, "Advanced"); break;
		case DIFF_MASTER:		strcpy(str, "Master"); break;
	}
	textArea.top	= 1040;
	textArea.left	= 1155;
	textArea.right	= 1400;
	textArea.bottom	= 1080;
	//df.rect(textArea.left, textArea.top, textArea.right-textArea.left, textArea.bottom-textArea.top);
	dtc.DrawInRect(&textArea, 38, 0, TEXTALIGN_LEFT|TEXTSCALE_NONE, dtc.ConvertFromRGBA(255, 255, 255, 200), "%s", str);	// 難易度文字列
	textArea.left	= 1400;
	textArea.top	= 1056;
	textArea.right	= 1450;
	//df.rect(textArea.left, textArea.top, textArea.right-textArea.left, textArea.bottom-textArea.top);
	dtcs.DrawInRect(&textArea, 19, 0, TEXTALIGN_LEFT|TEXTSCALE_NONE, dtc.ConvertFromRGBA(255, 255, 255, 200), "LEVEL");
	textArea.top	= 1040;
	textArea.left	= 1450;
	textArea.right	= 1520;
	//df.rect(textArea.left, textArea.top, textArea.right-textArea.left, textArea.bottom-textArea.top);
	dtc.DrawInRect(&textArea, 38, 0, TEXTALIGN_LEFT|TEXTSCALE_NONE, dtc.ConvertFromRGBA(255, 255, 255, 200), "%2d", h->iPlaylevel[(int)(bms[musicID].GetStoredDataKind())]);

	// 曲名表示
	textArea.left	= 1520;
	textArea.right	= 1900;
	textArea.top	= 1042;
	textArea.bottom	= 1080;
	//df.rect(textArea.left, textArea.top, textArea.right-textArea.left, textArea.bottom-textArea.top);
	dty.DrawInRect(TEXT_MUSICTITLE00+musicID, &textArea, 38, 0, TEXTALIGN_LEFT|TEXTSCALE_AUTOXY, dtc.ConvertFromRGBA(255, 255, 255, 200));


	dd.DrawEnd();

	// 継続
	return 0;
}



///////////////////////////////////////////////////////
// 曲選択モード初期化関数
///////////////////////////////////////////////////////
BOOL CGame::InitMusicSelectionMode() {
	iCursor1 = 0;

	iRippleEffectIndex = 0;
	ZeroMemory(iRippleEffectCount, sizeof(iRippleEffectCount));

	ZeroMemory(bOnVirtualKey, sizeof(bOnVirtualKey));

	// 背景を白に設定
	dd.SetBackColor(0xffffff);


	return TRUE;
}


///////////////////////////////////////////////////////
// 曲選択モード実行関数
///////////////////////////////////////////////////////
int CGame::RunMusicSelectionMode() {
	int i, j, k;


	// 開始時から経過した時間を算出
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	dElapsedTime = (double)(li.QuadPart - llStartTime) / llGlobalFreq;

	///////////////////////////////////////////////////////////////////////////////////////
	// 入力処理関係
	///////////////////////////////////////////////////////////////////////////////////////
	// キーボード入力
	BYTE key[256];
	di.GetKeyboard(key);

	// midiキーボード入力
	MIDIKEYSTATE midiKey[MIDIKEYCNT];
	BOOL getMidiKeyFlag = mi.GetKeyState(midiKey);

	// 仮想入力キーへ変換
	BOOL pressVirtualKey[VIRTUALKEYCNT];
	ZeroMemory(pressVirtualKey, sizeof(pressVirtualKey));
	if (dElapsedTime > 0.5) {	// 前画面時の多重入力による誤操作を防ぐ
		InputVirtualKey(pressVirtualKey, midiKey, key, getMidiKeyFlag);
	}


	// ESCキーで終了
	if (key[DIK_Q]&0x80) return -1;




	// 60FPSでのデータ操作
	int lp = tm.Run();
	for (k=0; k<lp; k++) {
		for (i=0; i<ELEMENTSIZE(iRippleEffectCount); i++) {
			if (iRippleEffectCount[i].count > 0)
				iRippleEffectCount[i].count -= 1;
		}
	}


	// 波紋エフェクトの処理
	if (rand()%10 == 0) {
		iRippleEffectCount[iRippleEffectIndex].count	= RIPPLEEFFECTCNT;
		iRippleEffectCount[iRippleEffectIndex].x		= rand()%2000 - 20;
		iRippleEffectCount[iRippleEffectIndex].y		= rand()%1200 - 20;
		iRippleEffectIndex++;
		if (iRippleEffectIndex == ELEMENTSIZE(iRippleEffectCount)) {
			iRippleEffectIndex = 0;
		}
	}


	// カーソル処理
	if (pressVirtualKey[0] || pressVirtualKey[1] ||
		pressVirtualKey[2] || pressVirtualKey[3] ||
		pressVirtualKey[4] || pressVirtualKey[5]) {
		// 左へ移動
		iCursor1--;
		if (iCursor1 < 0)
			iCursor1 = iLoadMusicCnt-1;
	}
	else if (pressVirtualKey[19] || pressVirtualKey[20] ||
		pressVirtualKey[21] || pressVirtualKey[22] ||
		pressVirtualKey[23] || pressVirtualKey[24]) {
		// 右へ移動
		iCursor1++;
		if (iCursor1 >= iLoadMusicCnt)
			iCursor1 = 0;
	}
	else if (pressVirtualKey[8] || pressVirtualKey[9] ||
		pressVirtualKey[10] || pressVirtualKey[11] ||
		pressVirtualKey[12] || pressVirtualKey[13] ||
		pressVirtualKey[14] || pressVirtualKey[15] ||
		pressVirtualKey[16]) {
		// スペース(決定)
		return 0;
	}



	////////////////////////////////////////////////////////////////////////////////////
	// デバイスロストチェック(フルスクリーン時にALT+TABを押した場合など)
	// ※復帰時は内部で管理しているテクスチャは自動的にリストアされるが、
	//   MANAGEDではない頂点バッファやテクスチャを使用している場合は、
	//   自分でロスト＆リストア処理を行う
	////////////////////////////////////////////////////////////////////////////////////
	if (!dd.CheckDevice()) {
		// ロスト中なら
		if (!bLostDevice) {
			// ロスト直後ならここで開放処理を行う
			DEBUG("デバイスがロストした\n");

			bLostDevice = TRUE;
		}

		// 描画せずに抜ける
		return 1;
	}

	if (bLostDevice) {
		// リストア直後ならここで再構築を行う
		DEBUG("リストアされた\n");

		bLostDevice = FALSE;
	}




	//////////////////////////////////////////////////////
	// 描画処理
	//////////////////////////////////////////////////////
	dd.DrawBegin();


	// 背景
	dd.SetPutStatus(TEX_BACKGND_MUSICSELECTION);
	dd.Put(TEX_BACKGND_MUSICSELECTION, 0, 0);

	// 波紋
	for (i=0; i<ELEMENTSIZE(iRippleEffectCount); i++) {
		if (iRippleEffectCount[i].count > 0) {
			dd.SetPutStatus(TEX_RIPPLEGLAY, (float)iRippleEffectCount[i].count/RIPPLEEFFECTCNT, (RIPPLEEFFECTCNT - iRippleEffectCount[i].count)/(float)JUDGEEFFECTCNT, 0.0f);
			dd.Put2(TEX_RIPPLEGLAY, iRippleEffectCount[i].x, iRippleEffectCount[i].y);
		}
	}


	dtc.Draw(20, 20, 72, 0, dtc.ConvertFromRGBA(0, 0, 0, 200), "Music Selection");


	// 選択中の曲のまわりにある曲を表示
	int drawX, drawY;
	const int jacketBoxSizeX = 250;
	const int boxMargin = 50;
	const int center = 790;
	int jacketImgSizeX = 169;
	int jacketImgSizeY = 169;
	RECT textArea;
	CDDTexPro90 *tex = NULL;

	drawY = 270;
	for (i=-3; i<=3; i++) {
		if (iCursor1+i < 0 || i==0 || iCursor1+i >= iLoadMusicCnt)
			continue;

		drawX = center+(jacketBoxSizeX+boxMargin)*i;
		if (i>0)
			drawX += 72;

		dd.SetPutStatus(TEX_JACKETBOX_SHADE, 1.0f, 0.5f, 0.0f);
		dd.Put(TEX_JACKETBOX_SHADE, drawX+3, drawY+3);
		dd.SetPutStatus(TEX_JACKETBOX, 1.0f, 0.5f, 0.0f);
		dd.Put(TEX_JACKETBOX, drawX, drawY);

		// ジャケット表示の倍率計算
		tex = dd.GetTexClass(TEXFILE_JACKET00+iCursor1+i);
		float scale = 1.0f;
		if (tex->GetWidth()/tex->GetHeight() > jacketImgSizeX/jacketImgSizeY) {
			scale = (float)jacketImgSizeX/(float)tex->GetWidth();
		}
		else {
			scale = (float)jacketImgSizeY/(float)tex->GetHeight();
		}
		dd.SetPutStatus(TEX_JACKET00+iCursor1+i, 1.0f, scale, 0.0f);
		dd.Put2(TEX_JACKET00+iCursor1+i, drawX+45 + jacketImgSizeX/2, drawY+62 + jacketImgSizeY/2);

		textArea.left	= drawX+18;
		textArea.right	= drawX+jacketBoxSizeX-10;
		textArea.top	= drawY+258;
		textArea.bottom = drawY+294;
		df.noFill();
		df.stroke(0, 0, 0, 100);
		//df.rect(textArea.left, textArea.top, textArea.right-textArea.left, textArea.bottom-textArea.top);
		dty.DrawInRect(TEXT_MUSICTITLE00+iCursor1+i, &textArea, 28, 0, TEXTALIGN_CENTERXY|TEXTSCALE_AUTOXY, dty.ConvertFromRGBA(70, 70, 70));
		textArea.top	= drawY+294;
		textArea.bottom	= drawY+324;
		//df.rect(textArea.left, textArea.top, textArea.right-textArea.left, textArea.bottom-textArea.top);
		dty.DrawInRect(TEXT_MUSICARTIST00+iCursor1+i, &textArea, 24, 0, TEXTALIGN_CENTERXY|TEXTSCALE_AUTOXY, dty.ConvertFromRGBA(70, 70, 70));
	}



	// 選択中の曲を表示
	drawX = 802;
	drawY = 240;
	LPBMSHEADER h = bms[iCursor1].GetHeader();
	dd.SetPutStatus(TEX_JACKETBOX_SHADE, 1.0f, 0.6f, 0.0f);
	dd.Put(TEX_JACKETBOX_SHADE, drawX+6, drawY+6);
	dd.SetPutStatus(TEX_JACKETBOX, 1.0f, 0.6f, 0.0f);
	dd.Put(TEX_JACKETBOX, drawX, drawY);

	// ジャケット表示時の倍率計算
	float scale = 1.0f;
	jacketImgSizeX = 201;
	jacketImgSizeY = 201;
	tex = dd.GetTexClass(TEXFILE_JACKET00+iCursor1);
	if (tex->GetWidth()/tex->GetHeight() > jacketImgSizeX/jacketImgSizeY) {
		scale = (float)jacketImgSizeX/(float)tex->GetWidth();
	}
	else {
		scale = (float)jacketImgSizeY/(float)tex->GetHeight();
	}
	dd.SetPutStatus(TEX_JACKET00+iCursor1, 1.0f, scale, 0.0f);
	dd.Put2(TEX_JACKET00+iCursor1, drawX+55 + jacketImgSizeX/2, drawY+78 + jacketImgSizeY/2);

	textArea.left	= drawX+20;
	textArea.top	= drawY+314;
	textArea.right	= drawX+290;
	textArea.bottom = drawY+354;
	df.noFill();
	df.stroke(0, 0, 0, 100);
	//df.rect(textArea.left, textArea.top, textArea.right-textArea.left, textArea.bottom-textArea.top);
	dty.DrawInRect(TEXT_MUSICTITLE00+iCursor1, &textArea, 34, 0, TEXTALIGN_CENTERXY|TEXTSCALE_AUTOXY, dty.ConvertFromRGBA(0, 0, 0, 200));
	textArea.top	= drawY+354;
	textArea.bottom	= drawY+388;
	//df.rect(textArea.left, textArea.top, textArea.right-textArea.left, textArea.bottom-textArea.top);
	dty.DrawInRect(TEXT_MUSICARTIST00+iCursor1, &textArea, 30, 0, TEXTALIGN_CENTERXY|TEXTSCALE_AUTOXY, dty.ConvertFromRGBA(0, 0, 0, 200));





	// スコアボックス表示
	drawX = 684;
	drawY = 680;

	dd.SetPutStatus(TEX_SCOREBOX, 1.0f, 0.5f, 0.0f);
	LPPUTOBJECT obj = dd.GetPutObject(TEX_SCOREBOX);
	dd.Put(TEX_SCOREBOX, drawX, drawY);

	// スコアボックス内のテキスト表示
	SIZE boxInMargin;
	boxInMargin.cx = 15;
	boxInMargin.cy = -10;
	int textMargin = 20;
	int textAreaHeight = 40;

	SIZE drawAreaSize;
	drawAreaSize.cx = obj->mSize.cx*obj->fScaleX - boxInMargin.cx*2;
	drawAreaSize.cy = obj->mSize.cy*obj->fScaleY - boxInMargin.cy*2;
	int difficultyCnt = 0;
	for (i=0; i<4; i++) {
		if (h->iPlaylevel[i] >= 0) difficultyCnt++;
	}
	int elementCnt = 0;
	for (i=1; i < difficultyCnt+1; i++) {
		textArea.left	= drawX+boxInMargin.cx;
		textArea.top	= (int)(drawY+boxInMargin.cy + drawAreaSize.cy*i/(difficultyCnt+1) - textAreaHeight/2);
		textArea.right	= textArea.left + drawAreaSize.cx;
		textArea.bottom = textArea.top + textAreaHeight;

		while (h->iPlaylevel[elementCnt] < 0) elementCnt++;	// 難易度が存在する要素まで飛ばす
		char str[15];
		switch (elementCnt) {
			case 0: strcpy(str, "Elementary"); break;
			case 1: strcpy(str, "Intermediate"); break;
			case 2: strcpy(str, "Advanced"); break;
			case 3: strcpy(str, "Master"); break;
		}
		//df.rect(textArea.left, textArea.top, textArea.right-textArea.left, textArea.bottom-textArea.top);
		dtcs.DrawInRect(&textArea, 28, 0, TEXTALIGN_LEFT|TEXTSCALE_NONE, dtc.ConvertFromRGBA(0, 0, 0, 200), "%s", str);	// 難易度文字列
		textArea.left	+= 125;
		textArea.top	+= 30;
		//df.rect(textArea.left, textArea.top, textArea.right-textArea.left, textArea.bottom-textArea.top);
		dts.DrawInRect(&textArea, 12, 0, TEXTALIGN_NONE|TEXTSCALE_NONE, dtc.ConvertFromRGBA(40, 40, 40, 200), "LEVEL");
		textArea.left	+= 31;
		textArea.top	-= 36;
		//df.rect(textArea.left, textArea.top, textArea.right-textArea.left, textArea.bottom-textArea.top);
		dtc.DrawInRect(&textArea, 50, 0, TEXTALIGN_NONE|TEXTSCALE_NONE, dtc.ConvertFromRGBA(20, 20, 20, 255), "%2d", h->iPlaylevel[elementCnt]);


		elementCnt++;
	}


/*
	dtc.Draw(0, 130, 32, 0, 0xffff0000, "pressed");
	dtc.Draw(0, 160, 32, 0, 0xffff0000, "hold");

	for (i=0; i<VIRTUALKEYCNT; i++) {
		dtc.Draw(i*60+20+100, 100, 32, 0, 0xffff0000, "%d", i);
		if (pressVirtualKey[i]) dtc.Draw(i*60+100, 130, 32, 0, 0xffff0000, "ON");
		else dtc.Draw(i*60+100, 130, 32, 0, 0x3f000000, "OFF");

		if (bOnVirtualKey[i]) dtc.Draw(i*60+100, 160, 32, 0, 0xffff0000, "ON");
		else dtc.Draw(i*60+100, 160, 32, 0, 0x3f000000, "OFF");
	}*/


	dd.DrawEnd();

	return 1;
}


//////////////////////////////////////////////
// 難易度選択モード初期化
//////////////////////////////////////////////
BOOL CGame::InitDifficultySectionMode() {

	ZeroMemory(bOnVirtualKey, sizeof(bOnVirtualKey));

	// 前回選択した難易度に最も近い難易度を選択するようにカーソルをずらす
	LPBMSHEADER h = bms[iCursor1].GetHeader();
	int curDiff = INT_MAX;	// 適当なでっかい数
	int prevDiff = INT_MAX;
	int i, tmp;
	for (i=3; i>=0; i--) {
		if (h->iPlaylevel[i] >= 0) {
			curDiff = abs(iCursor2-i);
			tmp = i;
		}
		if (curDiff > prevDiff) break;

		prevDiff = curDiff;
	}
	iCursor2 = tmp;

	return TRUE;
}



///////////////////////////////////////////////
// 難易度選択モード実行ループ
///////////////////////////////////////////////
int CGame::RunDifficultySelectionMode() {
	int i, j, k;

	// 開始時から経過した時間を算出
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	dElapsedTime = (double)(li.QuadPart - llStartTime) / llGlobalFreq;


	///////////////////////////////////////////////////////////////////////////////////////
	// 入力処理関係
	///////////////////////////////////////////////////////////////////////////////////////
	// キーボード入力
	BYTE key[256];
	di.GetKeyboard(key);

	// midiキーボード入力
	MIDIKEYSTATE midiKey[MIDIKEYCNT];
	BOOL getMidiKeyFlag = mi.GetKeyState(midiKey);

	// 仮想入力キーへ変換
	BOOL pressVirtualKey[VIRTUALKEYCNT];
	ZeroMemory(pressVirtualKey, sizeof(pressVirtualKey));

	if (dElapsedTime > 0.5) {	// 前画面時の多重入力による誤操作を防ぐ
		InputVirtualKey(pressVirtualKey, midiKey, key, getMidiKeyFlag);
	}


	// ESCキーで終了
	if (key[DIK_Q]&0x80) return -1;



	// 60FPSでのデータ操作
	int lp = tm.Run();
	for (k=0; k<lp; k++) {
		for (i=0; i<ELEMENTSIZE(iRippleEffectCount); i++) {
			if (iRippleEffectCount[i].count > 0)
				iRippleEffectCount[i].count -= 1;
		}
	}


	// 波紋エフェクトの処理
	if (rand()%10 == 0) {
		iRippleEffectCount[iRippleEffectIndex].count	= RIPPLEEFFECTCNT;
		iRippleEffectCount[iRippleEffectIndex].x		= rand()%2000 - 20;
		iRippleEffectCount[iRippleEffectIndex].y		= rand()%1200 - 20;
		iRippleEffectIndex++;
		if (iRippleEffectIndex == ELEMENTSIZE(iRippleEffectCount)) {
			iRippleEffectIndex = 0;
		}
	}


	// カーソル処理 (難易度が存在しない譜面はスキップする)
	BMSHEADER *h = bms[iCursor1].GetHeader();

	if (pressVirtualKey[0] || pressVirtualKey[1] ||
		pressVirtualKey[2] || pressVirtualKey[3] ||
		pressVirtualKey[4] || pressVirtualKey[5]) {
		// 左へ移動

		while (iCursor2 > -1) {
			iCursor2--;
			if (iCursor2 == -1) break;
			if (h->iPlaylevel[iCursor2] >= 0) break;
		}
		if (iCursor2 < -1) iCursor2 = -1;
	}
	else if (pressVirtualKey[19] || pressVirtualKey[20] ||
		pressVirtualKey[21] || pressVirtualKey[22] ||
		pressVirtualKey[23] || pressVirtualKey[24]) {
		// 右へ移動

		int tmp = iCursor2 +1;
		while (tmp < 4) {
			if (h->iPlaylevel[tmp] >= 0) break;
			tmp++;
		}
		if (tmp < 4) iCursor2 = tmp; // 1つ上の難易度にカーソル移動 

		/*while (iCursor2 < 4) {
			iCursor2++;
			if (iCursor2 == 4) break;
			if (h->iPlaylevel[iCursor2] >= 0) break;
		}
		if (iCursor2 > 4) iCursor2 = 4;*/
	}
	else if (pressVirtualKey[8] || pressVirtualKey[9] ||
		pressVirtualKey[10] || pressVirtualKey[11] ||
		pressVirtualKey[12] || pressVirtualKey[13] ||
		pressVirtualKey[14] || pressVirtualKey[15] ||
		pressVirtualKey[16]) {
		// (決定)

		if (iCursor2 == -1)
			return 2;
		else
			return 0;
	}



	////////////////////////////////////////////////////////////////////////////////////
	// デバイスロストチェック(フルスクリーン時にALT+TABを押した場合など)
	// ※復帰時は内部で管理しているテクスチャは自動的にリストアされるが、
	//   MANAGEDではない頂点バッファやテクスチャを使用している場合は、
	//   自分でロスト＆リストア処理を行う
	////////////////////////////////////////////////////////////////////////////////////
	if (!dd.CheckDevice()) {
		// ロスト中なら
		if (!bLostDevice) {
			// ロスト直後ならここで開放処理を行う
			DEBUG("デバイスがロストした\n");

			bLostDevice = TRUE;
		}

		// 描画せずに抜ける
		return 1;
	}

	if (bLostDevice) {
		// リストア直後ならここで再構築を行う
		DEBUG("リストアされた\n");

		bLostDevice = FALSE;
	}




	//////////////////////////////////////////////////////
	// 描画処理
	//////////////////////////////////////////////////////
	dd.DrawBegin();


	// 背景
	dd.SetPutStatus(TEX_BACKGND_MUSICSELECTION);
	dd.Put(TEX_BACKGND_MUSICSELECTION, 0, 0);

	// 波紋
	for (i=0; i<ELEMENTSIZE(iRippleEffectCount); i++) {
		if (iRippleEffectCount[i].count > 0) {
			dd.SetPutStatus(TEX_RIPPLEGLAY, (float)iRippleEffectCount[i].count/RIPPLEEFFECTCNT, (RIPPLEEFFECTCNT - iRippleEffectCount[i].count)/(float)JUDGEEFFECTCNT, 0.0f);
			dd.Put2(TEX_RIPPLEGLAY, iRippleEffectCount[i].x, iRippleEffectCount[i].y);
		}
	}


	dtc.Draw(20, 20, 72, 0, dty.ConvertFromRGBA(0, 0, 0, 200), "Difficulty Selection");

	//// 選択曲表示 ////
	RECT textArea;
	const D3DPRESENT_PARAMETERS *param = dd.GetD3DPRESENT_PARAMETERS();
	dd.SetPutStatus(TEX_BIGJACKETBOX, 1.0f, 0.5f, 0.0f);
	const LPPUTOBJECT objBigJacketBox = dd.GetPutObject(TEX_BIGJACKETBOX);

	int drawX = param->BackBufferWidth/2 - objBigJacketBox->mSize.cx*objBigJacketBox->fScaleX/2;
	int drawY = 170;

	// ジャケットボックス
	dd.SetPutStatus(TEX_BIGJACKETBOX_SHADE, 1.0f, 0.5f, 0.0f);
	dd.Put(TEX_BIGJACKETBOX_SHADE, drawX+4, drawY+4);
	dd.Put(TEX_BIGJACKETBOX, drawX, drawY);

	// ジャケット
	const int jacketImgSizeX = 205;
	const int jacketImgSizeY = 205;

	float scale = 1.0f;
	CDDTexPro90 *tex = dd.GetTexClass(TEXFILE_JACKET00+iCursor1);
	if (tex->GetWidth()/tex->GetHeight() > jacketImgSizeX/jacketImgSizeY) {
		scale = (float)jacketImgSizeX/(float)tex->GetWidth();
	}
	else {
		scale = (float)jacketImgSizeY/(float)tex->GetHeight();
	}
	dd.SetPutStatus(TEX_JACKET00+iCursor1, 1.0f, scale, 0.0f);
	dd.Put2(TEX_JACKET00+iCursor1, drawX+47 + jacketImgSizeX/2, drawY+45 + jacketImgSizeY/2);

	// 曲情報
	char genreStr[][16] ={ "", "ORIGINAL", "VARIETY", "POPS", "GAME", "VOCALOID", "ANIME" };
	textArea.left	= drawX + 310;
	textArea.top	= drawY + 30;
	textArea.right	= drawX + objBigJacketBox->mSize.cx*objBigJacketBox->fScaleX - 30;
	textArea.bottom	= textArea.top + 20;
	for (i=0; i<BMS_MAXGENRE; i++) {
		if (h->eGenre[i] != 0) {
			//df.rect(textArea.left, textArea.top, textArea.right-textArea.left, textArea.bottom-textArea.top);
			dtc.DrawInRect(&textArea, 24, 0, TEXTALIGN_LEFT|TEXTSCALE_AUTOXY, dty.ConvertFromRGBA(0, 0, 0, 200), "%s", genreStr[bms[iCursor1].GetHeader()->eGenre[i]]);
		}
		textArea.left += 120;
	}
	textArea.left	= drawX + 310;
	textArea.top	= drawY + 70;
	textArea.bottom	= textArea.top + 35;
	//df.rect(textArea.left, textArea.top, textArea.right-textArea.left, textArea.bottom-textArea.top);
	dty.DrawInRect(TEXT_MUSICTITLE00+iCursor1, &textArea, 40, 0, TEXTALIGN_LEFT|TEXTSCALE_AUTOXY, dty.ConvertFromRGBA(0, 0, 0, 220));
	textArea.left	= drawX + 320;
	textArea.top	= drawY + 115;
	textArea.bottom	= textArea.top + 30;
	//df.rect(textArea.left, textArea.top, textArea.right-textArea.left, textArea.bottom-textArea.top);
	dty.DrawInRect(TEXT_MUSICARTIST00+iCursor1, &textArea, 36, 0, TEXTALIGN_LEFT|TEXTSCALE_AUTOXY, dty.ConvertFromRGBA(0, 0, 0, 200));
	textArea.top	= drawY + 150;
	textArea.bottom	= textArea.top + 30;
	//df.rect(textArea.left, textArea.top, textArea.right-textArea.left, textArea.bottom-textArea.top);
	dty.DrawInRect(TEXT_MUSICSUBTITLEA00+iCursor1, &textArea, 36, 0, TEXTALIGN_LEFT|TEXTSCALE_AUTOXY, dty.ConvertFromRGBA(0, 0, 0, 200));
	textArea.top	= drawY + 185;
	textArea.bottom	= textArea.top + 30;
	//df.rect(textArea.left, textArea.top, textArea.right-textArea.left, textArea.bottom-textArea.top);
	dty.DrawInRect(TEXT_MUSICSUBTITLEB00+iCursor1, &textArea, 36, 0, TEXTALIGN_LEFT|TEXTSCALE_AUTOXY, dty.ConvertFromRGBA(0, 0, 0, 200));
	textArea.left	= drawX + 390;
	textArea.top	= drawY + 240;
	textArea.bottom	= textArea.top + 35;
	int bpmMin, bpmMax;
	bpmMin = bpmMax = h->fBpm;
	for (i=0; bms[iCursor1].GetObjeNum(BMS_BPMINDEX); i++) {
		LPBMSDATA b = bms[iCursor1].GetObje(BMS_BPMINDEX, i);
		if (h->fBpmIndex[b->lData] < bpmMin) bpmMin = h->fBpmIndex[b->lData];
		else if (h->fBpmIndex[b->lData] > bpmMax) bpmMax = h->fBpmIndex[b->lData];
	}
	//df.rect(textArea.left, textArea.top, textArea.right-textArea.left, textArea.bottom-textArea.top);
	dtc.DrawInRect(&textArea, 32, 0, TEXTALIGN_LEFT|TEXTSCALE_AUTOXY, dtc.ConvertFromRGBA(0, 0, 0, 200), "%3d", bpmMin);
	if (bpmMin != bpmMax) {
		textArea.left = drawX + 420;
		//df.rect(textArea.left, textArea.top, textArea.right-textArea.left, textArea.bottom-textArea.top);
		dtc.DrawInRect(&textArea, 32, 0, TEXTALIGN_LEFT|TEXTSCALE_AUTOXY, dtc.ConvertFromRGBA(0, 0, 0, 200), " - %3d", bpmMax);
	}



	// 難易度ボックス表示
	drawX = 110;
	drawY = 550;
	const int drawAreaSizeX = 1700;
	const int boxSizeX = 300;
	const int selectBoxSizeX = 330;
	int difficultyCnt = 0;

	for (i=0; i<4; i++) {
		if (h->iPlaylevel[i] >= 0) difficultyCnt++;
	}
	int elementCnt = 0;
	for (i=1; i<difficultyCnt+1; i++) {
		while (h->iPlaylevel[elementCnt] < 0) elementCnt++;		// 難易度が存在する要素まで飛ばす
		char str[15];
		switch (elementCnt) {
			case 0: strcpy(str, "Elementary"); break;
			case 1: strcpy(str, "Intermediate"); break;
			case 2: strcpy(str, "Advanced"); break;
			case 3: strcpy(str, "Master"); break;
		}

		int boxX = (int)(drawX + i*(float)drawAreaSizeX/(difficultyCnt+1));
		const LPPUTOBJECT objDiffBox = dd.GetPutObject(TEX_DIFFBOX_ELEMENTARY + elementCnt);
		const LPPUTOBJECT objDiffBoxShade = dd.GetPutObject(TEX_DIFFBOX_SHADE);

		if (elementCnt != iCursor2) {
			boxX -= boxSizeX/2;	// 難易度ボックスの左上の座標にする

			dd.SetPutStatus(TEX_DIFFBOX_SHADE, 1.0f, (float)boxSizeX/(float)objDiffBoxShade->mSize.cx, 0.0f);
			dd.Put(TEX_DIFFBOX_SHADE, boxX+3, drawY+3);
			dd.SetPutStatus(TEX_DIFFBOX_ELEMENTARY + elementCnt, 1.0f, (float)boxSizeX/(float)objDiffBox->mSize.cx, 0.0f);
			dd.Put(TEX_DIFFBOX_ELEMENTARY + elementCnt, boxX, drawY);

			textArea.left	= boxX  + 17;
			textArea.top	= drawY + 17;
			textArea.right	= boxX + objDiffBox->mSize.cx*objDiffBox->fScaleX;
			textArea.bottom	= textArea.top + 40;
			//df.rect(textArea.left, textArea.top, textArea.right-textArea.left, textArea.bottom-textArea.top);
			dtc.DrawInRect(&textArea, 38, 0, TEXTALIGN_LEFT|TEXTSCALE_AUTOXY, dtc.ConvertFromRGBA(0, 0, 0, 230), "%s", str);
			textArea.left	= boxX + 229;
			textArea.top	= drawY + 25;
			textArea.bottom	= textArea.top + 65;
			//df.rect(textArea.left, textArea.top, textArea.right-textArea.left, textArea.bottom-textArea.top);
			dtc.DrawInRect(&textArea, 60, 0, TEXTALIGN_LEFT|TEXTSCALE_AUTOXY, dtc.ConvertFromRGBA(0, 0, 0, 230), "%2d", h->iPlaylevel[elementCnt]);

		}
		else {
			boxX -= selectBoxSizeX/2;	// 難易度ボックスの左上の座標にする
			const int boxY = drawY - (selectBoxSizeX-boxSizeX)/2;

			dd.SetPutStatus(TEX_DIFFBOX_SHADE, 1.0f, (float)selectBoxSizeX/(float)objDiffBoxShade->mSize.cx, 0.0f);
			dd.Put(TEX_DIFFBOX_SHADE, boxX+6, boxY+6);
			dd.SetPutStatus(TEX_DIFFBOX_ELEMENTARY + elementCnt, 1.0f, (float)selectBoxSizeX/(float)objDiffBox->mSize.cx, 0.0f);
			dd.Put(TEX_DIFFBOX_ELEMENTARY + elementCnt, boxX, boxY);

			textArea.left	= boxX + 21;
			textArea.top	= boxY + 21;
			textArea.right	= boxX + objDiffBox->mSize.cx*objDiffBox->fScaleX;
			textArea.bottom	= textArea.top + 46;
			//df.rect(textArea.left, textArea.top, textArea.right-textArea.left, textArea.bottom-textArea.top);
			dtc.DrawInRect(&textArea, 41, 0, TEXTALIGN_LEFT|TEXTSCALE_AUTOXY, dtc.ConvertFromRGBA(0, 0, 0, 230), "%s", str);
			textArea.left	= boxX + 250;
			textArea.top	= boxY + 25;
			textArea.bottom	= textArea.top + 74;
			//df.rect(textArea.left, textArea.top, textArea.right-textArea.left, textArea.bottom-textArea.top);
			dtc.DrawInRect(&textArea, 68, 0, TEXTALIGN_LEFT|TEXTSCALE_AUTOXY, dtc.ConvertFromRGBA(0, 0, 0, 230), "%2d", h->iPlaylevel[elementCnt]);

		}

		elementCnt++;
	}


	// BACK文字表示
	SetRect(&textArea, 60, 700, 180, 750);
	if (iCursor2 == -1) {
		dtc.DrawInRect(&textArea, 45, 0, TEXTALIGN_CENTERXY|TEXTSCALE_NONE, dty.ConvertFromRGBA(0, 0, 0, 200), "BACK");
	}
	else {
		dtc.DrawInRect(&textArea, 40, 0, TEXTALIGN_CENTERXY|TEXTSCALE_NONE, dty.ConvertFromRGBA(0, 0, 0, 100), "BACK");

	}
	/*if (iCursor2 == 4) {
		dtc.Draw(1780, 700, 40, 0, dty.ConvertFromRGBA(0, 0, 0, 200), "OPTION");
	}
	else {
		dtc.Draw(1780, 700, 35, 0, dty.ConvertFromRGBA(0, 0, 0, 100), "OPTION");

	}*/


	dd.DrawEnd();

	return 1;
}



///////////////////////////////////////////////
// 演奏結果モード初期化
///////////////////////////////////////////////
BOOL CGame::InitResultMode() {
	ZeroMemory(bOnVirtualKey, sizeof(bOnVirtualKey));

	iCursor2 = 0;
	dd.SetBackColor(0xffffff);

	return TRUE;
}



///////////////////////////////////////////////
// 演奏結果モード実行ループ
///////////////////////////////////////////////
int CGame::RunResultMode() {
	int i;


	// 開始時から経過した時間を算出
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	dElapsedTime = (double)(li.QuadPart - llStartTime) / llGlobalFreq;


	if (dElapsedTime > 15.0) {
		// 15秒経つと自動で画面遷移する
		return 0;
	}





	///////////////////////////////////////////////////////////////////////////////////////
	// 入力処理関係
	///////////////////////////////////////////////////////////////////////////////////////
	// キーボード入力
	BYTE key[256];
	di.GetKeyboard(key);

	// midiキーボード入力
	MIDIKEYSTATE midiKey[MIDIKEYCNT];
	BOOL getMidiKeyFlag = mi.GetKeyState(midiKey);

	// 仮想入力キーへ変換
	BOOL pressVirtualKey[VIRTUALKEYCNT];
	ZeroMemory(pressVirtualKey, sizeof(pressVirtualKey));

	if (dElapsedTime > 0.5) {	// 前画面時の多重入力による誤操作を防ぐ
		InputVirtualKey(pressVirtualKey, midiKey, key, getMidiKeyFlag);
	}

	// ESCキーで終了
	if (key[DIK_Q]&0x80) return -1;


	// カーソル処理
	if (pressVirtualKey[0] || pressVirtualKey[1] ||
		pressVirtualKey[2] || pressVirtualKey[3] ||
		pressVirtualKey[4] || pressVirtualKey[5]) {
		// 左へ移動
		iCursor2--;
		if (iCursor2 < 0)
			iCursor2 = 0;
	}
	else if (pressVirtualKey[19] || pressVirtualKey[20] ||
		pressVirtualKey[21] || pressVirtualKey[22] ||
		pressVirtualKey[23] || pressVirtualKey[24]) {
		// 右へ移動
		iCursor2++;
		if (iCursor2 > 2)
			iCursor2 = 2;
	}
	else if (pressVirtualKey[8] || pressVirtualKey[9] ||
		pressVirtualKey[10] || pressVirtualKey[11] ||
		pressVirtualKey[12] || pressVirtualKey[13] ||
		pressVirtualKey[14] || pressVirtualKey[15] ||
		pressVirtualKey[16]) {
		// スペース(決定)

		if (iCursor2 == 0)
			return 0;	// next
		else if (iCursor2 == 1)
			return 2;	// more info.
		else if (iCursor2 == 2)
			return 3;	// retry

	}




	////////////////////////////////////////////////////////////////////////////////////
	// デバイスロストチェック(フルスクリーン時にALT+TABを押した場合など)
	// ※復帰時は内部で管理しているテクスチャは自動的にリストアされるが、
	//   MANAGEDではない頂点バッファやテクスチャを使用している場合は、
	//   自分でロスト＆リストア処理を行う
	////////////////////////////////////////////////////////////////////////////////////
	if (!dd.CheckDevice()) {
		// ロスト中なら
		if (!bLostDevice) {
			// ロスト直後ならここで開放処理を行う
			DEBUG("デバイスがロストした\n");

			bLostDevice = TRUE;
		}

		// 描画せずに抜ける
		return 1;
	}

	if (bLostDevice) {
		// リストア直後ならここで再構築を行う
		DEBUG("リストアされた\n");

		bLostDevice = FALSE;
	}





	//////////////////////////////////////////////////////
	// 描画処理
	//////////////////////////////////////////////////////
	dd.DrawBegin();


	// 背景
	dd.SetPutStatus(TEX_BACKGND_MUSICSELECTION);
	dd.Put(TEX_BACKGND_MUSICSELECTION, 0, 0);


	dtc.Draw(20, 20, 72, 0, dty.ConvertFromRGBA(0, 0, 0, 200), "Result");

	df.stroke(0, 0, 0, 100);
	df.noFill();
	//// 選択曲表示 ////

	const int jacketImgSizeX = 290;
	const int jacketImgSizeY = 290;
	const int jacketBoxSizeX = 400;
	int drawX = 430;
	int drawY = 200;
	RECT textArea;
	LPBMSHEADER h = bms[iCursor1].GetHeader();
	LPPUTOBJECT objJacketBox = dd.GetPutObject(TEX_RESULTJACKETBOX_SHADE);

	// ジャケットボックス表示
	dd.SetPutStatus(TEX_RESULTJACKETBOX_SHADE, 1.0f, (float)jacketBoxSizeX/(float)objJacketBox->mSize.cx, 0.0f);
	dd.Put(TEX_RESULTJACKETBOX_SHADE, drawX+5, drawY+5);
	dd.SetPutStatus(TEX_RESULTJACKETBOX, 1.0f, (float)jacketBoxSizeX/(float)objJacketBox->mSize.cx, 0.0f);
	dd.Put(TEX_RESULTJACKETBOX, drawX, drawY);

	// ジャケット画像表示
	float scale = 1.0f;
	CDDTexPro90 *tex = dd.GetTexClass(TEXFILE_JACKET00+iCursor1);
	if (tex->GetWidth()/tex->GetHeight() > jacketImgSizeX/jacketImgSizeY) {
		scale = (float)jacketImgSizeX/(float)tex->GetWidth();
	}
	else {
		scale = (float)jacketImgSizeY/(float)tex->GetHeight();
	}
	dd.SetPutStatus(TEX_JACKET00+iCursor1, 1.0f, scale, 0.0f);
	dd.Put2(TEX_JACKET00+iCursor1, drawX+55 + jacketImgSizeX/2, drawY+51 + jacketImgSizeY/2);

	// 曲情報
	char genreStr[][16] ={ "", "ORIGINAL", "VARIETY", "POPS", "GAME", "VOCALOID", "ANIME" };
	textArea.left	= drawX + 20;
	textArea.top	= drawY + 380;
	textArea.right	= drawX + jacketBoxSizeX - 20;
	textArea.bottom	= textArea.top + 20;
	for (i=0; i<BMS_MAXGENRE; i++) {
		if (h->eGenre[i] != 0) {
			//df.rect(textArea.left, textArea.top, textArea.right-textArea.left, textArea.bottom-textArea.top);
			dtc.DrawInRect(&textArea, 18, 0, TEXTALIGN_LEFT|TEXTSCALE_AUTOXY, dty.ConvertFromRGBA(0, 0, 0, 200), "%s", genreStr[bms[iCursor1].GetHeader()->eGenre[i]]);
		}
		textArea.left += 100;
	}
	textArea.left	= drawX + 20;
	textArea.top	= drawY + 410;
	textArea.bottom	= textArea.top + 35;
	//df.rect(textArea.left, textArea.top, textArea.right-textArea.left, textArea.bottom-textArea.top);
	dty.DrawInRect(TEXT_MUSICTITLE00+iCursor1, &textArea, 40, 0, TEXTALIGN_LEFT|TEXTSCALE_AUTOXY, dty.ConvertFromRGBA(0, 0, 0, 220));
	textArea.top	= drawY + 450;
	textArea.bottom	= textArea.top + 25;
	//df.rect(textArea.left, textArea.top, textArea.right-textArea.left, textArea.bottom-textArea.top);
	dty.DrawInRect(TEXT_MUSICARTIST00+iCursor1, &textArea, 36, 0, TEXTALIGN_LEFT|TEXTSCALE_AUTOXY, dty.ConvertFromRGBA(0, 0, 0, 200));
	textArea.top	= drawY + 480;
	textArea.bottom	= textArea.top + 25;
	//df.rect(textArea.left, textArea.top, textArea.right-textArea.left, textArea.bottom-textArea.top);
	dty.DrawInRect(TEXT_MUSICSUBTITLEA00+iCursor1, &textArea, 36, 0, TEXTALIGN_LEFT|TEXTSCALE_AUTOXY, dty.ConvertFromRGBA(0, 0, 0, 200));
	textArea.top	= drawY + 510;
	textArea.bottom	= textArea.top + 25;
	//df.rect(textArea.left, textArea.top, textArea.right-textArea.left, textArea.bottom-textArea.top);
	dty.DrawInRect(TEXT_MUSICSUBTITLEB00+iCursor1, &textArea, 36, 0, TEXTALIGN_LEFT|TEXTSCALE_AUTOXY, dty.ConvertFromRGBA(0, 0, 0, 200));

	// 難易度表示
	char str[13];
	switch (bms[iCursor1].GetStoredDataKind()) {
		case DIFF_ELEMENTARY:	strcpy(str, "Elementary"); break;
		case DIFF_INTERMEDIATE: strcpy(str, "Intermediate"); break;
		case DIFF_ADVANCED:		strcpy(str, "Advanced"); break;
		case DIFF_MASTER:		strcpy(str, "Master"); break;
	}
	textArea.left	= drawX + 80;
	textArea.top	= drawY + 547;
	textArea.bottom	= textArea.top + 35;
	//df.rect(textArea.left, textArea.top, textArea.right-textArea.left, textArea.bottom-textArea.top);
	dtc.DrawInRect(&textArea, 34, 0, TEXTALIGN_LEFT|TEXTSCALE_AUTOXY, dtc.ConvertFromRGBA(0, 0, 0, 200), "%s", str);
	textArea.left	= drawX + 335;
	textArea.top	= drawY + 540;
	textArea.bottom	= textArea.top + 60;
	//df.rect(textArea.left, textArea.top, textArea.right-textArea.left, textArea.bottom-textArea.top);
	dtc.DrawInRect(&textArea, 46, 0, TEXTALIGN_LEFT|TEXTSCALE_AUTOXY, dtc.ConvertFromRGBA(0, 0, 0, 200), "%2d", h->iPlaylevel[(int)(bms[iCursor1].GetStoredDataKind())]);


	//// 判定結果表示 ////
	drawX = 1040;
	drawY = 250;
	const int textAreaSizeX = 450;
	const int textHeight = 60;
	const TOTALJUDGEREAULT *res = mg.GetTotalJudgeResult();
	double score = 0;
	double groove = 0.0;
	mg.GetPlayStatus(&score, &groove);
	if (100.0 <= groove && groove < 110.0)	groove = 100.0;
	else if (groove >= 110.0)				groove -= 10.0;

	textArea.left	= drawX;
	textArea.top	= drawY;
	textArea.right	= textArea.left + textAreaSizeX;
	textArea.bottom	= textArea.top + textHeight;
	const char judgestr[5][13] ={ "Just+", "Just", "Early/Late", "Early2/Late2", "Miss" };
	for (i=0; i<5; i++) {
		//df.rect(textArea.left, textArea.top, textArea.right-textArea.left, textArea.bottom-textArea.top);
		dtc.DrawInRect(&textArea, 40, 0, TEXTALIGN_LEFT|TEXTSCALE_AUTOXY, dtc.ConvertFromRGBA(0, 0, 0, 220), "%s", judgestr[i]);
		switch (i) {
			case 0: dtc.DrawInRect(&textArea, 40, 0, TEXTALIGN_RIGHT|TEXTSCALE_AUTOXY, dtc.ConvertFromRGBA(0, 0, 0, 220), "%3d", res->just); break;
			case 1: dtc.DrawInRect(&textArea, 40, 0, TEXTALIGN_RIGHT|TEXTSCALE_AUTOXY, dtc.ConvertFromRGBA(0, 0, 0, 220), "%3d", res->justE + res->justL); break;
			case 2: dtc.DrawInRect(&textArea, 40, 0, TEXTALIGN_RIGHT|TEXTSCALE_AUTOXY, dtc.ConvertFromRGBA(0, 0, 0, 220), "%3d/%3d", res->early, res->late); break;
			case 3: dtc.DrawInRect(&textArea, 40, 0, TEXTALIGN_RIGHT|TEXTSCALE_AUTOXY, dtc.ConvertFromRGBA(0, 0, 0, 220), "%3d/%3d", res->early2, res->late2); break;
			case 4: dtc.DrawInRect(&textArea, 40, 0, TEXTALIGN_RIGHT|TEXTSCALE_AUTOXY, dtc.ConvertFromRGBA(0, 0, 0, 220), "%3d", res->miss); break;
		}
		textArea.top	= textArea.bottom;
		textArea.bottom	= textArea.top + textHeight;
	}

	textArea.top	= drawY + 330;
	textArea.bottom = textArea.top + 100;
	//df.rect(textArea.left, textArea.top, textArea.right-textArea.left, textArea.bottom-textArea.top);
	dtc.DrawInRect(&textArea, 40, 0, TEXTALIGN_LEFT|TEXTSCALE_AUTOXY, dty.ConvertFromRGBA(0, 0, 0, 200), "GROOVE");
	dtc.DrawInRect(&textArea, 40, 0, TEXTALIGN_RIGHT|TEXTSCALE_AUTOXY, dty.ConvertFromRGBA(0, 0, 0, 200), "%3.1f %%", groove);

	textArea.top	= drawY + 460;
	textArea.bottom	= textArea.top + 120;
	//df.rect(textArea.left, textArea.top, textArea.right-textArea.left, textArea.bottom-textArea.top);
	dtc.DrawInRect(&textArea, 52, 0, TEXTALIGN_LEFT|TEXTSCALE_AUTOXY, dty.ConvertFromRGBA(0, 0, 0, 200), "SCORE");
	dtc.DrawInRect(&textArea, 52, 2, TEXTALIGN_RIGHT|TEXTSCALE_AUTOXY, dty.ConvertFromRGBA(0, 0, 0, 200), "%7.0f", score);


	textArea.top	= 850;
	textArea.left	= 850;
	textArea.right	= 1070;
	textArea.bottom	= 950;
	//df.rect(textArea.left, textArea.top, textArea.right-textArea.left, textArea.bottom-textArea.top);
	dtc.DrawInRect(&textArea, 52, 15, TEXTALIGN_CENTERXY|TEXTSCALE_AUTOXY, dtc.ConvertFromRGBA(0, 0, 0, 220), "END");


	dd.DrawEnd();

	return 1;
}


///////////////////////////////////////////////
// 演奏モードの終了処理
///////////////////////////////////////////////
BOOL CGame::ExitPlayMusicMode() {

	// キー音の解放
	for (int i=0; i<BMS_MAXINDEX; i++) {
		// WAVファイル名が存在するなら
		if (strlen(bms[iCursor1].GetWavFile(i))>0)
			ds.DelSound(i);
	}

	// 背景メディアの解放
	for (int i=0; i<BMS_MAXINDEX; i++) {
		LPBACKMEDIA m = bms[iCursor1].GetBackMedia(i);
		switch (m->type) {
			case BKMEDIATYPE_IMAGE:
				dd.DelTexture(TEXFILE_BKGNDIMAGE00);
				break;
			case BKMEDIATYPE_MOVIE:
				dm.Clear();
				break;
		}
	}
	iBackMediaIndex = -1;

	bms[iCursor1].ClearObjeData();
	mg.ClearAllJudgeResult();

	return TRUE;
}

BOOL CGame::RunScreenTransition(double stime) {
	// 開始時から経過した時間を算出
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	dElapsedTime = (double)(li.QuadPart - llStartTime) / llGlobalFreq;

	if (dElapsedTime > stime) {
		return 1;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// デバイスロストチェック(フルスクリーン時にALT+TABを押した場合など)
	// ※復帰時は内部で管理しているテクスチャは自動的にリストアされるが、
	//   MANAGEDではない頂点バッファやテクスチャを使用している場合は、
	//   自分でロスト＆リストア処理を行う
	////////////////////////////////////////////////////////////////////////////////////
	if (!dd.CheckDevice()) {
		// ロスト中なら
		if (!bLostDevice) {
			// ロスト直後ならここで開放処理を行う
			DEBUG("デバイスがロストした\n");

			bLostDevice = TRUE;
		}

		// 描画せずに抜ける
		return 0;
	}

	if (bLostDevice) {
		// リストア直後ならここで再構築を行う
		DEBUG("リストアされた\n");

		bLostDevice = FALSE;
	}



	//////////////////////////////////////////////////////
	// 描画処理
	//////////////////////////////////////////////////////
	dd.DrawBegin(false);

	dd.SetPutStatus(TEX_BACKGND_WHITE, (dElapsedTime)/stime, 1.0f, 0.0f);
	dd.Put(TEX_BACKGND_WHITE, 0, 0);

	dd.DrawEnd();

	return 0;
}

BOOL CGame::RunLoadingScreen() {

	////////////////////////////////////////////////////////////////////////////////////
	// デバイスロストチェック(フルスクリーン時にALT+TABを押した場合など)
	// ※復帰時は内部で管理しているテクスチャは自動的にリストアされるが、
	//   MANAGEDではない頂点バッファやテクスチャを使用している場合は、
	//   自分でロスト＆リストア処理を行う
	////////////////////////////////////////////////////////////////////////////////////
	if (!dd.CheckDevice()) {
		// ロスト中なら
		if (!bLostDevice) {
			// ロスト直後ならここで開放処理を行う
			DEBUG("デバイスがロストした\n");

			bLostDevice = TRUE;
		}

		// 描画せずに抜ける
		return 0;
	}

	if (bLostDevice) {
		// リストア直後ならここで再構築を行う
		DEBUG("リストアされた\n");

		bLostDevice = FALSE;
	}



	//////////////////////////////////////////////////////
	// 描画処理
	//////////////////////////////////////////////////////
	dd.DrawBegin(false);

	RECT textArea;
	textArea.left = 0;
	textArea.top  = 0;
	textArea.right= 1920;
	textArea.bottom = 1080;
	dtc.DrawInRect(&textArea, 65, 5, TEXTALIGN_CENTERXY|TEXTSCALE_AUTOXY, dtc.ConvertFromRGBA(0, 0, 0, 200), "Now Loading...");

	dd.DrawEnd();

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////
// ゲームメインルーチン
/////////////////////////////////////////////////////////////////////////
BOOL CGame::Run(HINSTANCE hinst) {
	// ゲームメインループ
	MSG msg;
	BOOL bLoop=TRUE;
	while (bLoop) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message==WM_QUIT) {
				bLoop = FALSE;
				DEBUG("WM_QUIT\n");
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// メインゲーム処理分け
		switch (eState) {
			case G_INIT:
				// 初期化
				if (!Init(hinst)) {
					eState = G_END;
				}
				else {
					eState = G_GAMEINIT;
				}
				break;

			case G_GAMEINIT:
				// 音楽ゲームの初期化
				if (!InitGame()) {
					eState = G_END;
				}
				else {
					eState = G_TITLEINIT;
				}
				break;


			case G_TITLEINIT:
				// タイトル画面の初期化
				if (!InitTitle()) {
					eState = G_END;
				}
				else {
					eState = G_TITLE;

					// 現在の時間を開始時間とする
					LARGE_INTEGER li;
					QueryPerformanceCounter(&li);
					llStartTime = li.QuadPart;
				}

			case G_TITLE:
				// タイトル画面
				switch (RunTitle()) {
					case -1:
						eState = G_TITLEINIT;
						break;
					case 0:
						eState = G_TITLEFADEOUT;

						// 現在の時間を開始時間とする
						LARGE_INTEGER li;
						QueryPerformanceCounter(&li);
						llStartTime = li.QuadPart;

						break;
					case 1:
						eState = G_TITLE;
						break;
				}
				break;

			case G_TITLEFADEOUT:
				// タイトル画面から曲選択へ画面遷移中

				if (RunScreenTransition(1.0)) {
					eState = G_SELECTMUSICINIT;
				}
				else {
					eState = G_TITLEFADEOUT;
				}

				break;

			case G_SELECTMUSICINIT:
				// 曲選択処理の初期化
				if (!InitMusicSelectionMode()) {
					eState = G_END;
				}
				else {
					eState = G_SELECTMUSIC;

					// 現在の時間を開始時間とする
					LARGE_INTEGER li;
					QueryPerformanceCounter(&li);
					llStartTime = li.QuadPart;
				}

				break;

			case G_SELECTMUSIC:
				// 曲選択処理
				switch (RunMusicSelectionMode()) {
					case -1:
						eState = G_TITLEINIT;
						break;
					case 0:
						eState = G_SELECTDIFFICULTYINIT;
						break;
					case 1:
						// 次フレームも曲選択モード
						break;
					default:
						break;
				}
				break;

			case G_SELECTDIFFICULTYINIT:
				// 難易度選択処理の初期化
				switch (InitDifficultySectionMode()) {
					case 0:
						eState = G_END;
						break;
					case 1:
						eState = G_SELECTDIFFICULTY;

						// 現在の時間を開始時間とする
						LARGE_INTEGER li;
						QueryPerformanceCounter(&li);
						llStartTime = li.QuadPart;

						break;
					default:
						break;
				}
				break;

			case G_SELECTDIFFICULTY:
				// 難易度選択処理
				switch (RunDifficultySelectionMode()) {
					case -1:
						eState = G_SELECTMUSICINIT;
						break;
					case 0:
						eState = G_SELECTDIFFFADEOUT;

						// 現在の時間を開始時間とする
						LARGE_INTEGER li;
						QueryPerformanceCounter(&li);
						llStartTime = li.QuadPart;

						break;
					case 1:
						// 次フレームも難易度選択モード
						break;
					case 2:
						// 曲選択モードへ戻る
						eState = G_SELECTMUSIC;
						break;
					default:
						break;
				}

				break;

			case G_SELECTDIFFFADEOUT:
				if (RunScreenTransition(1.0)) {
					eState = G_LOADING;
				}
				else {
					eState = G_SELECTDIFFFADEOUT;
				}
				break;

			case G_LOADING:
				if (RunLoadingScreen()) {
					eState = G_PLAYSONGINIT;
				}
				else {
					eState = G_LOADING;
				}
				break;

			case G_PLAYSONGINIT:
				// メインゲーム初期化

				DIFFICULTY diff;
				switch (iCursor2) {
					case 0: diff = DIFF_ELEMENTARY;		break;
					case 1: diff = DIFF_INTERMEDIATE;	break;
					case 2: diff = DIFF_ADVANCED;		break;
					case 3: diff = DIFF_MASTER;			break;
					default:							break;
				}

				switch (InitPlayMusicMode(iCursor1, diff)) {
					case 0:
						eState = G_SELECTDIFFICULTY;
						break;
					case 1:
						eState = G_PLAYSONG;

						Sleep(100);		// 前処理が確実に終わるように、少しだけ待機

						// 現在の時間を開始時間とする
						LARGE_INTEGER li;
						QueryPerformanceCounter(&li);
						llStartTime = li.QuadPart;

						break;
					default:
						break;
				}
				break;

			case G_PLAYSONG:
				// メインゲーム
				switch (RunPlayMusicMode(iCursor1, 0)) {
					case -1:
						// ESCAPE
						eState = G_PLAYSONGEND;
						break;
					case 1:
						// ゲーム終了
						eState = G_RESULTINIT;
						break;
				}
				break;

			case G_RESULTINIT:
				// 演奏結果画面の初期化
				switch (InitResultMode()) {
					case 0:
						eState = G_SELECTMUSIC;
						break;
					case 1:
						eState = G_RESULT;

						// 現在の時間を開始時間とする
						LARGE_INTEGER li;
						QueryPerformanceCounter(&li);
						llStartTime = li.QuadPart;


						break;
				}
				break;

			case G_RESULT:
				// 演奏結果画面
				switch (RunResultMode()) {
					case -1:
						eState = G_PLAYSONGEND;
						break;
					case 0:
						// 曲選択へ戻る
						eState = G_PLAYSONGEND;
						break;
					case 1:
						eState = G_RESULT;
						break;
					case 2:
						// 詳細結果へ
						break;
					case 3:
						// リトライ
						break;
				}

				break;

			case G_PLAYSONGEND:
				// 曲演奏モードの終了
				switch (ExitPlayMusicMode()) {
					case 0:
						eState = G_END;
						break;
					case 1:
						//eState = G_SELECTMUSIC;
						eState = G_TITLEINIT;
						break;
				}

				break;

			case G_END:
				// 終了処理
				Clear();
				bLoop = FALSE;
				break;

			default:
				// 未定義のステート
				DEBUG("異常終了\n");
				return FALSE;
		}

		Sleep(3);
	}

	win.Delete();

	// プログラム正常終了
	return TRUE;
}






/////////////////////////////////////////////////////////////
// 登録した全ての譜面データのヘッダだけロードする
/////////////////////////////////////////////////////////////
BOOL CGame::LoadAllBmsHeader(const char *scoreFileData) {
	FILE *fp;
	if ((fp = fopen(scoreFileData, "r")) == NULL) {
		DEBUG("ファイル \"%s\"が見つかりません。", scoreFileData);
		return FALSE;
	}

	char buf[MAX_PATH];
	char fileName[MAX_PATH];
	int cnt = 0;
	while (cnt < MAXMUSICCNT) {
		ZeroMemory(buf, sizeof(buf));
		if ((fgets(buf, sizeof(buf), fp)) == NULL)
			break;

		// 最後の改行文字を削除
		if (buf[strlen(buf)-1] == '\n')
			buf[strlen(buf)-1] = NULL;

		// ヘッダロード
		ZeroMemory(fileName, sizeof(fileName));
		sprintf(fileName, "MUSIC/%s", buf);

		if (!bms[cnt].LoadHeader(fileName)) {
			//DEBUG("譜面データ \"%s\" のロードに失敗しました\n", fileName);
		}
		else {
			cnt++;
		}
	}
	iLoadMusicCnt = cnt;

	return TRUE;
}

BOOL CGame::InputVirtualKey(BOOL *virtualKey, MIDIKEYSTATE *midiKey, BYTE *keyboard, BOOL IsUseMidi) {
	if (virtualKey == NULL) return FALSE;


	if (IsUseMidi) {
		// midiキーボードを使う

		for (int i=0; i<VIRTUALKEYCNT; i++) {
			switch (midiKey[i]) {
				case MIDIKEYSTATE_PRESSED:
					virtualKey[i] = TRUE;
					bOnVirtualKey[i] = TRUE;
					break;
				case MIDIKEYSTATE_RELEASED:
					bOnVirtualKey[i] = FALSE;
					break;
			}
		}
	}
	else {
		// キーボードを使う

		static const int KEYID[] ={		// キーのリスト
			DIK_1, DIK_2, DIK_3, DIK_4, DIK_5, DIK_6, DIK_7, DIK_8, DIK_9, DIK_0, DIK_MINUS, DIK_CIRCUMFLEX, DIK_YEN, DIK_BACKSPACE, DIK_NUMLOCK, DIK_DIVIDE, DIK_MULTIPLY,
			DIK_Q, DIK_W, DIK_E, DIK_R, DIK_T, DIK_Y, DIK_U, DIK_I, DIK_O, DIK_P, DIK_AT, DIK_LBRACKET, DIK_RETURN, DIK_NUMPAD7, DIK_NUMPAD8, DIK_NUMPAD9,
			DIK_A, DIK_S, DIK_D, DIK_F, DIK_G, DIK_H, DIK_J, DIK_K, DIK_L, DIK_SEMICOLON, DIK_COLON, DIK_RBRACKET, DIK_NUMPAD4, DIK_NUMPAD5, DIK_NUMPAD6,
			DIK_Z, DIK_X, DIK_C, DIK_V, DIK_B, DIK_N, DIK_M, DIK_COMMA, DIK_PERIOD, DIK_SLASH, DIK_BACKSLASH, DIK_RSHIFT, DIK_NUMPAD1, DIK_NUMPAD2, DIK_NUMPAD3,
		};

		BOOL on[VIRTUALKEYCNT];
		for (int i=0; i<VIRTUALKEYCNT; i++) {
			on[i] = bOnVirtualKey[i];	// バックアップ
		}
		ZeroMemory(bOnVirtualKey, sizeof(bOnVirtualKey));

		for (int i=0; i<ELEMENTSIZE(KEYID); i++) {
			double x;
			if (!mg.CalcKeyPosition(KEYID[i], 0.0, 0.99, &x)) continue;

			int virKeyNum = (int)(x*VIRTUALKEYCNT);		// 仮想キーの位置に変換

			if (keyboard[KEYID[i]]&0x80) {
				bOnVirtualKey[virKeyNum] = TRUE;

				if (!on[virKeyNum]) {
					// まだ押されていなければ押された瞬間とする
					virtualKey[virKeyNum] = TRUE;
				}
			}
		}
	}


	return TRUE;
}



