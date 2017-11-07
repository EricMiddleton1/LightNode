#pragma once

#include <vector>

#include "LED.hpp"

class Light
{
public:
	static const int MAX_LED = 255;

	Light(const std::string& name, int ledCount);

	std::string getName() const;
	int size() const;
	
	std::vector<LED>::iterator begin();
	std::vector<LED>::const_iterator begin() const;

	std::vector<LED>::iterator end();
	std::vector<LED>::const_iterator end() const;

	virtual void update() = 0;
protected:
	std::string name;
	std::vector<LED> leds;
};
