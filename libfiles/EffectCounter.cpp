#include "EffectCounter.hpp"


EffectCounter::EffectCounter() {
	ulMaxCount = 0;
	uiIndex = 0;
}

EffectCounter::~EffectCounter() {
	counter.assign(counter.size(), { 0ul, 0.0f, 0.0f });
	ulMaxCount = 0;
	uiIndex = 0;
}

void EffectCounter::Update() {
	int cnt = timer.Run();

	for (int i=0; i<counter.size(); i++) {
		counter[i].count -= cnt;
		if (counter[i].count < 0) counter[i].count = 0;
	}
}

void EffectCounter::Create(unsigned int element, unsigned long maxCount, int updateFps) {
	counter.resize(element);
	counter.assign(counter.size(), { 0ul, 0.0f, 0.0f });

	ulMaxCount = maxCount;
	uiIndex = 0;
	
	timer.Start(updateFps);
}

void EffectCounter::Reset() {
	counter.assign(counter.size(), { 0ul, 0.0f, 0.0f });
	ulMaxCount = 0;
	uiIndex = 0;
}

void EffectCounter::CountStart(float x, float y) {
	counter[uiIndex].count = ulMaxCount;
	counter[uiIndex].x = x;
	counter[uiIndex].y = y;

	uiIndex++;
	if (uiIndex >= counter.size()) uiIndex = 0;

}

void EffectCounter::CountStart(float x, float y, int val) {
	counter[uiIndex].count = val;
	counter[uiIndex].x = x;
	counter[uiIndex].y = y;

	uiIndex++;
	if (uiIndex >= counter.size()) uiIndex = 0;

}

const std::vector<EFFECTCOUNT>::const_iterator EffectCounter::GetCount() {
	Update();

	return counter.cbegin();
}

void EffectCounter::Delete() {
	counter.erase(counter.cbegin(), counter.cend());
}
