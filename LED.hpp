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

	bool isOn() const;
	Color getColor() const;

	void turnOn();
	void turnOff();
	
	uint8_t getHue() const;
	uint8_t getSat() const;
	uint8_t getVal() const;

	void setHue(uint8_t hue);
	void setSat(uint8_t sat);
	void setVal(uint8_t val);

private:
	friend class Light;

	void updateColorFilter();
	static void filter(uint8_t& out, uint8_t target, uint8_t inc);

	mutable std::mutex colorMutex;
	uint8_t h, s, v, targetH, targetS, targetV;
	bool on;
};
