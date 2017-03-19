#pragma once

#include <vector>
#include <cstdint>
#include <mutex>

#include "Color.hpp"

class LightStrip
{
public:
	enum class Type {
		Analog = 0,
		Digital,
		Matrix
	};
	
	LightStrip();
	LightStrip(Type type, size_t ledCount);

	Type getType() const;
	size_t getSize() const;
	
	void setPixels(const std::vector<Color>& colors);

protected:
	virtual void update() = 0;

	Type type;
	std::vector<Color> leds;
	mutable std::mutex ledMutex;
};
