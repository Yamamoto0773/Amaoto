#pragma warning( disable : 4996 )
#include "CBmsPro.h"
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
//#define DEBUGMODE
#include "DEBUG.H"

#define SAFE_FREE(x)		{ if(x) { free(x); x=NULL; } }

////////////////////////////////////////////
// コンストラクタ
////////////////////////////////////////////
CBmsPro::CBmsPro(void) {
	ZeroMemory(&mBH, sizeof(mBH));
	ZeroMemory(&pBmsData, sizeof(pBmsData));
	ZeroMemory(&iBmsData, sizeof(iBmsData));
	ZeroMemory(&pNoteData, sizeof(pNoteData));
	ZeroMemory(&mWavFile, sizeof(mWavFile));
	ZeroMemory(&mBackMedia, sizeof(mBackMedia));
	ZeroMemory(&mBmsBar, sizeof(mBmsBar));
	ZeroMemory(&mLaneData, sizeof(mLaneData));
	ZeroMemory(&mNoteDefData, sizeof(mNoteDefData));
	ZeroMemory(&mLastError, sizeof(mLastError));
	iNoteData = 0;
	iHoldNote = 0;

	// デフォルト値セット
	ClearAll();
}

////////////////////////////////////////////
// デストラクタ
////////////////////////////////////////////
CBmsPro::~CBmsPro() {
	ClearAll();
}

////////////////////////////////////////////
// データの初期化
////////////////////////////////////////////
BOOL CBmsPro::ClearAll(void) {
	int i;

	// ヘッダ初期化
	ZeroMemory(&mBH, sizeof(mBH));

	// 小節の長さを1.0で初期化
	ZeroMemory(&mBmsBar, sizeof(mBmsBar));
	for (int i = 0; i < 1001; i++) {
		mBmsBar[i].fScale = 1.0f;
	}

	mBH.fBpm = 130;
	for (i = 0; i < BMS_MAXINDEX; i++) {
		mBH.fBpmIndex[i] = 120.0f;
	}

	SAFE_FREE(pBmsData[BMS_STRETCH]);
	iBmsData[BMS_STRETCH] = 0;
	SAFE_FREE(pBmsData[BMS_TEMPO]);
	iBmsData[BMS_TEMPO] = 0;

	if (!ClearObjeData())
		return FALSE;

	return TRUE;
}


