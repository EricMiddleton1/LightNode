#include "LightStrip.hpp"

#include <stdexcept>
#include <string>

using namespace std;

LightStrip::LightStrip(LightStrip::Type _type, size_t _ledCount)
	:	type(_type)
	,	leds(_ledCount) {
}

LightStrip::Type LightStrip::getType() const {
	return type;
}

size_t LightStrip::getSize() const {
	unique_lock<mutex> ledLock(ledMutex);

	return leds.size();
}

void LightStrip::setPixels(const vector<Color>& _colors) {
	unique_lock<mutex> ledLock(ledMutex);

	if(_colors.size() != leds.size()) {
		throw runtime_error("[LightStrip::setColors]: Invalid color vector size "
			"(expected " + to_string(leds.size()) + ", actually "
			+ to_string(_colors.size()) + ")");
	}

	leds = _colors;

	update();
}
