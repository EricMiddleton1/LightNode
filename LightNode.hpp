#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <array>
#include <thread>
#include <chrono>
#include <functional>
#include <cstdint>
#include <iostream>
#include <deque>

#include "Packet.hpp"
#include "Light.hpp"

class LightNode
{
public:
	LightNode(const std::vector<std::shared_ptr<Light>>& lights,
		const std::string& name);
	~LightNode();

private:
	static const uint16_t PORT = 5492;
	static const int BUFFER_SIZE = 65535;
	static const uint8_t HEADER = 0xAA;

	void startListening();

	void handleReceive(const boost::system::error_code& error,
		size_t bytesTransferred);

	void sendDatagram(const boost::asio::ip::udp::endpoint& endpoint,
		const std::vector<uint8_t>& data);

	void threadRoutine();

	void updateColor(uint8_t, const std::vector<uint8_t>&);

	//Network stuff
	boost::asio::io_service ioService;
	std::unique_ptr<boost::asio::io_service::work> ioWork;
	boost::asio::ip::udp::endpoint recvEndpoint;
	boost::asio::ip::udp::socket udpSocket;
	
	std::thread asyncThread;
	
	std::string name;
	std::vector<std::shared_ptr<Light>> lights;

	std::array<uint8_t, BUFFER_SIZE> readBuf;

	std::deque<std::vector<uint8_t>> sendQueue;
};
