#pragma once

#include <cstdint>

#include "Light.hpp"

class Matrix : public Light {
public:
	Matrix(boost::asio::io_service& ioService, const std::string& name, uint8_t _width,
		uint8_t _height);
	~Matrix();

	uint8_t getWidth() const;
	uint8_t getHeight() const;

	static bool isMatrix(const Light&);
private:
	uint8_t width, height;
};
