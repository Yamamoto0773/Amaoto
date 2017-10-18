#ifndef _MUSIC_GAME_
#define _MUSIC_GAME_

//////////////////////////////////////////////////////
//���y�Q�[���Ŏg�������I�ȏ������܂Ƃ߂��N���X�ł��B
//�ėp���͂���܂���B
//////////////////////////////////////////////////////

#include <dinput.h>
#include <math.h>


// ����
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
	LONG			lTime;				// �f�[�^���L�^���ꂽ���� (���蕶���\���ɗ��p
	int				iKey;				// ���肵���L�[�̃L�[�R�[�h
	LONG			lNoteID;			// �m�[�g�̔ԍ�
	JUDGE			eJudge;				// ���茋��
	int				iLane;				// ���[���ԍ�
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

	// �L�[�̈ʒu���w�肵���l�͈̔͂ŕԂ�
	BOOL			CalcKeyPosition(int DIKcode, float min, float max, double *posX=NULL, double *posY=NULL);	
	// �X�R�A��O���[�������v�Z���邽�߂ɕK�v�ȏ���n��
	BOOL			InitCalcPlayStatus(size_t allNoteJudgeCnt, BOOL RateChangeSmoose=TRUE);
	// �X�R�A��O���[������Ԃ�
	BOOL			GetPlayStatus(double *score=NULL, double *groove=NULL);					
	// 1�̔��茋�ʂ�ۑ�����
	BOOL			SaveJudgeResult(LONG RecTime, int keyCode, LONG noteNum, JUDGE judge, int iLane); 
	// �w�肵���ԍ��̔��茋�ʂ�Ԃ�
	LPNOTEJUDGE		GetJudgeResult(size_t num);		
	// �e����̉񐔂�Ԃ�
	const TOTALJUDGEREAULT	*GetTotalJudgeResult();
	// ���茋�ʂ�ۑ������m�[�g�̐���Ԃ�
	int				GetSavedNoteCnt();
	// �S�Ă̔��茋�ʂ��N���A����
	BOOL			ClearAllJudgeResult();													



private:
	TOTALJUDGEREAULT	judgeResult;
	size_t			iAllNoteJudgeCnt;
	double			dScore;
	double			dGrooveRate;
	int				iCombo;
	double			dVariGrooveRate;
	LPNOTEJUDGE		pNoteJudge;			// �e�m�[�g�̔��茋�� ( -4:MISS(E) -3:EARLY(2) -2:EARLY -1:JUST(E) 0:JUST 1:JUST(L) 2:LATE 3:LATE(2) 4:MISS(L) )
	int				iNoteJudge;			// ���茋�ʂ̊i�[��
	BOOL			bRateChangeSmoose;
	int				iPrevCnt;			// �O��v�Z��������̔ԍ�

};

#endif