////////////////////////////////////////////
//譜面データを初期化(ヘッダーデータ以外を初期化)
BOOL CBmsPro::ClearObjeData(void) {
	int i;

	// 実データ初期化
	for (i = 0; i < BMS_MAXCHANNEL; i++) {
		if (!(i == BMS_STRETCH || i == BMS_TEMPO)) {
			SAFE_FREE(pBmsData[i]);		// BMSデータ領域をクリア
			iBmsData[i] = 0;			// データの数をクリア
		}
	}

	// ファイル名
	ZeroMemory(&mWavFile, sizeof(mWavFile));

	ZeroMemory(&mBackMedia, sizeof(mBackMedia));
	for (i=0; i<BMS_MAXINDEX; i++) {
		mBackMedia[i].type = BKMEDIATYPE_NONE;
	}

	// ノートレーン情報の初期化
	ZeroMemory(&mLaneData, sizeof(mLaneData));
	for (i=0; i<BMS_MAXNOTELANE; i++) {
		mLaneData[i].iColor[0] = 255;
		mLaneData[i].iColor[1] = 255;
		mLaneData[i].iColor[2] = 255;
		mLaneData[i].iColor[3] = 150;
		mLaneData[i].fBottomX = -1.0f;
		mLaneData[i].fTopX = -1.0f;
		ZeroMemory(&mLaneData[i].mName, sizeof(mLaneData[i].mName));
	}

	// ノート配置データの初期化
	SAFE_FREE(pNoteData);
	iNoteData = 0;

	// ノート定義データ情報の初期化
	ZeroMemory(&mNoteDefData, sizeof(mNoteDefData));
	for (i=0; i<BMS_MAXINDEX; i++) {
		mNoteDefData[i].fScore = 1.0f;
		mNoteDefData[i].fSize = 1.0f;
		mNoteDefData[i].fVol = 1.0f;
	}

	iHoldNote = 0;

	eStoredDataKind = DIFF_NONE;

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
// ヘッダ情報だけを取り出す
////////////////////////////////////////////////////////////////////////////////////////
BOOL CBmsPro::LoadHeader(const char *file) {
	ClearAll();

	FILE *fp;

	fp = fopen(file, "r");
	if (!fp) {
		DEBUG("ファイルが見つからない : %s\n", file);
		sprintf(mLastError, "[%s] ファイルが見つからない", file);
		return FALSE;
	}
	// ファイルパスを保存
	strcpy(mBH.mDataFilePath, file);


	char buf[1024];
	char tmp[5];
	int num;
	int ch;
	int line;
	int i;
	char *c;
	bool flag = FALSE;

	while (true) {
		// 1行を読みこむ
		ZeroMemory(buf, 1024);
		fgets(buf, 1024, fp);
		if (buf[0] == NULL && feof(fp))	// ファイルの終端なら検索終わり
			break;

		// コマンド以外なら飛ばす
		if (buf[0] != '#')
			continue;

		// 最後の改行を消去
		if (buf[strlen(buf) - 1] == '\n')
			buf[strlen(buf) - 1] = NULL;

		// コマンドの解析
		int cmd = GetCommand(buf);


		// 不明なコマンドならスキップ
		if (cmd <= -2) {
			DEBUG("不明なコマンド : %s\n", buf);
			continue;
		}

		// パラメータの分割
		char str[1024];
		ZeroMemory(str, 1024);
		if (!GetCommandString(buf, str)) {
			// 文字列の取得が失敗なら
			fclose(fp);
			sprintf(mLastError, "[%s] パラメータの分割エラー\n", buf);
			return FALSE;
		}


		// HEADER定義領域であるかチェック
		if (cmd == 16) {
			// #DEFINESTART
			if (strncmp("HEADER", str, 6) == 0) {
				flag = TRUE;
				continue;
			}
		}

		if (cmd == 18) {
			// #DEFINEEND
			if (strncmp("HEADER", str, 6) == 0) {
				break;
			}
		}

		if (!flag)
			continue;



		// パラメータの代入
		ZeroMemory(tmp, sizeof(tmp));
		switch (cmd) {
			case 0:		// GENRE
				tmp[0] = buf[6];
				tmp[1] = buf[7];
				tmp[2] = NULL;
				num = atoi3610(tmp);		// インデックス番号の取得
				if (num >= BMS_MAXGENRE) {
					DEBUG("#GENREのインデックス番号が上限を超えています\n");
					break;
				}

				// ジャンル番号を取得
				if (stricmp(str, "ORIGINAL") == 0) {
					mBH.eGenre[num] = MUSICGENRE::ORIGINAL;
				}
				else if (stricmp(str, "VARIETY") == 0) {
					mBH.eGenre[num] = MUSICGENRE::VARIETY;
				}
				else if (stricmp(str, "POPS") == 0) {
					mBH.eGenre[num] = MUSICGENRE::POPS;
				}
				else if (stricmp(str, "GAME") == 0) {
					mBH.eGenre[num] = MUSICGENRE::GAME;
				}
				else if (stricmp(str, "VOCALOID") == 0) {
					mBH.eGenre[num] = MUSICGENRE::VOCALOID;
				}
				else if (stricmp(str, "ANIME") == 0) {
					mBH.eGenre[num] = MUSICGENRE::ANIME;
				}
				else {
					DEBUG("登録されていないジャンル名です。\"%s\"\n", buf);
					return FALSE;
				}

				DEBUG("ジャンル%d		[%s]\n", num, str);
				break;
			case 1:		// TITLE
				strcpy(mBH.mTitle, str);
				DEBUG("タイトル     [%s]\n", mBH.mTitle);
				break;
			case 2:		// SUBTITLE
				tmp[0] = buf[9];
				tmp[1] = buf[10];
				tmp[2] = NULL;
				num = atoi3610(tmp);		// インデックス番号取得
				itoa1036(num, tmp);
				if (num >= BMS_MAXSUBTITLE) {
					DEBUG("#SUBTITLEのインデックス番号が上限を超えています\n");
					break;
				}
				strcpy(mBH.mSubTitle[num], str);
				DEBUG("SUBTITLE%s \"%s\"\n", tmp, mBH.mSubTitle[num]);
				break;
			case 3:		// ARTIST
				strcpy(mBH.mArtist, str);
				DEBUG("アーティスト [%s]\n", mBH.mArtist);
				break;
			case 4:		// BPM
				if (buf[4] == ' ' || buf[4] == 0x09) {
					// 基本コマンドなら
					mBH.fBpm = (float)atof(str);
					AddData(BMS_TEMPO, 0, (LONG)mBH.fBpm);
					DEBUG("初期BPM      [%.2f]\n", mBH.fBpm);
				}
				else {
					// 拡張コマンドなら
					tmp[0] = buf[4];
					tmp[1] = buf[5];
					tmp[2] = NULL;
					num = atoi3610(tmp);	// 16進数
					itoa1036(num, tmp);
					mBH.fBpmIndex[num] = (float)atof(str);
					DEBUG("インデックスBPM : %s [%f]\n", tmp, mBH.fBpmIndex[num]);
				}
				break;
			case 5:		// MIDIFILE
				strcpy(mBH.mMidifile, str);
				DEBUG("MIDIFILE \"%s\"\n", mBH.mMidifile);
				break;
			case 6:		// PLAYLEVEL
				if ((c = strtok(str, " ,/")) == NULL) {
					fclose(fp);
					DEBUG("#PLAYLEVELパラメーターの書式エラー\"%s\"\n", str);
					return FALSE;
				}
				mBH.iPlaylevel[DIFF_ELEMENTARY] = atoi(c);
				i=1;
				while (i<4) {
					if ((c = strtok(NULL, " ,/")) == NULL) {
						fclose(fp);
						DEBUG("#PLAYLEVELパラメーターの書式エラー\"%s\"\n", str);
						return FALSE;
					}
					mBH.iPlaylevel[i] = atoi(c);
					i++;
				}
				DEBUG("難易度	:%d %d %d %d\n", mBH.iPlaylevel[DIFF_ELEMENTARY], mBH.iPlaylevel[DIFF_INTERMEDIATE], mBH.iPlaylevel[DIFF_ADVANCED], mBH.iPlaylevel[DIFF_MASTER]);
				break;
			case 7:		// VOLWAV
				mBH.fWavVol = (float)atof(str);
				DEBUG("WAVデータの音量 %d%\n", mBH.fWavVol);
				break;

			case 17:	// JACKET
				strcpy(mBH.mJacket, str);
				DEBUG("JACKET \"%s\"\n", mBH.mJacket);
				break;
			case 19:
				// ID
				mBH.iID = atoi(str);
				break;
			case -1:
				// #aaabb形式データ

				// 小節番号の取得
				ZeroMemory(&tmp, sizeof(tmp));
				memcpy(tmp, buf + 1, 3);
				line = atoi(tmp);			// 10進数

				// チャンネル番号の取得
				ZeroMemory(&tmp, sizeof(tmp));
				tmp[0] = buf[4];
				tmp[1] = buf[5];
				ch = atoi3610(tmp);		// 16進数


				if (ch == BMS_STRETCH) {
					// 小節の倍率変更命令の場合
					mBmsBar[line].fScale = (float)atof(str);
					DEBUG("小節倍率変更 : %f倍\n", mBmsBar[line].fScale);
				}


				// 小節番号の最大値を記憶する
				if (mBH.lEndBar < line)
					mBH.lEndBar = line;
				break;
		}
	}

	fclose(fp);

	if (!flag) {
		// 定義領域がなかったら
		return FALSE;
	}


	// 最後の小節内にもデータが存在するため、その次の小節を終端小節とする
	mBH.lEndBar++;

	// 小節倍率データを元に全ての小節情報を算出
	LONG cnt = 0;	// 現在の小節の開始カウント値
	for (i = 0; i <= mBH.lEndBar; i++) {
		// 小節リストを加算
		mBmsBar[i].lTime = cnt;												// 現在の小節の開始カウントを記録
		mBmsBar[i].lLength = (LONG)(BMS_RESOLUTION * mBmsBar[i].fScale);	// 倍率からこの小節の長さカウント値を算出

		cnt += mBmsBar[i].lLength;	// この小節のカウント数を加算して次の小節の開始カウントとする
	}

	// 最大カウントを保存
	mBH.lMaxCount = cnt;


	return TRUE;
}



BOOL CBmsPro::LoadObjeData(DIFFICULTY diff) {
	// オブジェデータを全てクリア
	ClearObjeData();

	// ヘッダーデータがロードされているかチェック
	if (!mBH.mDataFilePath)
		return FALSE;

	// ファイルを開く
	FILE *fp;
	fp = fopen(mBH.mDataFilePath, "r");
	if (!fp) {
		DEBUG("ファイルが見つからない : %s\n", mBH.mDataFilePath);
		sprintf(mLastError, "[%s] ファイルが見つからない", mBH.mDataFilePath);
		return FALSE;
	}


	// パターンマッチ用の難易度文字列を作成
	char diffStr[13];
	switch (diff) {
		case DIFF_ELEMENTARY:
			strncpy(diffStr, "ELEMENTARY", 11);
			break;
		case DIFF_INTERMEDIATE:
			strncpy(diffStr, "INTERMEDIATE", 13);
			break;
		case DIFF_ADVANCED:
			strncpy(diffStr, "ADVANCED", 9);
			break;
		case DIFF_MASTER:
			strncpy(diffStr, "MASTER", 7);
			break;
		default:
			break;
	}



	char buf[1024];
	char tmp[4];
	int num;
	int ch;
	int line;
	int i, j;
	char *c;
	bool flag = FALSE;

	int len;
	LONG tick;
	BMSDATA holdEndData[BMS_MAXINDEX];	// ホールド終点ノートを一時保存する配列
	int holdCnt = 0;
	ZeroMemory(holdEndData, sizeof(holdEndData));

	while (true) {
		// 1行を読みこむ
		ZeroMemory(buf, 1024);
		fgets(buf, 1024, fp);
		if (buf[0] == NULL && feof(fp))	// ファイルの終端なら検索終わり
			break;

		// コマンド以外なら飛ばす
		if (buf[0] != '#')
			continue;

		// 最後の改行を消去
		if (buf[strlen(buf) - 1] == '\n')
			buf[strlen(buf) - 1] = NULL;

		// コマンドの解析
		int cmd = GetCommand(buf);

		// 不明なコマンドならスキップ
		if (cmd <= -2) {
			DEBUG("不明なコマンド : %s\n", buf);
			continue;
		}

		// パラメータの分割
		char str[1024];
		ZeroMemory(str, 1024);
		if (!GetCommandString(buf, str)) {
			// 文字列の取得が失敗なら
			fclose(fp);
			sprintf(mLastError, "[%s] パラメータの分割エラー", buf);
			return FALSE;
		}

		// 定義領域であるかチェック
		if (cmd == 16) {
			// #DEFINESTART

			if (strncmp(diffStr, str, strlen(diffStr)) == 0) {
				flag = TRUE;
				continue;
			}
		}
		if (cmd == 18) {
			// #DEFINEEND
			if (strncmp(diffStr, str, strlen(diffStr)) == 0) {
				break;
			}
		}
		if (!flag)
			continue;



		// パラメータの代入
		ZeroMemory(tmp, sizeof(tmp));
		switch (cmd) {
			case 8:		// WAV
				tmp[0] = buf[4];
				tmp[1] = buf[5];
				num = atoi3610(tmp);			// 16進数
				strcpy(mWavFile[num], str);
				itoa1036(num, tmp);
				DEBUG("WAV%s : %s\n", tmp, mWavFile[num]);

				break;
			case 9:		// LANEPOS
				tmp[0] = buf[8];
				tmp[1] = buf[9];
				tmp[2] = NULL;
				num = atoi3610(tmp);			// インデックス番号を16進数から10進数に変換
				itoa1036(num, tmp);
				char *c1, *c2;
				if ((c1 = strtok(str, " ,:")) == NULL || (c2 = strtok(NULL, " ,:")) == NULL) {
					fclose(fp);
					DEBUG("#LANEPOS%sパラメーターの書式エラー \"%s\"\n", tmp, str);
					return FALSE;
				}
				mLaneData[num].fTopX = (float)atof(c2);
				mLaneData[num].fBottomX = (float)atof(c1);
				DEBUG("LANEPOS%s : %f %f\n", tmp, mLaneData[num].fBottomX, mLaneData[num].fTopX);
				break;
			case 10:	// LANECOLOR
				tmp[0] = buf[10];
				tmp[1] = buf[11];
				tmp[2] = NULL;
				num = atoi3610(tmp);
				itoa1036(num, tmp);
				if ((c = strtok(str, " ,/")) == NULL) {
					fclose(fp);
					DEBUG("#LANECOLOR%sパラメーターの書式エラー\"%s\"\n", tmp, str);
					return FALSE;
				}
				mLaneData[num].iColor[0] = atoi(c);
				i=1;
				while (i<4) {
					if ((c = strtok(NULL, " ,/")) == NULL) {
						fclose(fp);
						DEBUG("#LANECOLOR%sパラメーターの書式エラー\"%s\"\n", tmp, str);
						return FALSE;
					}
					mLaneData[num].iColor[i] = atoi(c);
					i++;
				}
				DEBUG("LANE COLOR%s : R[%d] G[%d] B[%d] A[%d]\n", tmp, mLaneData[num].iColor[0], mLaneData[num].iColor[1], mLaneData[num].iColor[2], mLaneData[num].iColor[3]);
				break;
			case 11:	// LANENAME
				tmp[0] = buf[9];
				tmp[1] = buf[10];
				tmp[2] = NULL;
				num = atoi3610(tmp);
				itoa1036(num, tmp);
				strcpy(mLaneData[num].mName, str);;
				DEBUG("LANE NAME%s  \"%s\"\n", tmp, mLaneData[num].mName);
				break;
			case 12:	// NOTESOUND
				tmp[0] = buf[10];
				tmp[1] = buf[11];
				tmp[2] = NULL;
				num = atoi3610(tmp);
				itoa1036(num, tmp);
				mNoteDefData[num].lSound = atoi3610(str);
				DEBUG("NOTE SOUND%s  %02X\n", tmp, mNoteDefData[num].lSound);
				break;
			case 13:	// NOTESCORE
				tmp[0] = buf[10];
				tmp[1] = buf[11];
				tmp[2] = NULL;
				num = atoi3610(tmp);
				itoa1036(num, tmp);
				mNoteDefData[num].fScore = (float)atof(str);
				DEBUG("NOTE SCORE%s x%4.3f\n", tmp, mNoteDefData[num].fScore);
				break;
			case 14:	// NOTESIZE
				tmp[0] = buf[9];
				tmp[1] = buf[10];
				tmp[2] = NULL;
				num = atoi3610(tmp);
				itoa1036(num, tmp);
				mNoteDefData[num].fSize = (float)atof(str);
				DEBUG("NOTE SIZE%s %4.3f\n", tmp, mNoteDefData[num].fSize);
				break;
			case 15:	// NOTECOLOR
				tmp[0] = buf[10];
				tmp[1] = buf[11];
				tmp[2] = NULL;
				num = atoi3610(tmp);
				itoa1036(num, tmp);
				if ((c = strtok(str, " ,/")) == NULL) {
					fclose(fp);
					DEBUG("#NOTECOLOR%sパラメーターの書式エラー\"%s\"\n", tmp, str);
					return FALSE;
				}
				mNoteDefData[num].iColor[0] = atoi(c);
				i=1;
				while (i<4) {
					if ((c = strtok(NULL, " ,/")) == NULL) {
						fclose(fp);
						DEBUG("#NOTECOLOR%sパラメーターの書式エラー\"%s\"\n", tmp, str);
						return FALSE;
					}
					mNoteDefData[num].iColor[i] = atoi(c);
					i++;
				}
				DEBUG("NOTE COLOR%s : R[%d] G[%d] B[%d] A[%d]\n", tmp, mNoteDefData[num].iColor[0], mNoteDefData[num].iColor[1], mNoteDefData[num].iColor[2], mNoteDefData[num].iColor[3]);
				break;
			case 20:	// NOTEVOL
				tmp[0] = buf[8];
				tmp[1] = buf[9];
				tmp[2] = '\0';
				num = atoi3610(tmp);
				itoa1036(num, tmp);
				mNoteDefData[num].fVol = (float)atof(str);
				DEBUG("#NOTE VOL%s %f\n", tmp, mNoteDefData[num].fVol);
				break;
			case 21:	// BACKMEDIA
				// 拡張子で判別
				ZeroMemory(&tmp, sizeof(tmp));
				tmp[0] = buf[10];
				tmp[1] = buf[11];
				tmp[2] = '\0';
				num = atoi3610(tmp);
				ZeroMemory(&tmp, sizeof(tmp));
				i=0, j=strlen(str)-1;
				while (str[j] != '.') tmp[i++] = str[j--];
				tmp[i] = '\0';
				{
					j = strlen(tmp)-1;
					for (i=0; i<strlen(tmp); i++) {
						if (i >= j) break;
						char buf = tmp[i];
						tmp[i] = tmp[j];
						tmp[j] = buf;
						j--;
					}

					const char *pattern[] ={ "avi", "mp4", "m4a", "mov", "qt", "m2ts", "ts", "mpeg", "mpg",
					"mkv", "wmv", "asf", "flv", "f4v", "asf", "webm", "ogm", "bmp", "tga", "dds", "dib", "jpg", "png" };
					for (i=0; i<23; i++) {
						if (strnicmp(tmp, pattern[i], 3) == 0) break;
					}
					if (i >= 17)  mBackMedia[num].type = BKMEDIATYPE_IMAGE;
					else mBackMedia[num].type = BKMEDIATYPE_MOVIE;
				}
				strncpy(mBackMedia[num].mFileName, str, MAX_PATH);
				ZeroMemory(&tmp, sizeof(tmp));
				itoa1036(num, tmp);
				DEBUG("#BACKGROUMD%s %s", tmp, mBackMedia[num].mFileName);
				break;
			case -1:
				// #aaabb 形式データ

				// 小節番号の取得
				ZeroMemory(&tmp, sizeof(tmp));
				memcpy(tmp, buf + 1, 3);
				line = atoi(tmp);			// 10進数

				// チャンネル番号の取得
				ZeroMemory(&tmp, sizeof(tmp));
				tmp[0] = buf[4];
				tmp[1] = buf[5];
				ch = atoi3610(tmp);		// 16進数

				if (ch == BMS_STRETCH)
					continue;


				// データが存在するかチェック
				if (strlen(str) < 1) {
					DEBUG("データが定義されていない箇所が存在\n%s", buf);
					continue;
				}

				// データが偶数かチェック
				if (strlen(str) % 2 == 1) {
					DEBUG("データが偶数個ではない [%s]\n", buf);
					fclose(fp);
					sprintf(mLastError, "[%s] データが偶数個ではない", buf);
					return FALSE;
				}

				// データ数
				len = (int)strlen(str) / 2;

				// 現在の小節のカウント値から1音符分のカウント値を算出
				tick = mBmsBar[line].lLength / len;


				// 実データを追加
				ZeroMemory(&tmp, sizeof(tmp));

				for (i = 0; i < len; i++) {
					tmp[0] = str[i * 2];
					tmp[1] = str[i * 2 + 1];
					int data = atoi3610(tmp);			// 16進数
					if (data > 0) {
						// データが存在する場合
						if (ch >= BMS_NOTE && data == BMS_HOLDENDNOTE) {
							// ホールド終点ノートなら
							holdEndData[holdCnt].lData = ch - BMS_NOTE;
							holdEndData[holdCnt].lTime = mBmsBar[line].lTime + (tick * i);
							holdCnt++;
						}
						else {
							AddData(ch, mBmsBar[line].lTime + (tick * i), data);
						}
					}
				}


				break;	// case -1:
		} // switch
	} // wihle

	fclose(fp);

	if (!flag) {
		// 定義領域がなかったら
		return FALSE;
	}



	//// 実データの最適化 /////
	// ソート
	for (i = 0; i < BMS_MAXCHANNEL; i++)
		Sort(i);
	SortNoteData();

	// ホールドノートにデータを格納
	for (i=0; i<holdCnt; i++) {
		for (j=iNoteData-1; j>=0; j--) {
			if (holdEndData[i].lData == pNoteData[j].iLane) {
				// ホールド始点ノートと同じレーンなら
				if (pNoteData[j].lTime1 < holdEndData[i].lTime) {
					// ホールド終点ノートの前にあるノートなら
					if (pNoteData[j].eType == HIT) {
						// ヒットノートなら
						pNoteData[j].eType = NOTETYPE::HOLD;
						pNoteData[j].lTime2 = holdEndData[i].lTime;
						pNoteData[j].bFlag2 = TRUE;
						break;
					}
					// ホールドノートなら
					DEBUG("ホールド終点ノートが連続して定義されています。 lane:%ld time:%ld\n", holdEndData[i].lData, holdEndData[i].lTime);
					return FALSE;
				}
			}

			if (j==0) {
				DEBUG("ホールド始点ノートが見つかりません。 lane:%ld time:%ld\n", holdEndData[i].lData, holdEndData[i].lTime);
				return FALSE;
			}
		}
	}

	iHoldNote = holdCnt;
	eStoredDataKind = diff;

	return TRUE;
}


////////////////////////////////////////////
// データロード
////////////////////////////////////////////
BOOL CBmsPro::Load(const char *file, DIFFICULTY diffID) {
	// ヘッダ＆小節倍率の読み込み
	if (!LoadHeader(file)) {
		DEBUG("ヘッダ読み取りエラー \"%s\"\n", file);
		return FALSE;
	}
	DEBUG("HEADER OK\n");

	// 実データの読み込み
	if (!LoadObjeData(diffID)) {
		DEBUG("データ読み込みエラー \"%s\"\n", file);
		return FALSE;
	}
	DEBUG("DATA OK\n");

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
// BMSファイルにセーブ
////////////////////////////////////////////////////////////////////////////////////////
BOOL CBmsPro::Save(const char *file) {
	FILE *fp;
	fp = fopen(file, "wb");
	if (!fp) {
		sprintf(mLastError, "[%s] ファイル書き込みエラー", file);
		return FALSE;
	}

	// ヘッダの書き出し
	fprintf(fp, "; charatsoft.com BMS export library v1.00\n");
	fprintf(fp, "\n");
	//fprintf(fp, "#GENRE %s\n", mBH.eGenre);
	fprintf(fp, "#TITLE %s\n", mBH.mTitle);
	fprintf(fp, "#ARTIST %s\n", mBH.mArtist);
	fprintf(fp, "#BPM %5.2f\n", mBH.fBpm);
	fprintf(fp, "#PLAYLEVEL %d %d %d %d\n", mBH.iPlaylevel[DIFF_ELEMENTARY], mBH.iPlaylevel[DIFF_INTERMEDIATE], mBH.iPlaylevel[DIFF_ADVANCED], mBH.iPlaylevel[DIFF_MASTER]);

	if (strlen(mBH.mJacket) > 0)	fprintf(fp, "#STAGEFILE %s\n", mBH.mJacket);
	if (strlen(mBH.mMidifile) > 0)	fprintf(fp, "#MIDIFILE %s\n", mBH.mMidifile);

	fprintf(fp, "\n");

	int i;
	char buf[1024];

	// WAVファイルの書き出し
	for (i = 0; i < BMS_MAXINDEX; i++) {
		if (mWavFile[i][0]) {
			// NULLで無ければ書き出す
			itoa1016(i, buf, 2);
			fprintf(fp, "#WAV%s %s\n", buf, mWavFile[i]);
		}
	}
	fprintf(fp, "\n");

	// ノートレーン情報の書き出し
	//for (i = 0; i < BMS_MAXNOTELANE; i++) {
	//	if (mLaneData[i].fBottomX != -1.0f && mLaneData[i].fTopX != -1.0f) {
	//		// 初期値でなければ書き出す
	//		itoa1016(i, buf, 2);
	//		fprintf(fp, "#NOTELANE%s %f %f", buf, mLaneData[i].fTopX, mLaneData[i].fBottomX);
	//	}
	//}


	// 出力するチャンネルリスト
	static const int ch[] ={
		BMS_BACKMUSIC,
		//		BMS_TEMPO,
				BMS_BACKMEDIA,
		//		BMS_BPMINDEX,
				0x11,0x12,0x13,0x14,0x15,0x16,0x18,0x19,
				0x21,0x22,0x23,0x24,0x25,0x26,0x28,0x29,
				-1,
	};

	// データの書き出し
	int n = 0;
	int index;
	int line;
	char s[1024];
	char dst[1024];

	while (ch[n] != -1) {

		index = 0;
		line = 0;

		// 全小節の処理
		while (index < iBmsData[ch[n]]) {
			strcpy(s, "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");

			// 現在の小節内のデータを処理
			while (index < iBmsData[ch[n]]) {
				if ((line + 1)*BMS_RESOLUTION <= pBmsData[ch[n]][index].lTime)
					break;

				// 時間から書き込み先アドレスを算出
				int addr = (int)(pBmsData[ch[n]][index].lTime - line*BMS_RESOLUTION);
				int fix_addr = (int)((float)addr / (BMS_RESOLUTION / 192)) * 2;

				// 既に１つ前の情報があるか
				if (s[fix_addr] != '0' || s[fix_addr + 1] != '0') {
					// 既にある場合は現在までのデータを書き込んでおく
					if (LineCompact(s, dst))
						fprintf(fp, "#%03d%02X:%s\n", line, ch[n], dst);
					strcpy(s, "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
				}

				// データをセット
				itoa1016(pBmsData[ch[n]][index].lData, buf, 2);
				memcpy(&s[fix_addr], buf, 2);

				index++;
			}

			// 残りを書き出す
			if (strcmp(s, "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000") != 0) {
				if (LineCompact(s, dst)) {
					fprintf(fp, "#%03d%02X:%s\n", line, ch[n], dst);
				}
			}

			line++;
		}

		fprintf(fp, "\n");

		n++;
	}

	fclose(fp);
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
// 指定チャンネルのデータを昇順に並び替える
////////////////////////////////////////////////////////////////////////////////////////
BOOL CBmsPro::Sort(int ch) {
	if (ch<0 || ch>BMS_MAXINDEX - 1)
		return FALSE;

	// 昇順に並び替える
	int i, j;
	for (i = 0; i < iBmsData[ch] - 1; i++) {
		for (j = i + 1; j<iBmsData[ch]; j++) {
			if (pBmsData[ch][i].lTime > pBmsData[ch][j].lTime) {
				// 構造体を入れ替える
				BMSDATA dmy = pBmsData[ch][i];		// ダミーに保存
				pBmsData[ch][i] = pBmsData[ch][j];		// iにjを入れる
				pBmsData[ch][j] = dmy;					// jにダミーを入れる
			}
		}
	}
	return TRUE;
}

BOOL CBmsPro::SortNoteData() {
	int i, j;
	for (i=0; i<iNoteData-1; i++) {
		for (j=i+1; j<iNoteData; j++) {
			if (pNoteData[i].lTime1 > pNoteData[j].lTime1) {
				BMSNOTE n = pNoteData[i];
				pNoteData[i] = pNoteData[j];
				pNoteData[j] = n;
			}
		}
	}

	return TRUE;
}

BOOL CBmsPro::CheckIsBackMedia() {
	BOOL res = FALSE;
	for (int i=0; i<BMS_MAXINDEX; i++) {
		if (mBackMedia[i].type != BKMEDIATYPE_NONE) {
			res = TRUE;
			break;
		}
	}

	return res;
}


////////////////////////////////////////////////////////////////////////////////////////
// リスタート用にオブジェのフラグを元に戻す
////////////////////////////////////////////////////////////////////////////////////////
BOOL CBmsPro::Restart(void) {
	int i;

	for (i=0; i<iNoteData; i++) {
		pNoteData[i].bFlag1 = TRUE;
		if (pNoteData[i].eType == HOLD)
			pNoteData[i].bFlag2 = TRUE;
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
// 時間からBMSカウント値を計算
////////////////////////////////////////////////////////////////////////////////////////
LONG CBmsPro::GetCountFromTime(double sec) {
	LONG cnt = 0;			// BMSカウント
	double t = 0;			// BMS上の時間
	double bpm = 130;

	if (iBmsData[BMS_TEMPO] > 0) {
		bpm = pBmsData[BMS_TEMPO][0].fData;		// 初期BPM
	}

	if (sec < 0)
		return 0;

	//	DEBUG( "■指定時間 (%.2f秒)\n",sec );

		// 指定時間を越えるまでタイムを加算
	int i;
	for (i = 0; i < iBmsData[BMS_TEMPO]; i++) {

		// １つ前の時間と新しい時間との経過時間から秒を算出
		double add = (double)(pBmsData[BMS_TEMPO][i].lTime - cnt) / (bpm / 60) / (BMS_RESOLUTION / 4);
		//		DEBUG( "  [%d] 経過時間 %f秒\n",i,t+add );

				// 現在のテンポ値で時間が過ぎたら抜ける
		if (t + add > sec) {
			//			DEBUG( "                  └時間が過ぎた\n" );
			break;
		}

		t += add;										// 経過時間を加算
		bpm = (double)pBmsData[BMS_TEMPO][i].fData;		// 次のBPMをセット
		cnt = pBmsData[BMS_TEMPO][i].lTime;			// 計算済みのカウントをセット
	}

	//	DEBUG( "  BPM %f\n",bpm );
	//	DEBUG( "  CNT %I64d\n",cnt );

		// 指定時間と1つ前までの時間の差分
	double sub = sec - t;
	//	DEBUG( "  差分 %f秒\n",sub );

		// 差分からBMSカウント数を算出
	LONG cnt2 = (LONG)(sub * (BMS_RESOLUTION / 4) * (bpm / 60));

	// BMSカウント値に加算
	cnt += cnt2;
	//	DEBUG( "  結果のカウント値 %I64d\n",cnt );

	return cnt;
}










////////////////////////////////////////////////////////////////////////////////////////
// 16進数文字列を数値に変換
////////////////////////////////////////////////////////////////////////////////////////
int CBmsPro::atoi1610(const char *s) {
	int ret = 0;			// 10進数に変換した値
	int i = 0;				// 参照する文字配列
	while (s[i]) {
		if (!(s[i] >= '0' && s[i] <= '9') &&
			!(s[i] >= 'A' && s[i] <= 'Z') &&
			!(s[i] >= 'a' && s[i] <= 'z'))
			return 0;

		ret *= 16;				// 16倍
		int n = s[i] - '0';
		if (n > 9)
			n -= 7;
		if (n > 15)
			n -= 0x20;
		ret += n;
		i++;
	}
	return ret;
}

////////////////////////////////////////////////////////////////////////////////////////
// 10進数を桁付きの16進数文字へ変換
////////////////////////////////////////////////////////////////////////////////////////
BOOL CBmsPro::itoa1016(int num, char *dst, int keta) {
	if (num < 0)
		return FALSE;

	itoa(num, dst, 16);

	// 小文字チェック
	int i;
	for (i = 0; i < (int)strlen(dst); i++) {
		if (dst[i] >= 'a' && dst[i] <= 'z') {
			dst[i] -= 0x20;
		}
	}

	// 桁数指定の場合
	if (keta >= 0) {
		// 桁数が足りない場合
		if ((int)strlen(dst) < keta) {
			char dmy[1024];
			ZeroMemory(dmy, sizeof(dmy));
			for (i = 0; i < keta - (int)strlen(dst); i++) {
				dmy[i] = '0';			// 先頭に足りない分の0を追加
			}
			strcpy(&dmy[i], dst);		// その次に算出された文字列をつなげる
			strcpy(dst, dmy);			// 出力結果にダミー文字列をコピー		
		}
	}

	return TRUE;
}



int CBmsPro::atoi3610(const char * s) {
	int i=0;
	int ret = 0;

	while (s[i] != '\0') {
		if (!('0' <= s[i] && s[i] <= '9') &&
			!('a' <= s[i] && s[i] <= 'z') &&
			!('A' <= s[i] && s[i] <= 'Z')) {
			return 0;
		}

		ret *= 36;
		if (s[i] >= 'a') ret += (s[i] - 'a'+10);
		else if (s[i] >= 'A') ret += (s[i] - 'A'+10);
		else ret += (s[i] - '0');

		i++;
	}

	return ret;
}



int CBmsPro::itoa1036(int num, char * dst) {
	int cnt = 1;

	while (cnt >= 0) {
		int tmp = num%36;
		if (tmp < 10) {
			dst[cnt] = (char)(tmp+'0');
		}
		else {
			dst[cnt] = (char)(tmp-10+'A');
		}

		num /= 36;

		cnt--;
	}
	dst[2] = '\0';

	return 1;
}



////////////////////////////////////////////////////////////////////////////////////////
// 1つのデータを追加
////////////////////////////////////////////////////////////////////////////////////////
BOOL CBmsPro::AddData(int ch, LONG cnt, LONG data) {
	// チャンネル番号をチェック
	if (ch < 0 || ch>255)
		return FALSE;

	// 小節長変更コマンドなら何もしない
	if (ch == BMS_STRETCH)
		return FALSE;

	// データが無ければ何もしない
	if (data == 0)
		return TRUE;



	if (ch == BMS_BPMINDEX) {
		// BPMのインデックス指定(新)
		iBmsData[BMS_TEMPO]++;
		pBmsData[BMS_TEMPO] = (LPBMSDATA)realloc(pBmsData[BMS_TEMPO], iBmsData[BMS_TEMPO] * sizeof(BMSDATA));
		ZeroMemory(&pBmsData[BMS_TEMPO][iBmsData[BMS_TEMPO] - 1], sizeof(BMSDATA));	// 追加した配列をクリア
		pBmsData[BMS_TEMPO][iBmsData[BMS_TEMPO] - 1].lTime = cnt;
		pBmsData[BMS_TEMPO][iBmsData[BMS_TEMPO] - 1].lData = (LONG)mBH.fBpmIndex[data];	// テンポリストに入っているテンポ値を登録(LONG型にも保存)
		pBmsData[BMS_TEMPO][iBmsData[BMS_TEMPO] - 1].fData = mBH.fBpmIndex[data];		// テンポリストに入っているテンポ値を登録
		pBmsData[BMS_TEMPO][iBmsData[BMS_TEMPO] - 1].bFlag = TRUE;
	}
	else if (BMS_NOTE <= ch && ch < BMS_NOTE+BMS_MAXNOTELANE) {
		// ノートデータ
		iNoteData++;
		pNoteData = (LPBMSNOTE)realloc(pNoteData, sizeof(BMSNOTE)*iNoteData);
		ZeroMemory(&pNoteData[iNoteData-1], sizeof(BMSNOTE));
		pNoteData[iNoteData-1].eType = NOTETYPE::HIT;
		pNoteData[iNoteData-1].lTime1 = cnt;
		pNoteData[iNoteData-1].lTime2 = cnt;
		pNoteData[iNoteData-1].lIndex = data;
		pNoteData[iNoteData-1].iLane = ch - BMS_NOTE;
		pNoteData[iNoteData-1].bFlag1 = TRUE;
		pNoteData[iNoteData-1].bFlag2 = FALSE;
	}
	else if (ch < BMS_MAXCHANNEL) {
		// その他のデータ
		iBmsData[ch]++;
		pBmsData[ch] = (LPBMSDATA)realloc(pBmsData[ch], iBmsData[ch] * sizeof(BMSDATA));
		ZeroMemory(&pBmsData[ch][iBmsData[ch] - 1], sizeof(BMSDATA));					// 追加した配列をクリア
		pBmsData[ch][iBmsData[ch] - 1].lTime = cnt;
		pBmsData[ch][iBmsData[ch] - 1].lData = data;
		pBmsData[ch][iBmsData[ch] - 1].fData = (float)data;								// float型にも保存
		pBmsData[ch][iBmsData[ch] - 1].bFlag = TRUE;
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
// コマンド番号を返す
// 戻り値
//		0以上 : コマンド番号
//		-1    : オブジェ配置データ
//		-2    : 不明なコマンド
////////////////////////////////////////////////////////////////////////////////////////
int CBmsPro::GetCommand(const char *s) {
	static const char *command[22] ={
		"GENRE",
		"TITLE",
		"SUBTITLE",
		"ARTIST",
		"BPM",
		"MIDIFILE",
		"PLAYLEVEL",
		"VOLWAV",
		"WAV",
		"LANEPOS",
		"LANECOLOR",
		"LANENAME",
		"NOTESOUND",
		"NOTESCORE",
		"NOTESIZE",
		"NOTECOLOR",
		"DEFINESTART",
		"JACKET",
		"DEFINEEND",
		"ID",
		"NOTEVOL",
		"BACKMEDIA"
	};

	// 検索ルーチン
	int i;
	for (i = 0; i < 22; i++) {
		if (strnicmp(s + 1, command[i], strlen(command[i])) == 0)
			return i;	// コマンドならその番号を返す
	}

	// 先頭が'#nnncc'形式か
	BOOL obj = TRUE;
	for (i = 0; i < 5; i++) {
		if (s[i + 1]<'0' || s[i + 1]>'z') {
			obj = FALSE;
			break;
		}
	}

	// オブジェ配置なら -1 
	if (obj) {
		return -1;
	}

	// 処理不可能文字列なら
	return -2;
}

////////////////////////////////////////////////////////////////////////////////////////
// コマンドの文字列部分を返す（'\n'は削除 ':'も区切りとして処理）
////////////////////////////////////////////////////////////////////////////////////////
BOOL CBmsPro::GetCommandString(const char *src, char *dst) {
	int i = 0;
	int j = 0;

	// まずソースデータからデータ部分までのポインタを算出
	while (1) {
		if (src[i] == ' ' || src[i] == 0x09 || src[i] == ':') {
			i++;
			break;
		}
		if (src[i] == '\n' || src[i] == NULL) {
			return FALSE;
		}
		i++;
	}

	// 終端までをコピー
	while (1) {
		if (src[i] == '\n' || src[i] == NULL)
			break;
		dst[j] = src[i];
		i++;
		j++;
	}
	dst[j] = NULL;
	return TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////
// データを最適化して返す
////////////////////////////////////////////////////////////////////////////////////////
BOOL CBmsPro::LineCompact(const char *src, char *dst) {
	int i, j, k;
	char buf[1024];		// ワークバッファ
	strcpy(buf, src);	// 元データをコピー
	*dst = NULL;		// 出力先初期化

	// 010000020000030000040000010000020000030000040000010000020000030000040000010000020000030000040000
	//   ^^  ^^  ^^  ^^  ^^  ^^  ^^  ^^  ^^  ^^  ^^  ^^  ^^  ^^  ^^  ^^  ^^  ^^  ^^  ^^  ^^  ^^  ^^  ^^  i=1  (i+1)=2で割り切れる文字数か→○
	//   ^^^^  ^^^^  ^^^^  ^^^^  ^^^^  ^^^^  ^^^^  ^^^^  ^^^^  ^^^^  ^^^^  ^^^^  ^^^^  ^^^^  ^^^^  ^^^^  i=2  (i+1)=3で割り切れる文字数か→○
	//   ^^^^^^  ^^^^^^  ^^^^^^  ^^^^^^  ^^^^^^  ^^^^^^  ^^^^^^  ^^^^^^  ^^^^^^  ^^^^^^  ^^^^^^  ^^^^^^  i=3  (i+1)=4で割り切れる文字数か→○
	//   ^^^^^^^^  ^^^^^^^^  ^^^^^^^^  ^^^^^^^^  ^^^^^^^^  ^^^^^^^^  ^^^^^^^^  ^^^^^^^^  ^^^^^^^^  ^^^^  i=4  (i+1)=5で割り切れる文字数か→×
	//   ^^^^^^^^^^  ^^^^^^^^^^  ^^^^^^^^^^  ^^^^^^^^^^  ^^^^^^^^^^  ^^^^^^^^^^  ^^^^^^^^^^  ^^^^^^^^^^  i=5  (i+1)=6で割り切れる文字数か→○
	//   ^^^^^^^^^^^^  ^^^^^^^^^^^^  ^^^^^^^^^^^^  ^^^^^^^^^^^^  ^^^^^^^^^^^^  ^^^^^^^^^^^^  ^^^^^^^^^^  i=6  (i+1)=7で割り切れる文字数か→×
	//                                              ：
	//   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^      i=45 (i+1)=46で割り切れる文字数か→×
	//   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^    i=46 (i+1)=47で割り切れる文字数か→×
	//   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^  i=47 (i+1)=48で割り切れる文字数か→○
	while (1) {

		// データ数を算出
		int l = (int)strlen(buf) / 2;

		// 空文字ならデータ無し
		if (l == 0) {
			return FALSE;
		}

		// 1個のデータなら
		if (l == 1) {
			if (buf[0] == '0' && buf[1] == '0') {
				// 00ならデータ無し
				return FALSE;
			}
			// それ以外なら確定
			break;
		}


		// 2個以上なら最適化処理
		BOOL div = FALSE;					// 分割出来たか
		for (i = 1; i < l; i++) {

			// 分割する文字数
			int fetch_size = 1 + i;

			// 割り切れるか
			if ((l%fetch_size) != 0)
				continue;					// 割り切れないならスキップ

			// 分割文字数内の2番目以降に00以外が含まれるか
			BOOL zero = TRUE;		// 00以外が存在するか
			for (j = 0; j < l; j += fetch_size) {
				for (k = 1; k < fetch_size; k++) {
					int p = (j + k) * 2;
					if (buf[p] != '0' || buf[p + 1] != '0') {
						// 00以外が存在するなら
						zero = FALSE;
						j = l;				// 上位ループも終わらせる
						break;
					}
				}
			}

			// 00のみなら現座時の分割サイズで分割
			if (zero) {
				k = 1;
				for (j = 0 + fetch_size; j < l; j += fetch_size) {			// 1文字目は常に確定なので2つ目からコピー
					int src = j * 2;
					int dst = k * 2;
					buf[k * 2 + 0] = buf[j * 2 + 0];
					buf[k * 2 + 1] = buf[j * 2 + 1];
					// 次のコピー先へ
					k++;
				}
				buf[k * 2] = NULL;
				l = k;
				div = TRUE;

				//				DEBUG( "分割後 : %s\n",buf );
				break;
			}
		}

		// これ以上分割出来なかったら終わる
		if (!div)
			break;
	}

	// 結果を返す
	strcpy(dst, buf);

	return TRUE;
}
