#include "EffectManager.hpp"


EffectManager::EffectManager() {
	dTimeLength = 0;
	uiIndex = 0;
}

EffectManager::~EffectManager() {
	effect.erase(effect.cbegin(), effect.cend());
}


void EffectManager::Reset() {
	effect.assign(effect.size(), { 0ul, 0.0f, 0.0f });
	dTimeLength = 0;
	uiIndex = 0;
}

void EffectManager::Create(wchar_t fileName, const draweffectoption_func& changeDrawingFunc, double timeLength, unsigned int element) {
	effect.resize(element);
	effect.assign(effect.size(), { 0ul, 0.0f, 0.0f });

	dTimeLength = timeLength;
	uiIndex = 0;
}

void EffectManager::Start(float x, float y) {
	Start(x, y, dTimeLength);
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


DRAWEFFECTPARAM EffectManager::drawEffectDefFunc(float timePos) {
	DRAWEFFECTPARAM param;
	param.alpha = timePos;
	param.offsetX = 0.0f;
	param.offsetY = 0.0;
	param.scale = 1.0f;

	return param;
}