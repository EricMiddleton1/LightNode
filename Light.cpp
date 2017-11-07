#include "Light.hpp"

#include <stdexcept>
#include <string>

using namespace std;
using namespace chrono_literals;

Light::Light(boost::asio::io_service& ioService, const string& _name, int _ledCount)
	:	timer{ioService, 10ms, [this]{for(auto& led : leds) {
			led.updateColorFilter();
		}}}
	,	name{_name}
	,	leds(_ledCount)	{
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
