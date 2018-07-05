#include "MusicGame.h"

#define SAFE_FRRE(x) { if(x){free(x); x=NULL;} }

MyMusicGame::MyMusicGame() {
	iAllNoteJudgeCnt = 0;
	dScore = 0.0;
	dGrooveRate = 0.0;
	dVariGrooveRate = 0.0;;
	pNoteJudge = NULL;
	iNoteJudge = 0;
	bRateChangeSmoose = TRUE;
	iPrevCnt = 0;
	iCombo = 0;
	ZeroMemory(&judgeResult, sizeof(judgeResult));
}

MyMusicGame::~MyMusicGame() {
}

BOOL MyMusicGame::CalcKeyPosition( int DIKcode , float min, float max, double *posX, double *posY) {

	switch (DIKcode) {
	case DIK_1:
		if (posX) *posX = 0.0;
		if (posY) *posY = 0.0;
		break;
	case DIK_Q:
		if (posX) *posX = 1.0;
		if (posY) *posY = 1.0;
		break;
	case DIK_A:
		if (posX) *posX = 1.0;
		if (posY) *posY = 2.0;
		break;
	case DIK_2:
		if (posX) *posX = 2.0;
		if (posY) *posY = 0.0;
		break;
	case DIK_Z:
		if (posX) *posX = 2.0;
		if (posY) *posY = 3.0;
		break;
	case DIK_W:
		if (posX) *posX = 3.0;
		if (posY) *posY = 1.0;
		break;
	case DIK_S:
		if (posX) *posX = 3.0;
		if (posY) *posY = 2.0;
		break;
	case DIK_3:
		if (posX) *posX = 4.0;
		if (posY) *posY = 0.0;
		break;
	case DIK_X:
		if (posX) *posX = 4.0;
		if (posY) *posY = 3.0;
		break;
	case DIK_E:
		if (posX) *posX = 5.0;
		if (posY) *posY = 1.0;
		break;
	case DIK_D:
		if (posX) *posX = 5.0;
		if (posY) *posY = 2.0;
		break;
	case DIK_4:
		if (posX) *posX = 6.0;
		if (posY) *posY = 0.0;
		break;
	case DIK_C:
		if (posX) *posX = 6.0;
		if (posY) *posY = 3.0;
		break;
	case DIK_R:
		if (posX) *posX = 7.0;
		if (posY) *posY = 1.0;
		break;
	case DIK_F:
		if (posX) *posX = 7.0;
		if (posY) *posY = 2.0;
		break;
	case DIK_5:
		if (posX) *posX = 8.0;
		if (posY) *posY = 0.0;
		break;
	case DIK_V:
		if (posX) *posX = 8.0;
		if (posY) *posY = 3.0;
		break;
	case DIK_T:
		if (posX) *posX = 9.0;
		if (posY) *posY = 1.0;
		break;
	case DIK_G:
		if (posX) *posX = 9.0;
		if (posY) *posY = 2.0;
		break;
	case DIK_6:
		if (posX) *posX = 10.0;
		if (posY) *posY = 0.0;
		break;
	case DIK_B:
		if (posX) *posX = 10.0;
		if (posY) *posY = 3.0;
		break;
	case DIK_Y:
		if (posX) *posX = 11.0;
		if (posY) *posY = 1.0;
		break;
	case DIK_H:
		if (posX) *posX = 11.0;
		if (posY) *posY = 2.0;
		break;
	case DIK_7:
		if (posX) *posX = 12.0;
		if (posY) *posY = 0.0;
	case DIK_N:
		if (posX) *posX = 12.0;
		if (posY) *posY = 3.0;
		break;
	case DIK_U:
		if (posX) *posX = 13.0;
		if (posY) *posY = 1.0;
		break;
	case DIK_J:
		if (posX) *posX = 13.0;
		if (posY) *posY = 2.0;
		break;
	case DIK_8:
		if (posX) *posX = 14.0;
		if (posY) *posY = 0.0;
		break;
	case DIK_M:
		if (posX) *posX = 14.0;
		if (posY) *posY = 3.0;
		break;
	case DIK_I:
		if (posX) *posX = 15.0;
		if (posY) *posY = 1.0;
		break;
	case DIK_K:
		if (posX) *posX = 15.0;
		if (posY) *posY = 2.0;
		break;
	case DIK_9:
		if (posX) *posX = 16.0;
		if (posY) *posY = 0.0;
		break;
	case DIK_COMMA:
		if (posX) *posX = 16.0;
		if (posY) *posY = 3.0;
		break;
	case DIK_O:
		if (posX) *posX = 17.0;
		if (posY) *posY = 1.0;
		break;
	case DIK_L:
		if (posX) *posX = 17.0;
		if (posY) *posY = 2.0;
		break;
	case DIK_0:
		if (posX) *posX = 18.0;
		if (posY) *posY = 0.0;
		break;
	case DIK_PERIOD:
		if (posX) *posX = 18.0;
		if (posY) *posY = 3.0;
		break;
	case DIK_P:
		if (posX) *posX = 19.0;
		if (posY) *posY = 1.0;
		break;
	case DIK_SEMICOLON:
		if (posX) *posX = 19.0;
		if (posY) *posY = 2.0;
		break;
	case DIK_MINUS:
		if (posX) *posX = 20.0;
		if (posY) *posY = 0.0;
		break;
	case DIK_SLASH:
		if (posX) *posX = 20.0;
		if (posY) *posY = 3.0;
		break;
	case DIK_AT:
		if (posX) *posX = 21.0;
		if (posY) *posY = 1.0;
		break;
	case DIK_COLON:
		if (posX) *posX = 21.0;
		if (posY) *posY = 2.0;
		break;
	case DIK_CIRCUMFLEX:
		if (posX) *posX = 22.0;
		if (posY) *posY = 0.0;
		break;
	case DIK_BACKSLASH:
		if (posX) *posX = 22.0;
		if (posY) *posY = 3.0;
		break;
	case DIK_LBRACKET:
		if (posX) *posX = 23.0;
		if (posY) *posY = 1.0;
		break;
	case DIK_RBRACKET:
		if (posX) *posX = 23.0;
		if (posY) *posY = 2.0;
		break;
	case DIK_YEN:
		if (posX) *posX = 24.0;
		if (posY) *posY = 0.0;
		break;
	case DIK_BACKSPACE:
		if (posX) *posX = 25.0;
		if (posY) *posY = 0.0;
		break;
	case DIK_RETURN:
		if (posX) *posX = 25.0;
		if (posY) *posY = 1.5;
		break;
	case DIK_RSHIFT:
		if (posX) *posX = 25.0;
		if (posY) *posY = 3.0;
		break;
	case DIK_NUMLOCK:
		if (posX) *posX = 26.0;
		if (posY) *posY = 0.0;
		break;
	case DIK_NUMPAD7:
		if (posX) *posX = 26.0;
		if (posY) *posY = 1.0;
		break;
	case DIK_NUMPAD4:
		if (posX) *posX = 26.0;
		if (posY) *posY = 2.0;
		break;
	case DIK_NUMPAD1:
		if (posX) *posX = 26.0;
		if (posY) *posY = 3.0;
		break;
	case DIK_DIVIDE:
		if (posX) *posX = 27.0;
		if (posY) *posY = 0.0;
		break;
	case DIK_NUMPAD8:
		if (posX) *posX = 27.0;
		if (posY) *posY = 1.0;
		break;
	case DIK_NUMPAD5:
		if (posX) *posX = 27.0;
		if (posY) *posY = 2.0;
		break;
	case DIK_NUMPAD2:
		if (posX) *posX = 27.0;
		if (posY) *posY = 3.0;
		break;
	case DIK_MULTIPLY:
		if (posX) *posX = 28.0;
		if (posY) *posY = 0.0;
		break;
	case DIK_NUMPAD9:
		if (posX) *posX = 28.0;
		if (posY) *posY = 1.0;
		break;
	case DIK_NUMPAD6:
		if (posX) *posX = 28.0;
		if (posY) *posY = 2.0;
	case DIK_NUMPAD3:
		if (posX) *posX = 28.0;
		if (posY) *posY = 3.0;
		break;
	default:
		if (posX) *posX = -1.0;
		if (posY) *posY = -1.0;
		return FALSE;
	}
	
	if (posX) (*posX) = min + (*posX)/28.0*(max-min);
	if (posY) (*posY) = min + (*posY)/3.0*(max-min);

	return TRUE;
}

