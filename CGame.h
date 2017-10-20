#ifndef _CGAME_H
#define _CGAME_H
////////////////////////////////////////////////////////////////////////////
// CGame : ゲームメインルーチン v2.00
// ・Win10動作確認済み
////////////////////////////////////////////////////////////////////////////
#include "./libfiles/CDDPro90.h"
#include "./libfiles/CDSPro81.h"
#include "./libfiles/CDIPro81.h"
#include "./libfiles/CWindow.h"
#include "./libfiles/CBmsPro.h"
#include "./libfiles/CTimer.h"
#include "./libfiles/DirectXText.h"
#include "./libfiles/DXTextANSI.h"
#include "./libfiles/DirectXFigure.h"
#include "./libfiles/MusicGame.h"
#include "./libfiles/DirectXMovie.h"
#include "./libfiles/MIDIinput.h"



#define MAXMUSICCNT				(128)		// 読み込める曲の最大数

#define JUDGEEFFECTCNT			(80)
#define RIPPLEEFFECTCNT			(70)

#define VIRTUALKEYCNT			(25)		// 仮想入力キーの数


#define TEXT_MUSICTITLE00		(0)
#define TEXT_MUSICARTIST00		(TEXT_MUSICTITLE00+MAXMUSICCNT)
#define TEXT_MUSICSUBTITLEA00 	(TEXT_MUSICARTIST00+MAXMUSICCNT)
#define TEXT_MUSICSUBTITLEB00	(TEXT_MUSICSUBTITLEA00+MAXMUSICCNT)


#define TEXFILE_IMAGE0			(0)
#define TEXFILE_IMAGE1			(1)
#define TEXFILE_IMAGE2			(2)
#define TEXFILE_IMAGE3			(3)
#define TEXFILE_IMAGE4			(4)
#define TEXFILE_IMAGE5			(5)
#define TEXFILE_IMAGE6			(6)
#define TEXFILE_IMAGE7			(7)
#define TEXFILE_IMAGE8			(8)
#define TEXFILE_BKGNDIMAGE00	(10)
#define TEXFILE_JACKET00		(TEXFILE_BKGNDIMAGE00+BMS_MAXINDEX)

#define TEX_JUDGELINE			(0)
#define TEX_JUDGELINEEFFECT		(1)
#define TEX_NOTE				(2)
#define TEX_DIFFBOX_ELEMENTARY	(3)
#define TEX_DIFFBOX_INTERMEDIATE (4)
#define TEX_DIFFBOX_ADVANCED	(5)
#define TEX_DIFFBOX_MASTER		(6)
#define TEX_DIFFBOX_PROFESSIONAL (7)
#define TEX_DIFFBOX_SHADE		(8)
#define TEX_JACKETBOX			(9)
#define TEX_SCOREBOX			(10)
#define TEX_BIGJACKETBOX		(11)
#define TEX_RESULTJACKETBOX		(12)
#define TEX_JACKETBOX_SHADE		(13)
#define TEX_BIGJACKETBOX_SHADE	(14)
#define TEX_RESULTJACKETBOX_SHADE (15)
#define TEX_JUDGESTR_EARLY2		(16)
#define TEX_JUDGESTR_EARLY		(17)
#define TEX_JUDGESTR_JUSTE		(18)
#define TEX_JUDGESTR_JUST		(19)
#define TEX_JUDGESTR_JUSTL		(20)
#define TEX_JUDGESTR_LATE		(21)
#define TEX_JUDGESTR_LATE2		(22)
#define TEX_JUDGESTR_MISS		(23)
#define TEX_JUDGEEFFECT_JUST	(24)
#define TEX_RIPPLEGLAY			(25)
#define TEX_BACKGND_MUSICSELECTION (26)
#define TEX_LOGO_AMAOTO			(27)
#define TEX_LOGO_AMATSUBU			(28)
#define TEX_BACKGND_TITLE		(29)
#define TEX_BACKGND_WHITE		(30)
#define TEX_BUTTONEFFECT		(31)
#define TEX_RIPPLEWHITE			(32)
#define TEX_BKGND00				(100)
#define TEX_JACKET00			(TEX_BKGND00+BMS_MAXINDEX)

#define SND_AMAOTO				BMS_MAXINDEX


#define ELEMENTSIZE(x) ((sizeof(x)/sizeof(x[0])))


typedef struct _EFFECTSTATE {
	int count;
	int x;
	int y;
}EFFECTSTATE;


