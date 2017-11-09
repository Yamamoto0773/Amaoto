#ifndef _MUSIC_GAME_
#define _MUSIC_GAME_

//////////////////////////////////////////////////////
//音楽ゲームで使う内部的な処理をまとめたクラスです。
//汎用性はありません。
//////////////////////////////////////////////////////

#include <dinput.h>
#include <math.h>


// 判定
typedef enum _JUDGE {
	HOLDBREAK = -5,
	MISS_E,
	EARLY2,
	EARLY,
	JUST_E,
	JUST,
	JUST_L,
	LATE,
	LATE2,
	MISS_L
}JUDGE;


typedef struct _NOTEJUDGE {
	LONG			lTime;				// データが記録された時間 (判定文字表示に利用
	int				iKey;				// 判定したキーのキーコード
	LONG			lNoteID;			// ノートの番号
	JUDGE			eJudge;				// 判定結果
	int				iLane;				// レーン番号
} NOTEJUDGE, *LPNOTEJUDGE;


typedef struct _TOTALJUDGERESULT {
	int just;
	int justL;
	int justE;
	int early;
	int late;
	int early2;
	int late2;
	int miss;
	int holdbreak;
	int notesJudgeCnt;
}TOTALJUDGEREAULT;


class MyMusicGame {
public:
	MyMusicGame();
	virtual ~MyMusicGame();

	// キーの位置を指定した値の範囲で返す
	BOOL			CalcKeyPosition(int DIKcode, float min, float max, double *posX=NULL, double *posY=NULL);	
	// スコアやグルーヴ率を計算するために必要な情報を渡す
	BOOL			InitCalcPlayStatus(size_t allNoteJudgeCnt, BOOL RateChangeSmoose=TRUE);
	// スコアやグルーヴ率を返す
	BOOL			GetPlayStatus(double *score=NULL, double *groove=NULL);					
	// 1つの判定結果を保存する
	BOOL			SaveJudgeResult(LONG RecTime, int keyCode, LONG noteNum, JUDGE judge, int iLane); 
	// 指定した番号の判定結果を返す
	LPNOTEJUDGE		GetJudgeResult(size_t num);		
	// 各判定の回数を返す
	const TOTALJUDGEREAULT	*GetTotalJudgeResult();
	// 判定結果を保存したノートの数を返す
	int				GetSavedNoteCnt();
	// 全ての判定結果をクリアする
	BOOL			ClearAllJudgeResult();													



private:
	TOTALJUDGEREAULT	judgeResult;
	size_t			iAllNoteJudgeCnt;
	double			dScore;
	double			dGrooveRate;
	int				iCombo;
	double			dVariGrooveRate;
	LPNOTEJUDGE		pNoteJudge;			// 各ノートの判定結果 ( -4:MISS(E) -3:EARLY(2) -2:EARLY -1:JUST(E) 0:JUST 1:JUST(L) 2:LATE 3:LATE(2) 4:MISS(L) )
	int				iNoteJudge;			// 判定結果の格納先
	BOOL			bRateChangeSmoose;
	int				iPrevCnt;			// 前回計算した判定の番号

};

#endif