BOOL MyMusicGame::InitCalcPlayStatus(size_t allNoteJudgeCnt, BOOL RateChangeSmoose) {
	ClearAllJudgeResult();

	bRateChangeSmoose = RateChangeSmoose;
	iAllNoteJudgeCnt = allNoteJudgeCnt;

	judgeResult.notesJudgeCnt = allNoteJudgeCnt;

	pNoteJudge = (LPNOTEJUDGE)malloc(sizeof(NOTEJUDGE)*iAllNoteJudgeCnt);

	return TRUE;
}

BOOL MyMusicGame::GetPlayStatus(double *score, double *groove) {
	int i;
	double grTmp = 0.0;
	double scTmp = 0.0;

	for (i=iPrevCnt; i<iNoteJudge; i++) {
		grTmp = 1.3/iAllNoteJudgeCnt * 100.0;		// 総ノート数に応じて変化量の大きさを調節
		scTmp = 1000000.0/iAllNoteJudgeCnt;		// 総ノート数に応じて変化量の大きさを調節			

		switch (pNoteJudge[i].eJudge) {
		case JUST:		// just
			grTmp *= 0.500;
			scTmp *= 1.000;

			judgeResult.just++;
			break;
		case JUST_L:		// just L
			grTmp *= 0.450;
			scTmp *= 1.00;

			judgeResult.justL++;
			break;
		case JUST_E:	// just E
			grTmp *= 0.425;
			scTmp *= 1.00;

			judgeResult.justE++;
			break;
		case EARLY:
			grTmp *= 0.250;
			scTmp *= 0.50;

			judgeResult.early++;
			break;
		case LATE:
			grTmp *= 0.250;
			scTmp *= 0.50;

			judgeResult.late++;
			break;
		case EARLY2:
			grTmp *= -0.05;
			scTmp *= 0.30;

			judgeResult.early2++;
			break;
		case LATE2:
			grTmp *= -0.05;
			scTmp *= 0.30;

			judgeResult.late2++;
			break;
		case MISS_E:
		case MISS_L:
			grTmp *= -1.00;
			scTmp *= 0.00;

			judgeResult.miss++;
			break;
		case HOLDBREAK:
			grTmp *= -1.00;
			scTmp *= 0.00;

			judgeResult.holdbreak++;
			break;
		default:
			break;
		}
		grTmp*=2;


		dVariGrooveRate += grTmp;
		dScore += scTmp;

	}
	iPrevCnt = iNoteJudge;

	// グルーヴ率の増減処理
	if (bRateChangeSmoose) {
		if (fabs(dVariGrooveRate) < 0.05) {
			dGrooveRate += dVariGrooveRate;
			dVariGrooveRate = 0.0;
		}
		else {
			dGrooveRate += dVariGrooveRate/16.0;
			dVariGrooveRate -= dVariGrooveRate/16.0;
		}
	}
	else {
		dGrooveRate += dVariGrooveRate;
		dVariGrooveRate = 0.0;
	}
	if (dGrooveRate > 130.0)
		dGrooveRate = 130.0;
	else if (dGrooveRate < 0.0)
		dGrooveRate = 0.0;


	*score = dScore;
	*groove = dGrooveRate;

	return TRUE;
}

