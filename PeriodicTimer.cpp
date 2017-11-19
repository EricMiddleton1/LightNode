#include "PeriodicTimer.hpp"

#include <stdexcept>
#include <iostream>

PeriodicTimer::PeriodicTimer(boost::asio::io_service& _ioService,
	const std::chrono::microseconds& _period, const TimerHandler& _handler)
	:	running{false}
	,	ioService{_ioService}
	,	period{_period}
	,	handler{_handler}
	,	timer{ioService} {
	
	if(!handler) {
		throw std::runtime_error("PeriodicTimer: Invalid handler");
	}

}

PeriodicTimer::~PeriodicTimer() {
	stop();
}

void PeriodicTimer::start() {
	running = true;
	
	timer.expires_from_now(period);
	timer.async_wait([this](const boost::system::error_code& error) {
		cbTimer(error);
	});

	ioService.post(handler);
}

void PeriodicTimer::stop() {
	running = false;
	timer.cancel();
}

bool PeriodicTimer::isRunning() const {
	return running;
}

void PeriodicTimer::cbTimer(const boost::system::error_code& error) {
	if(error == boost::asio::error::operation_aborted) {
		return;
	}
	else if(error) {
		std::cout << "[Error] PeriodicTimer: cbTimer: " << error.message() << std::endl;
	}

	resetTimer();

	if(!error) {
		handler();
	}
}

void PeriodicTimer::resetTimer() {
	if(running) {
		timer.expires_from_now(timer.expires_from_now() + period);
		timer.async_wait([this](const boost::system::error_code& error) {
			cbTimer(error);
		});
	}
}
