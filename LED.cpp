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
	return Color::HSV(h, s, v);
}

void LED::turnOn() {
	on = true;
}

void LED::turnOff() {
	on = false;
}

uint8_t LED::getHue() const {
	return h;
}

uint8_t LED::getSat() const {
	return s;
}

uint8_t LED::getVal() const {
	return v;
}

void LED::setHue(uint8_t hue) {
	h = hue;
}

void LED::setSat(uint8_t sat) {
	s = sat;
}

void LED::setVal(uint8_t val) {
	v = val;
}
