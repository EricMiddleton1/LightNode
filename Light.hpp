#pragma once

#include <vector>

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
	
	void setGammaCorrect(bool);

	void startTransition(unsigned int transitionPeriod);

	virtual void update() = 0;
protected:
	PeriodicTimer transitionTimer;
	unsigned int transitionPeriod, tick;
	
	std::string name;
	std::vector<LED> leds;
	bool gammaCorrect;
};
