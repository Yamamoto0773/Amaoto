#pragma once

#pragma warning(disable : 4996)

/////////////////////////////////////////////////////
// Windows API���g�p����MIDI�L�[�{�[�h�̏�Ԃ��擾����N���X�ł�
// �Q�l�@http://puarts.com/?pid=1124

// �쐬�ҁ@�R�{���C
// �쐬���@2017/10/11
/////////////////////////////////////////////////////


#include <stdio.h>
#include <Windows.h>
#include <mmsystem.h>	

#pragma comment(lib, "winmm.lib")


#define MSGBUFFERSIZE	(1024)
#define MIDIKEYCNT			(25)

typedef enum _MIDIKEYSTATE {
	MIDIKEYSTATE_NONE		= 0,
	MIDIKEYSTATE_PRESSED	= 1,
	MIDIKEYSTATE_HOLD		= 2,
	MIDIKEYSTATE_RELEASED	= 3
}MIDIKEYSTATE;


class MIDIInput {
private:
	HMIDIIN				hMidiInHandle;
	static CHAR			mMsgBuffer[MSGBUFFERSIZE];
	static unsigned int	iWriteMsgByte;				// mMsgBuffer�ɏ������񂾃o�C�g��
	static MIDIKEYSTATE	eKeyState[MIDIKEYCNT];



private:
	static void CALLBACK MidiInProc(
		HMIDIIN midi_in_handle,
		UINT	wMsg,
		DWORD	dwInstance,
		DWORD	dwParam1,
		DWORD	dwParam2
		);

	
	static int WriteMsgBuffer(					
		unsigned int	&totalbufCnt,		//[in][out]
		CHAR			*buffer,			//[in][out]
		CONST CHAR		*format, ...		//[in]
		);

	
public:
	MIDIInput();
	~MIDIInput();

	BOOL Create( void );
	BOOL StartInput( void );
	BOOL StopInput( void );
	BOOL Clear( void );

	VOID GetMidiMessage( CHAR *buf );
	BOOL IsMessage(void);

	BOOL GetKeyState( MIDIKEYSTATE *state);

};




