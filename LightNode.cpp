#include "LightNode.hpp"

#include <stdexcept>
#include <string>

std::string toString(const std::vector<uint8_t>& data) {
	std::string str("{");

	if(!data.empty()) {
		for(int i = 0; i < (data.size() - 1); ++i) {
			str += std::to_string(static_cast<int>(data[i])) + ", ";
		}
		str += std::to_string(static_cast<int>(data[data.size()-1]));
	}
	str += "}";

	return str;
}

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
		try {
			Packet p({readBuf.begin(), readBuf.begin() + bytesTransferred});
			uint8_t lightID = p.getLightID();

			if(lightID >= lights.size()) {
				std::cerr << "[Error] LightNode::handleReceive: Invalid Light ID: "
					<< static_cast<int>(lightID) << std::endl;
			}
			else {
				auto& light = *lights[lightID];
				auto data = p.data();
				bool update = false;

				switch(p.getID()) {
					case Packet::ID::NodeInfo:
						sendDatagram(recvEndpoint,
							Packet::NodeInfoResponse(lights.size(), name).asDatagram());
					break;

					case Packet::ID::LightInfo:
						sendDatagram(recvEndpoint,
							Packet::LightInfoResponse(lightID, light.size(),
								light.getName()).asDatagram());
					break;

					case Packet::ID::TurnOn:
						if(data.size() == 1) {
							for(auto& led : light)
								led.turnOn();
							update = true;
							light.setValPeriod(10*data[0]);
						}
						else {
							std::cerr << "[Error] Invalid packet length for TurnOn" << std::endl;
						}
					break;
					
					case Packet::ID::TurnOff:
						if(data.size() == 1) {
							for(auto& led : light)
								led.turnOff();
							update = true;
							light.setValPeriod(10*data[0]);
						}
						else {
							std::cerr << "[Error] Invalid packet length for TurnOff" << std::endl;
						}
					break;

					case Packet::ID::SetBrightness:
					break;

					case Packet::ID::SetColor:
					break;

					case Packet::ID::UpdateColor:
						try {
							updateColor(lightID, data);
							update = true;
						}
						catch(const std::exception& e) {
							std::cerr << "[Error] LightNode::handleReceive: " << e.what() << std::endl;
						}
					break;

					case Packet::ID::ChangeBrightness: {
						if(data.size() == 2) {
							int brightness = static_cast<int>(light.begin()[0].getTargetVal()); 
							int delta = 255*static_cast<int>(static_cast<int8_t>(p.data()[1])) / 100;
							brightness = std::min(255, std::max(0, brightness + delta));

							for(auto& led : light) {
								led.setTargetVal(brightness);
							}

							update = true;
							light.setValPeriod(10*data[0]);
						}
						else {
							std::cerr << "[Error] Invalid packet length for ChangeBrightness"
								<< std::endl;
						}
					}
					break;


					break;
				}

				if(update) {
					if(data.size() < 1) {
						std::cerr << "[Error] LightNode::handleReceive: Invalid data size "
							"for light update" << std::endl;
					}
					else {
						light.startTransition();
					}
				}
			}
		}
		catch(const std::exception& e) {
			std::cerr << "[Error] LightNode::handleReceive: " << e.what() << std::endl;
		}
	}
	startListening();
}

void LightNode::sendDatagram(const boost::asio::ip::udp::endpoint& endpoint,
	const std::vector<uint8_t>& data) {
	
	sendQueue.push_back(data);

	udpSocket.async_send_to(boost::asio::buffer(sendQueue.back()), endpoint,
		[this](const boost::system::error_code& ec, size_t bytesTransferred) {
			sendQueue.pop_front();

			if(ec) {
				std::cerr << "[Error] LightNode::cbSendDatagram: " << ec.message() << std::endl;
			}
		});
}

void LightNode::threadRoutine() {
		ioService.run();
}

void LightNode::updateColor(uint8_t lightID, const std::vector<uint8_t>& data) {
	auto& light = *lights[lightID];

	if(data.size() != (3 + 3*light.size())) {
		throw std::runtime_error(std::string("LightNode::updateColor: invalid size: ")
			+ std::to_string(data.size()));
	}
	

	light.setHuePeriod(10*data[0]);
	light.setSatPeriod(10*data[1]);
	light.setValPeriod(10*data[2]);

	for(int i = 0; i < light.size(); ++i) {
		int index = 3 + 3*i;

		auto led = light.begin() + i;

		led->setTargetHue(data[index]);
		led->setTargetSat(data[index+1]);
		led->setTargetVal(data[index+2]);
	}
}
