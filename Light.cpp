#include "Light.hpp"

#include <stdexcept>
#include <string>
#include <iostream>
#include <numeric>

using namespace std;
using namespace std::literals::chrono_literals;

Light::Light(boost::asio::io_service& _ioService, const string& _name, int _ledCount)
	: transitionTimer{_ioService, 1ms, [this]() {
			float hTime, sTime, vTime;

			hTime = (huePeriod == 0) ? 1.f : min(1.f, static_cast<float>(tick) / huePeriod);
			sTime = (satPeriod == 0) ? 1.f : min(1.f, static_cast<float>(tick) / satPeriod);
			vTime = (valPeriod == 0) ? 1.f : min(1.f, static_cast<float>(tick) / valPeriod);

			tick = std::min(std::max({huePeriod, satPeriod, valPeriod}), tick + 1);

			for(auto& led : leds) {
				led.update(hTime, sTime, vTime);
			}
			
			update();

			if(std::min({hTime, sTime, vTime}) >= 1.f) {
				transitionTimer.stop();
			}
		}}
	,	huePeriod{0}
	,	satPeriod{0}
	,	valPeriod{0}
	,	tick{0}
	,	name{_name}
	,	leds(_ledCount) {
	
	transitionTimer.start();
}

string Light::getName() const {
	return name;
}

int Light::size() const {
	return leds.size();
}

vector<LED>::iterator Light::begin() {
	return leds.begin();
}

vector<LED>::const_iterator Light::begin() const {
	return leds.begin();
}

vector<LED>::iterator Light::end() {
	return leds.end();
}

vector<LED>::const_iterator Light::end() const {
	return leds.end();
}

void Light::setGammaCorrect(bool _gammaCorrect) {
	gammaCorrect = _gammaCorrect;
}

void Light::setHuePeriod(unsigned int period) {
	huePeriod = period;
}

void Light::setSatPeriod(unsigned int period) {
	satPeriod = period;
}

void Light::setValPeriod(unsigned int period) {
	valPeriod = period;
}

void Light::startTransition() {
	for(auto& led : leds) {
		led.startTransition();
	}

	tick = 0;

	transitionTimer.start();
}
