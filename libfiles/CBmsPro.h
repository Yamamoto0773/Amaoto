#ifndef _CBMSPRO_H
#define _CBMSPRO_H
///////////////////////////////////////////////////////////////////////////////////
// CBmsPro : BMSクラス チュートリアルバージョン v3.05                            //
//                                                                               //
// このソースコードは自由に改変して使用可能です。                                //
// また商用利用も可能ですが、すべての環境で正しく動作する保障はありません。      //
//                          http://www.charatsoft.com/                           //
///////////////////////////////////////////////////////////////////////////////////
#include <Windows.h>

#define BMS_RESOLUTION		9600			// 1小節のカウント値
#define BMS_MAXINDEX		(36*36)			// 最大インデックス数
#define BMS_MAXNOTELANE		(36*36-36)			// ノートレーンの最大定義数
#define BMS_MAXCHANNEL		35			// チャンネルの最大数
#define BMS_MAXGENRE		0x0A			// 登録できるジャンルの最大数
#define BMS_MAXSUBTITLE		0x02			// 登録できるサブタイトルの登録数

// BMSチャンネル定義
#define BMS_BACKMUSIC		0x01			// その位置にきたら、自動的に再生されるWAVを指定します
#define BMS_STRETCH			0x02			// その小節の長さを定義したデータ倍します（１０進数、小数ともに使用可）
#define BMS_TEMPO			0x03			// 再生テンポ（ＢＰＭ / １分間の四分音符数）の途中変更（１６進数）
#define BMS_BACKMEDIA		0x04			// 背景用メディアのタイミング指定
#define BMS_EXTENEDOBJ		0x05			// 落下してくるオブジェを別のキャラクターナンバーのものにすり替える機能
#define BMS_CHGPOORANI		0x06			// POORを出したときに表示される画像を変更
//#define BMS_LAYERANIME		0x07			// Ch.04で指定したBGAの上にかぶせるBMPを指定できます
#define BMS_BPMINDEX		0x08			// BPMのインデックス指定(新)
#define BMS_NOTE			36				// ノートの配置

#define BMS_HOLDENDNOTE		BMS_MAXINDEX-1	// ホールド終点ノートを表すインデックス番号


typedef enum _NOTETYPE {
	HIT=0,
	HOLD
}NOTETYPE;

typedef enum _DIFFICULTY {
	DIFF_NONE=-1,
	DIFF_ELEMENTARY,
	DIFF_INTERMEDIATE,
	DIFF_ADVANCED,
	DIFF_MASTER,
}DIFFICULTY;

typedef enum _MUSICGENRE {
	ORIGINAL=1,
	VARIETY,
	POPS,
	GAME,
	VOCALOID,
	ANIME,
	INVALID
}MUSICGENRE;

typedef enum _BACKMEDIATYPE {
	BKMEDIATYPE_NONE = 0,
	BKMEDIATYPE_IMAGE,
	BKMEDIATYPE_MOVIE
}BACKMEDIATYPE;


// BMSヘッダ情報
typedef struct _BMSHEADER {
	int			iID;						// 曲ID
	char		mDataFilePath[MAX_PATH];	// 譜面データへのパス
	char		mTitle[256];				// 曲のタイトル
	char		mSubTitle[BMS_MAXSUBTITLE][128];	// 曲のサブタイトル
	char		mArtist[128];				// 曲のアーティスト
	MUSICGENRE	eGenre[BMS_MAXGENRE];		// 曲のジャンル
	float		fBpm;						// 初期テンポ(初期値は130)
	char		mMidifile[MAX_PATH];		// バックグラウンドで流すMIDIファイル
	int			iPlaylevel[4];				// 曲の難易度
	float		fWavVol;					// 音量を元の何%にするか
	char		mJacket[MAX_PATH];			// ジャケット
	float		fBpmIndex[BMS_MAXINDEX];	// テンポインデックス(初期値は120)
	char		mComment[512];				// コメント

	long		lEndBar;					// 終了小節
	long		lMaxCount;					// 最大のカウント数
} BMSHEADER, *LPBMSHEADER;


// BMSデータ情報
typedef struct _BMSDATA {
	LONG		lTime;						// このデータの開始位置(BMSカウント値)
	LONG		lData;						// long型データ
	float		fData;						// 小数値データ(テンポ用)
	BOOL		bFlag;						// bool型データ
} BMSDATA, *LPBMSDATA;


// ノートデータ1(配置情報
typedef struct _BMSNOTE {
	NOTETYPE	eType;						// ノートの種類
	LONG		lTime1;						// このデータの開始位置(BMSカウント値)
	LONG		lTime2;						// このデータの開始位置(ホールド終点ノート用
	BOOL		bFlag1;						// 判定済かどうか
	BOOL		bFlag2;						// 判定済かどうか(ホールド終点ノート用
	int			iLane;						// ノートレーン番号
	LONG		lIndex;						// ノートデータ参照用番号(0x01～0xFF)
}BMSNOTE, *LPBMSNOTE;

// ノートデータ2(ノートの定義情報
typedef struct _NOTEDEFDATA {
	LONG		lSound;						// ノートの音
	float		fVol;						// 音量
	float		fSize;						// サイズ
	float		fScore;						// スコア倍率
	int			iColor[4];					// 色
}NOTEDEFDATA, *LPNOTEDEFDATA;

// 小節情報
typedef struct _BMSBAR {
	float		fScale;						// この小節の長さ倍率
	LONG		lTime;						// この小節の開始位置(BMSカウント値)
	LONG		lLength;					// この小節の長さ(BMSカウント値)
} BMSBAR, *LPBMSBAR;

