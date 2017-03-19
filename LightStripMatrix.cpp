#include "LightStripMatrix.hpp"


LightStripMatrix::LightStripMatrix()
	:	LightStrip(LightStrip::Type::Matrix, 0)
	,	width{0}
	,	height{0} {
}

LightStripMatrix::LightStripMatrix(uint8_t _width, uint8_t _height)
	:	LightStrip(LightStrip::Type::Matrix, (size_t)_width*_height)
	,	width{_width}
	,	height{_height} {
}

uint8_t LightStripMatrix::getWidth() const {
	return width;
}

uint8_t LightStripMatrix::getHeight() const {
	return height;
}
