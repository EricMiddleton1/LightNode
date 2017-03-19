#pragma once

#include "LightStrip.hpp"

class LightStripMatrix : public LightStrip
{
public:
	LightStripMatrix();
	LightStripMatrix(uint8_t width, uint8_t height);
	
	uint8_t getWidth() const;
	uint8_t getHeight() const;
protected:
	uint8_t width, height;
};
