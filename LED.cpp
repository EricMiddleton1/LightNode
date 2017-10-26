#include "LED.hpp"


LED::LED()
	:	h{0}
	,	s{0}
	,	v{255}
	,	on{false} {

}

bool LED::isOn() const {
	return on;
}

Color LED::getRGB() const {
	return Color::HSV(h, s, v);
}

void LED::turnOn() {
	on = true;
}

void LED::turnOff() {
	on = false;
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
