#include "LightNode.hpp"
#include "LightStripMatrix.hpp"


LightNode::LightNode(const std::vector<std::shared_ptr<LightStrip>>& _strips,
	const std::string& _name)
	:	ioWork(std::make_unique<boost::asio::io_service::work>(ioService))
	,	udpSocket(ioService)
	,	clientEndpoint(boost::asio::ip::udp::v4(), PORT_SEND)
	,	recvEndpoint(boost::asio::ip::udp::v4(), PORT_RECV)
	,	aliveTimer(ioService)
	,	watchdogTimer(ioService)
	, asyncThread(std::bind(&LightNode::threadRoutine, this))
	,	name(_name)
	,	connected(false) {
	
	if(_strips.size() == 0) {
		throw std::runtime_error("LightNode::LightNode: Requires at least 1 LightStrip "
			"(you gave me 0)");
	}
	
	for(auto& strip : _strips) {
		switch(strip->getType()) {
			case LightStrip::Type::Analog:
				analogStrips.push_back(strip);
			break;

			case LightStrip::Type::Digital:
				digitalStrips.push_back(strip);
			break;

			case LightStrip::Type::Matrix:
				matrixStrips.push_back(strip);
			break;

			default:
				throw std::runtime_error("LightNode::LightNode: Invalid LightStrip type");
			break;
		}
	}

	udpSocket.open(boost::asio::ip::udp::v4());

	//Bind the socket
	udpSocket.bind(recvEndpoint);

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
		std::bind(&LightNode::handleReceive, this,
			std::placeholders::_1, std::placeholders::_2));
}

void LightNode::handleReceive(const boost::system::error_code& error,
	size_t bytesTransferred) {

	if(error) {
		std::cout << "[Error] handleReceive: " << error.message() << std::endl;
	}
	else if(bytesTransferred > 2) {
		//Check for the correct header
		int readHeader = readBuf[0] << 8 | readBuf[1];
		PacketID readId = static_cast<PacketID>(readBuf[2]);

		//If the packet header is correct
		if(readHeader == HEADER) {
			
			//Respond to the same address, but different port
			auto respondEndpoint = recvEndpoint;
			respondEndpoint.port(PORT_SEND);

			bool inBand = connected && (respondEndpoint == clientEndpoint);

			if(connected && inBand && (readId != PacketID::PING)) {
				feedWatchdog();
			}

			switch(readId) {
				case PacketID::PING:
					if(!inBand) {
						//We respond with info packet
						sendInfo(respondEndpoint);
					}
				break;

				case PacketID::INIT:
					//We respond with ack packet
					if(!connected) {
						sendAck(respondEndpoint);

						clientEndpoint = respondEndpoint;
						connected = true;
						startAliveTimer();
						feedWatchdog();

						std::cout << "[Info] Client connected" << std::endl;
					}
					else {
						sendNack(respondEndpoint, readId);
					}
				break;

				case PacketID::UPDATE:
					if(inBand) {
						if(!processUpdate(bytesTransferred)) {
							sendNack(respondEndpoint, readId);
						}
					}
					else {
						sendNack(respondEndpoint, readId);
					}
				break;

				default:
					std::cout << "[Error] Unimplemented packet id received: "
						<< static_cast<int>(readId) << std::endl;
				break;
			}
		}
		else { //The packet header was not correct
			std::cout << "[Error] Packet received with incorrect header: "
				<< (int)readHeader << std::endl;
		}
	}

	startListening();
}

void LightNode::threadRoutine() {
		ioService.run();
}

void LightNode::cbAliveTimer(const boost::system::error_code& ec) {
	if(ec) {
		if(ec.value() == boost::system::errc::operation_canceled) {
			return;
		}
		else {
			std::cout << "[Error] cbAliveTimer: " << ec.message() << std::endl;
		}
	}
	else if(!connected) {
		return;
	}
	else {
		std::vector<unsigned char> message;

		//Push the header
		message.push_back( (HEADER >> 8) & 0xFF );
		message.push_back( HEADER & 0xFF );

		//Push the identification
		message.push_back(static_cast<unsigned char>(PacketID::ALIVE));

		try {
			udpSocket.send_to(boost::asio::buffer(message), clientEndpoint);
		}
		catch(std::exception& e) {
			std::cerr << "sendAck exception caught: " << e.what() << std::endl;
		}
	}

	startAliveTimer();
}

