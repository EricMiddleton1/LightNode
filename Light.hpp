#pragma once

#include <vector>

#include <boost/asio.hpp>

#include "LED.hpp"
#include "PeriodicTimer.hpp"

class Light
{
public:
	static const int MAX_LED = 255;

	Light(boost::asio::io_service& ioService, const std::string& name, int ledCount);

	std::string getName() const;
	int size() const;
	
	std::vector<LED>::iterator begin();
	std::vector<LED>::const_iterator begin() const;

	std::vector<LED>::iterator end();
	std::vector<LED>::const_iterator end() const;

	virtual void update() = 0;
protected:
	PeriodicTimer timer;

	std::string name;
	std::vector<LED> leds;
};
