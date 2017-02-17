#pragma once

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <thread>
#include <chrono>
#include <functional>
#include <iostream>
#include "Color.hpp"

class Communicator
{
	public:
		enum class NodeType {
			ANALOG = 0,
			DIGITAL = 1,
			MATRIX = 2
		};

		Communicator(NodeType nodeType, uint16_t pixelCount,
			std::function<void(std::vector<Color>&)> cbUpdate);
		~Communicator();

		bool isConnected();

	private:
		static const uint16_t PORT_SEND = 54924;
		static const uint16_t PORT_RECV = 54923;
		static const int BUFFER_SIZE = 65535;
		static const int HEADER = 0xAA55;
		static const int ALIVE_TIMEOUT = 1000;
		static const int WATCHDOG_TIMEOUT = 3000;

		enum class PacketID {
			PING = 0x00,
			INIT = 0x01,
			INFO = 0x02,
			UPDATE = 0x03,
			ALIVE = 0x04,
			ACK = 0xFE,
			NACK = 0xFF};

		void startListening();

		void handleReceive(const boost::system::error_code& error,
			size_t bytesTransferred);

		void cbAliveTimer(const boost::system::error_code& ec);
		void cbWatchdogTimer(const boost::system::error_code& ec);

		void startAliveTimer();
		void feedWatchdog();

		void threadRoutine();

		void sendAck(const boost::asio::ip::udp::endpoint&);
		void sendNack(const boost::asio::ip::udp::endpoint&, PacketID);
		void sendInfo(const boost::asio::ip::udp::endpoint&);

		bool processUpdate(int bytesTransferred);

		//Network stuff
		boost::asio::io_service ioService;
		std::unique_ptr<boost::asio::io_service::work> ioWork;
		boost::asio::ip::udp::socket udpSocket;
		boost::asio::ip::udp::endpoint clientEndpoint, recvEndpoint;
		uint16_t sendPort, recvPort;

		//Timer stuff
		boost::asio::deadline_timer aliveTimer, watchdogTimer;

		std::thread asyncThread;

		bool connected;
		uint16_t pixelCount;
		NodeType nodeType;
		boost::array<unsigned char, BUFFER_SIZE> readBuf;
		std::function<void(std::vector<Color>&)> cbUpdate;
};