// ノートレーンデータ
typedef struct _LANE {
	float		fTopX;						// 上端のx座標
	float		fBottomX;					// 下端のx座標
	int			iColor[4];					// 色
	char		mName[56];					// 名前
} LANE, *LPLANE;

// 背景メディアデータ
typedef struct _BACKMEDIA {
	BACKMEDIATYPE type;
	char mFileName[MAX_PATH];
}BACKMEDIA, *LPBACKMEDIA;


////////////////////////////////////////////////////////////////////////////////////
// BMSクラス
////////////////////////////////////////////////////////////////////////////////////
class CBmsPro {
protected:
	BMSHEADER	mBH;										// BMSヘッダ情報
	BMSBAR		mBmsBar[1000+1];							// 小節データ(999小節時に1000番目も小節バーとして使うため+1しておく)

	DIFFICULTY	eStoredDataKind;							// 現在格納している譜面データの難易度
	LPBMSDATA	pBmsData[BMS_MAXCHANNEL];					// 実データ
	int			iBmsData[BMS_MAXCHANNEL];					// 実データのそれぞれの数

	LPBMSNOTE	pNoteData;									// ノートデータ
	int			iNoteData;									// ノートデータの数
	
	LANE		mLaneData[BMS_MAXNOTELANE];					// ノートレーンデータ
	NOTEDEFDATA mNoteDefData[BMS_MAXINDEX];					// ノートデータ
	int			iHoldNote;									// ホールドノートの数

	char		mWavFile[BMS_MAXINDEX][MAX_PATH];			// WAVのファイル名

	BACKMEDIA	mBackMedia[BMS_MAXINDEX];					// 背景メディアデータ

	char		mLastError[1024];							// エラーが発生した場合の文字列

public:
	CBmsPro();												// コンストラクタ
	virtual ~CBmsPro();										// デストラクタ

	// 基本メソッド
	BOOL ClearAll(void);								// データの初期化
	BOOL ClearObjeData(void);							// ヘッダーデータ以外を初期化
	BOOL LoadHeader(const char *file);					// BMSヘッダ情報だけを取り出す
	BOOL LoadObjeData(DIFFICULTY diff);					// オブジェデータの読み込み(先にヘッダーデータをロードしないと失敗します
	BOOL Load(const char *file, DIFFICULTY diff);		// BMSファイルのロード
	BOOL Save(const char *file);						// BMSファイルにセーブ
	BOOL Sort(int ch);									// 指定チャンネルのデータを昇順に並び替える
	BOOL SortNoteData();								// ノートデータを並び替え
	BOOL CheckIsBackMedia();							// 背景メディアデータが登録されているかチェック

	BOOL Restart(void);									// リスタート用にオブジェのフラグを元に戻す
	LONG GetCountFromTime(double sec);					// 時間からBMSカウント値を計算
	

public:
	// 内部データのアクセス
	inline const char*			GetLastError(void) { return mLastError; }				// 最後のエラー文字列

	// ゲーム必須メソッド
	inline LONG					GetMaxCount(void) { return mBH.lMaxCount; }			// ゲーム内の最大のカウント値
	inline int					GetBarNum(void) { return mBH.lEndBar+1; }			// 小節バーの個数(最後の小節も含むため+1する)
	inline const LPBMSBAR		GetBar(int num) { return &mBmsBar[num]; }			// 小節バーのデータ
	inline int					GetObjeNum(int ch) { return iBmsData[ch]; }			// 指定チャネルのデータ数を返す
	inline const LPBMSDATA		GetObje(int ch, int num) { return &pBmsData[ch][num]; }		// チャネルと配列番号でデータを取得する
	inline const LPBMSHEADER	GetHeader(void) { return &mBH; }					// ヘッダ情報を返す
	inline const char*			GetWavFile(int num) { return mWavFile[num]; }			// 使用しているWAVファイル名
	inline LPBACKMEDIA			GetBackMedia(int index) { return &mBackMedia[index]; }
	inline void					ResetBkmediaType(int index) { mBackMedia[index].type = BKMEDIATYPE_NONE; }		// 背景メディアの種類をリセット
	inline const LPLANE			GetLane(int num) { return &mLaneData[num]; }			// 指定した番号のノートレーン情報
	inline const LPNOTEDEFDATA	GetNoteDefData(int num) { return &mNoteDefData[num]; }			// 指定した番号のノートデータを返す	
	inline const LPBMSNOTE		GetBmsNote(int num) { return &pNoteData[num]; }			// 指定した番号のノート配置データを返す
	inline int					GetBmsNoteCnt(void)	{ return iNoteData; }					// ノートデータの数を返す
	inline int					GetBmsNoteJudgeCnt(void) {return iNoteData+iHoldNote;}		// ノートの判定数を返す
	inline DIFFICULTY			GetStoredDataKind(void) {return eStoredDataKind;}
	

private:
	// 非公開関数
	int	 atoi1610(const char *s);							// 16進数文字列を数値に変換
	BOOL itoa1016(int num, char *dst, int keta=-1);			// 10進数を桁付きの16進数文字へ変換
	int  atoi3610(const char *s);							// 36進数文字列を数値に変換
	int  itoa1036(int num, char *dst);						// 10進数を36進数文字列へ変換
	BOOL AddData(int ch, LONG cnt, LONG data);				// 1つのデータを追加（ソートはされない）
	int	 GetCommand(const char *s);						// コマンド番号を返す
	BOOL GetCommandString(const char *src, char *dst);		// パラメータ文字列を取得
	BOOL LineCompact(const char *src, char *dst);			// データを最適化して返す
};


typedef class CBmsPro CBMSPRO, *LPCBMSPRO;

#endif
