#include "LED.hpp"

#include <iostream>

LED::LED()
	:	start{0, 0, 127, false}
	,	cur{start}
	,	target{start} {

}

bool LED::isTargetOn() const {
	return target.on;
}

Color LED::getColor() const {
	return Color::HSV(cur.h, cur.s, cur.v);
}

void LED::turnOn() {
	target.on = true;
}

void LED::turnOff() {
	target.on = false;
}

uint8_t LED::getTargetHue() const {
	return target.h;
}

uint8_t LED::getTargetSat() const {
	return target.s;
}

uint8_t LED::getTargetVal() const {
	return target.v;
}

void LED::setTargetHue(uint8_t hue) {
	target.h = hue;
}

void LED::setTargetSat(uint8_t sat) {
	target.s = sat;
}

void LED::setTargetVal(uint8_t val) {
	if(val == 0) {
		target.on = false;
	}
	else {
		target.v = val;
		target.on = true;
	}
}

void LED::startTransition() {
	start = cur;
}

void LED::update(float hTime, float sTime, float vTime) {
	cur.h = filter(start.h, target.h, hTime, true);
	cur.s = filter(start.s, target.s, sTime);
	cur.v = filter(start.v, (target.on) ? target.v : 0, vTime);
}

uint8_t LED::filter(uint8_t start, uint8_t end, float t, bool wrap) {
	int slope = static_cast<int>(end) - start;

	if(wrap && std::abs(slope) > 127) {
		auto sign = slope > 0 ? 1 : -1;

		slope = (slope - sign*256);
	}

	int result = start + slope*t;
	return result;
}
