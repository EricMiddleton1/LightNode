#pragma once

#include <vector>

#include "LED.hpp"
#include "PeriodicTimer.hpp"

class Light
{
public:
	static const int MAX_LED = 255;

	Light(boost::asio::io_service& ioService, const std::string& name, int ledCount);
	virtual ~Light();

	std::string getName() const;
	int size() const;

	virtual LED& operator[](int index);
	virtual const LED& operator[](int index) const;

	void setGammaCorrect(bool);

	void setHuePeriod(unsigned int period);
	void setSatPeriod(unsigned int period);
	void setValPeriod(unsigned int period);

	void startTransition();

	virtual void update() = 0;
protected:
	virtual int ledMap(int index) const;

	PeriodicTimer transitionTimer;
	unsigned int huePeriod, satPeriod, valPeriod, tick;
	
	std::string name;
	std::vector<LED> leds;
	bool gammaCorrect;
};
