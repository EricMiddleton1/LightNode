#include "Light.hpp"

#include <stdexcept>
#include <string>
#include <iostream>

using namespace std;
using namespace std::literals::chrono_literals;

Light::Light(boost::asio::io_service& _ioService, const string& _name, int _ledCount)
	: transitionTimer{_ioService, 1ms, [this]() {
			float t;

			if(transitionPeriod == 0) {
				t = 1.f;
			}
			else {
				tick = std::min(transitionPeriod, tick + 1);
				t = static_cast<float>(tick) / transitionPeriod;
			}

			for(auto& led : leds) {
				led.update(t);
			}
			
			update();

			if(tick == transitionPeriod) {
				transitionTimer.stop();
			}
		}}
	,	transitionPeriod{0}
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

void Light::startTransition(unsigned int _transitionPeriod) {
	for(auto& led : leds) {
		led.startTransition();
	}

	transitionPeriod = _transitionPeriod;
	tick = 0;

	transitionTimer.start();
}
