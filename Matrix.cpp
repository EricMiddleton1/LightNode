#include "Matrix.hpp"


Matrix::Matrix(boost::asio::io_service& ioService, const std::string& name,
	uint8_t _width, uint8_t _height)
	:	Light(ioService, name, _width*_height)
	,	width{_width}
	,	height{_height} {
}

Matrix::~Matrix() {
}

uint8_t Matrix::getWidth() const {
	return width;
}

uint8_t Matrix::getHeight() const {
	return height;
}

bool Matrix::isMatrix(const Light& light) {
	return dynamic_cast<const Matrix*>(&light) != nullptr;
}