// ステート
enum STATE {
	G_INIT=0,	// ゲーム初期化
	G_GAMEINIT,	// 音楽ゲーム初期化(テクスチャの読み込みなど
	G_TITLEINIT,
	G_TITLE,		// タイトル画面
	G_TITLEFADEOUT,
	G_SELECTMUSICINIT,	// 曲選択処理の初期化
	G_SELECTMUSIC,		// 曲選択処理
	G_SELECTDIFFICULTYINIT,	// 難易度選択処理の初期化
	G_SELECTDIFFICULTY,		// 難易度選択処理
	G_SELECTDIFFFADEOUT,
	G_LOADING,		
	G_PLAYSONGINIT,	// 曲演奏処理の初期化
	G_PLAYSONG,		// 曲演奏処理
	G_RESULTINIT,	// 演奏結果画面の初期化
	G_RESULT,		// 演奏結果画面
	G_RESULT2,		// 演奏結果画面(詳細
	G_PLAYSONGEND,	// 曲演奏結果の終了処理
	G_END		// 全てのゲーム終了処理
};


class CGame {
	// ライブラリなど
	CWindow			win;				// ウインドウ管理
	CDDPro90		dd;					// Direct3D管理クラス
	CDSPro81		ds;					// DirectSound管理
	CDIPro81		di;					// DirectInput全般
	DirectXText		dty;				// テキスト関連 游ゴシック体
	DXTextANSI		dtc;				// century gothic fontsize:72
	DXTextANSI		dtcs;				// century gothic fontsize:30
	DXTextANSI		dts;				// sans serif fontsize:14
	DirectXFigure	df;					// 図形関連
	MyMusicGame		mg;					// 音楽ゲームの内部処理
	DirectXMovie	dm;					// 動画の再生クラス
	MIDIInput		mi;					// midiキーボードの入力


	CTimer			tm;					// 60FPS処理
	CBmsPro			bms[MAXMUSICCNT];	// BMSファイル管理用

	bool			bLostDevice;		// D3Dデバイスロスト状態フラグ

	// ゲームのメイン処理で使用する変数
	STATE			eState;				// ゲームのステート状況
	float			fScrMulti;			// 小節間の幅の倍率
	long			llStartTime;		// ゲーム開始時の時間(高解像度タイマー)
	long		llGlobalFreq;		// マシンの１秒間のカウント数(高解像度タイマー)
	double			dElapsedTime;		// 開始からの経過時間(秒)
	int				iStartNum[BMS_MAXNOTELANE+BMS_MAXINDEX];		// BMS演奏で計算開始する配列番号(処理を軽くするため)
	
	int				iCursor1;			// 現在選択中の項目
	int				iCursor2;			// 現在選択中の項目


	// ファイルロード
	int				iLoadMusicCnt;			// ロードした曲数

	// 入力状態
	bool			bOnVirtualKey[VIRTUALKEYCNT];		// 仮想入力キーが押されているか
	long			lHoldNote[VIRTUALKEYCNT];			// 現在判定中のHOLDノートID

	// 演出など
	int				iNoteEffectIndex[BMS_MAXNOTELANE];		// 次に使用される判定表示エフェクトのインデックス
	int				iNoteEffectCount[BMS_MAXNOTELANE][30];	// 判定表示エフェクトのカウンタ
	int				iRippleEffectIndex;						// 波紋エフェクトカウンタのインデックス
	EFFECTSTATE		iRippleEffectCount[50];					// 波紋エフェクトのカウント
	int				iBackMediaIndex;	// 現在表示中の背景メディアのインデックス番号
	bool			bIsBackMedia;		// 背景メディアが存在するかのフラグ
	bool			bFlag;				// 何かに使う

private:
	// 初期化
	bool Init( HINSTANCE hinst );		// 初期化＆ゲーム形成
	bool Clear( void );					// ロード済みデータの開放


	// ゲーム処理
	bool	InitGame();
	bool	InitPlayMusicMode(long musicID, DIFFICULTY diffID);							// 演奏モード初期化
	bool	InitTitle();
	int		RunTitle();																	// タイトル画面ループ
	int		RunPlayMusicMode(long musicID, bool demo = FALSE);							// 演奏モード実行ループ
	bool	InitMusicSelectionMode();													// 曲選択モード初期化
	int		RunMusicSelectionMode();													// 曲選択モード実行ループ
	bool	InitDifficultySectionMode();												// 難易度選択モード初期化
	int		RunDifficultySelectionMode();												// 難易度選択モード実行ループ
	bool	InitResultMode();
	int		RunResultMode();
	bool	ExitPlayMusicMode();
	bool	RunScreenTransition(double stime);
	bool	RunLoadingScreen();

	bool	LoadAllBmsHeader(const char *file);
	bool	InputVirtualKey(bool *virtualKey, MIDIKEYSTATE *midiKey, unsigned char *keyboard, bool IsUseMidi);	// 仮想入力キーへ入力


private:
	double	dGrooveRateCoef[BMS_MAXNOTELANE];	// ※CalcGrooveRate()で使います

public:
	// 公開関数
	CGame();
	virtual ~CGame();
	
	bool Run( HINSTANCE hinst );		// ゲームメインルーチン
};



#endif
