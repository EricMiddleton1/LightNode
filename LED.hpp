#pragma once

#include <mutex>
#include <cstdint>
#include <boost/asio.hpp>

#include "PeriodicTimer.hpp"
#include "Color.hpp"

class Light;

class LED {
public:
	LED();

	bool isTargetOn() const;
	Color getColor() const;

	void turnOn();
	void turnOff();
	
	uint8_t getTargetHue() const;
	uint8_t getTargetSat() const;
	uint8_t getTargetVal() const;

	void setTargetHue(uint8_t hue);
	void setTargetSat(uint8_t sat);
	void setTargetVal(uint8_t val);

	void startTransition();

	void update(float hTime, float sTime, float vTime);

private:
	struct State {
		uint8_t h, s, v;
		bool on;
	};

	static uint8_t filter(uint8_t start, uint8_t end, float t, bool wrap = false);
	
	State start, cur, target;
};
