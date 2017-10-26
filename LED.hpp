#pragma once

#include <cstdint>

#include "Color.hpp"

class LED {
public:
	LED();

	bool isOn() const;
	Color getRGB() const;

	void turnOn();
	void turnOff();

	void setHue(uint8_t hue);
	void setSat(uint8_t sat);
	void setVal(uint8_t val);

private:
	uint8_t h, s, v;
	bool on;
};