void LightNode::cbWatchdogTimer(const boost::system::error_code& ec) {
	if(ec.value() != boost::system::errc::operation_canceled) {
		aliveTimer.cancel();
		connected = false;

		std::cout << "[Info] Client timed out" << std::endl;
	}
}

void LightNode::startAliveTimer() {
	aliveTimer.expires_from_now(boost::posix_time::milliseconds(ALIVE_TIMEOUT));
	aliveTimer.async_wait(std::bind(&LightNode::cbAliveTimer, this,
		std::placeholders::_1));
}

void LightNode::feedWatchdog() {
	watchdogTimer.cancel();
	
	watchdogTimer.expires_from_now(
		boost::posix_time::milliseconds(WATCHDOG_TIMEOUT));
	watchdogTimer.async_wait(std::bind(&LightNode::cbWatchdogTimer, this,
		std::placeholders::_1));
}


void LightNode::sendAck(const boost::asio::ip::udp::endpoint& endpoint) {
	std::vector<unsigned char> message;

	//Push the header
	message.push_back( (HEADER >> 8) & 0xFF );
	message.push_back( HEADER & 0xFF );

	//Push the identification
	message.push_back(static_cast<unsigned char>(PacketID::ACK));

	try {
		udpSocket.send_to(boost::asio::buffer(message), endpoint);
	}
	catch(std::exception& e) {
		std::cerr << "sendAck exception caught: " << e.what() << std::endl;
	}
}

void LightNode::sendNack(const boost::asio::ip::udp::endpoint& endpoint,
	LightNode::PacketID id) {
	std::vector<unsigned char> message;

	//Push the header
	message.push_back( (HEADER >> 8) & 0xFF );
	message.push_back( HEADER & 0xFF );

	//Push the identification
	message.push_back(static_cast<unsigned char>(PacketID::NACK));

	//Push the ID we're NACKing
	message.push_back(static_cast<unsigned char>(id));

	try {
		udpSocket.send_to(boost::asio::buffer(message), endpoint);
	}
	catch(std::exception& e) {
		std::cerr << "sendAck exception caught: " << e.what() << std::endl;
	}
}

void LightNode::sendInfo(const boost::asio::ip::udp::endpoint& endpoint) {
	std::vector<unsigned char> message;

	//Push the header
	message.push_back( (HEADER >> 8) & 0xFF );
	message.push_back( HEADER & 0xFF );

	//Push the identification
	message.push_back(static_cast<unsigned char>(PacketID::INFO));

	message.push_back(analogStrips.size());
	message.push_back(digitalStrips.size());
	message.push_back(matrixStrips.size());

	for(const auto& strip : digitalStrips) {
		message.push_back((strip->getSize() >> 8) & 0xFF);
		message.push_back(strip->getSize() & 0xFF);
	}

	for(const auto& strip : matrixStrips) {
		auto matrix = std::dynamic_pointer_cast<LightStripMatrix>(strip);

		message.push_back(matrix->getWidth());
		message.push_back(matrix->getHeight());
	}

	message.insert(message.end(), name.begin(), name.end());


	try {
		std::cout << "[Info] LightNode::sendInfo: Sending info to "
			<< endpoint << std::endl;

		udpSocket.send_to(boost::asio::buffer(message), endpoint);
	}
	catch(std::exception& e) {
		std::cerr << "sendConfigMessage exception caught: " << e.what() << std::endl;
	}
}

bool LightNode::processUpdate(int bytesTransferred) {
	int payloadLength = bytesTransferred - 3;
	
	size_t i = 3;
	
	//TODO: Validate payload length

	for(auto& strip : analogStrips) {
		Color c(readBuf[i], readBuf[i+1], readBuf[i+2]);
		strip->setPixels({c});

		i += 3;
	}

	for(auto& strip : digitalStrips) {
		size_t stripSize = strip->getSize();
		std::vector<Color> colors;
		colors.reserve(stripSize);

		for(size_t j = 0; j < stripSize; ++j) {
			colors.push_back({readBuf[i], readBuf[i+1], readBuf[i+2]});
			i += 3;
		}

		strip->setPixels(colors);
	}

	for(auto& strip : matrixStrips) {
		size_t stripSize = strip->getSize();
		std::vector<Color> colors;
		colors.reserve(stripSize);

		for(size_t j = 0; j < stripSize; ++j) {
			colors.push_back({readBuf[i], readBuf[i+1], readBuf[i+2]});
			i += 3;
		}

		strip->setPixels(colors);
	}

	return true;
}
