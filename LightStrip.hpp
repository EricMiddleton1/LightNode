#pragma once

#include <vector>
#include <cstdint>

#include "Color.hpp"

class LightStrip
{
public:
	LightStrip(size_t ledCount);

	size_t getSize() const;
	void setColors(const std::vector<Color>& colors);

	virtual void update() = 0;
protected:
	std::vector<Color> leds;
};