BOOL MyMusicGame::SaveJudgeResult(LONG RecTime, int keyCode, LONG noteNum, JUDGE judge, int iLane) {
	if (!pNoteJudge)
		return FALSE;
	if (iNoteJudge >= iAllNoteJudgeCnt)
		return FALSE;

	pNoteJudge[iNoteJudge].lTime	= RecTime;
	pNoteJudge[iNoteJudge].iKey		= keyCode;
	pNoteJudge[iNoteJudge].lNoteID	= noteNum;
	pNoteJudge[iNoteJudge].eJudge	= judge;
	pNoteJudge[iNoteJudge].iLane	= iLane;
	iNoteJudge++;

	return TRUE;
}

LPNOTEJUDGE MyMusicGame::GetJudgeResult(size_t num) {
	if (num >= iNoteJudge)
		return FALSE;

	return &pNoteJudge[num];
}

const TOTALJUDGEREAULT *MyMusicGame::GetTotalJudgeResult() {
	return &judgeResult;
}

int MyMusicGame::GetSavedNoteCnt() {
	return iNoteJudge;
}

BOOL MyMusicGame::ClearAllJudgeResult() {
	SAFE_FRRE(pNoteJudge);
	ZeroMemory(&judgeResult, sizeof(judgeResult));
	iAllNoteJudgeCnt = 0;
	dScore = 0.0;
	dGrooveRate = 0.0;
	dVariGrooveRate = 0.0;;
	iNoteJudge = 0;
	bRateChangeSmoose = TRUE;
	iPrevCnt = 0;

	return TRUE;
}





