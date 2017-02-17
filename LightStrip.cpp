#include "LightStrip.hpp"

#include <stdexcept>
#include <string>

using namespace std;

LightStrip::LightStrip(size_t _ledCount)
	:	leds(_ledCount) {
}

size_t LightStrip::getSize() const {
	unique_lock<mutex> ledLock(ledMutex);

	return leds.size();
}

void LightStrip::setColors(const vector<Color>& _colors) {
	unique_lock<mutex> ledLock(ledMutex);

	if(_colors.size() != leds.size()) {
		throw runtime_error("[LightStrip::setColors]: Invalid color vector size "
			"(expected " + to_string(leds.size()) + ", actually "
			+ to_string(_colors.size()) + ")");
	}

	leds = _colors;
}
