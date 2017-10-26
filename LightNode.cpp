#include "LightNode.hpp"

#include <stdexcept>
#include <string>

LightNode::LightNode(const std::vector<std::shared_ptr<Light>>& _lights,
	const std::string& _name)
	:	ioWork{std::make_unique<boost::asio::io_service::work>(ioService)}
	,	recvEndpoint{boost::asio::ip::udp::v4(), PORT}
	,	udpSocket{ioService, recvEndpoint}
	, asyncThread{[this]() {threadRoutine();}}
	,	name{_name}
	,	lights{_lights} {
	
	if(_lights.size() == 0) {
		throw std::runtime_error("LightNode::LightNode: Requires at least 1 Light "
			"(you gave me 0)");
	}
	
	startListening();
}

LightNode::~LightNode() {
	ioWork.reset();
	ioService.stop();

	asyncThread.join();
}

void LightNode::startListening() {
	udpSocket.async_receive_from(boost::asio::buffer(readBuf),
		recvEndpoint,
		[this](const boost::system::error_code& ec, size_t bytesTransferred) {
			handleReceive(ec, bytesTransferred);
		});
}

void LightNode::handleReceive(const boost::system::error_code& error,
	size_t bytesTransferred) {

	if(error) {
		std::cout << "[Error] handleReceive: " << error.message() << std::endl;
	}
	else if(bytesTransferred > 1) {
		//If the packet header is correct
		if(readBuf[0] == HEADER) {
			try {
				Packet p({readBuf.begin()+1, readBuf.end()});
				uint8_t lightID = p.getLightID();

				if(lightID >= lights.size()) {
					std::cerr << "[Error] LightNode::handleReceive: Invalid Light ID: "
						<< lightID << std::endl;
				}

				auto& light = *lights[lightID];

				bool update = false;

				if(lightID < lights.size()) {
					switch(p.getID()) {
						case Packet::ID::NodeInfo:
						break;

						case Packet::ID::LightInfo:
						break;

						case Packet::ID::TurnOn:
							for(auto& led : light)
								led.turnOn();
							update = true;
						break;
						
						case Packet::ID::TurnOff:
							for(auto& led : light)
								led.turnOff();
							update = true;
						break;

						case Packet::ID::UpdateColor:
							try {
								updateColor(lightID, p.data());
								update = true;
							}
							catch(const std::exception& e) {
								std::cerr << "[Error] LightNode::handleReceive: " << e.what() << std::endl;
							}
						break;

						case Packet::ID::ChangeBrightness:
						break;
					}

					if(update) {
						light.update();
					}
				}
				else {
					std::cerr << "[Error] LightNode::handleReceive: Invalid Light ID: " << (int)lightID
						<< std::endl;
				}
			}
			catch(const std::exception& e) {
				std::cerr << "[Error] LightNode::handleReceive: " << e.what() << std::endl;
			}
		}
		else { //The packet header was not correct
			std::cerr << "[Error] Packet received with incorrect header: "
				<< (int)readBuf[0] << std::endl;
		}
	}

	startListening();
}

void LightNode::threadRoutine() {
		ioService.run();
}

void LightNode::updateColor(uint8_t lightID, const std::vector<uint8_t>& data) {
	if(data.size() < 2) {
		//throw std::runtime_error(std::string("LightNode::updateColor: invalid size: ")
			//+ std::to_string(data.size());
	}
	
	int colorMask = *(data.begin());
	if(colorMask == 0) {
		throw std::runtime_error("LightNode::updateColor: NULL color mask");
	}

	bool useHue = colorMask & 0x4,
		useSat = colorMask & 0x2,
		useVal = colorMask & 0x1;

	int stride = useHue + useSat + useVal;
	if( ((data.size()-1) % stride) != 0 ) {
		throw std::runtime_error(std::string("LightNode::updateColor: invalid size: ")
			+ std::to_string(data.size()));
	}

	auto& light = *lights[lightID];

	if(data.size() == (1 + stride)) {
		int i = 1;

		if(useHue) {
			for(auto& led : light)
				led.setHue(data[i]);
			++i;
		}
		if(useSat) {
			for(auto& led : light)
				led.setSat(data[i]);
			++i;
		}
		if(useVal) {
			for(auto& led : light)
				led.setVal(data[i]);
		}
	}
	else {
		if(data.size() > (light.size() + 1)) {
			throw std::runtime_error(std::string("LightNode::updateColor: invalid size: ")
				+ std::to_string(data.size()));
		}
		
		auto ledItr = light.begin();

		for(auto dataItr = data.begin()+1; dataItr < data.end(); ++ledItr) {
			if(useHue)
				ledItr->setHue(*(dataItr++));
			if(useSat)
				ledItr->setSat(*(dataItr++));
			if(useVal)
				ledItr->setVal(*(dataItr++));
		}
	}
}
