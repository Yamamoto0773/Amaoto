#pragma once


#include "GameController.hpp"
#include "HighPrecisionTimer.hpp"
#include "libfiles/CDSPro81.h"
#include "libfiles/DirectXText.h"
#include "libfiles/CDDPro90.h"
#include "libfiles/CDSPro81.h"
#include "libfiles/CBmsPro.h"
#include "GameController.hpp"


class GameScene {
private:
	static int iCursor1;
	static int iCursor2;

	HighPrecisionTimer cSceneTime;
	static CDDPro90 *dd;
	static CDSPro81 *ds;
	static GameController *cController;
	

	static CBmsPro *bms;

	enum class SCENECHANGESTATE {
		CONTINUATION = 0,
		NEXT,
	}eChangeState;

	
	

private:
	
public:
	GameScene();
	virtual ~GameScene();

	virtual bool Init() = 0;
	virtual int Update() = 0;
	virtual void Draw() const = 0;
	virtual bool Exit() = 0;

};
