#include "LED.hpp"

using namespace std;
using namespace chrono_literals;

LED::LED()
	:	h{0}
	,	s{0}
	,	v{255}
	,	on{false} {

}

bool LED::isOn() const {
	return on;
}

Color LED::getColor() const {
	lock_guard<std::mutex> colorLock(colorMutex);

	return on ? Color::HSV(h, s, v) : Color{};
}

void LED::turnOn() {
	on = true;
}

void LED::turnOff() {
	on = false;
}

uint8_t LED::getHue() const {
	lock_guard<std::mutex> colorLock(colorMutex);

	return h;
}

uint8_t LED::getSat() const {
	lock_guard<std::mutex> colorLock(colorMutex);

	return s;
}

uint8_t LED::getVal() const {
	lock_guard<std::mutex> colorLock(colorMutex);

	return v;
}

void LED::setHue(uint8_t hue) {
	lock_guard<std::mutex> colorLock(colorMutex);

	targetH = hue;
}

void LED::setSat(uint8_t sat) {
	lock_guard<std::mutex> colorLock(colorMutex);

	targetS = sat;
}

void LED::setVal(uint8_t val) {
	lock_guard<std::mutex> colorLock(colorMutex);

	targetV = val;
}

void LED::updateColorFilter() {
	lock_guard<std::mutex> colorLock(colorMutex);

	filter(h, targetH, 5);
	filter(s, targetS, 5);
	filter(v, targetV, 5);
}

void LED::filter(uint8_t& out, uint8_t target, uint8_t inc) {
	if(out != target) {
		int dir = ((int)target - (int)out) > 0 ? 1 : -1;
		int diff = ((int)target - (int)out) * dir;

		out += dir*std::min(diff, (int)inc);
	}
}
