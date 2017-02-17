#include "Communicator.hpp"

using namespace std;

Communicator::Communicator(NodeType _nodeType,
	uint16_t _pixelCount, function<void(vector<Color>&)> _cbUpdate)
	:	ioWork(new boost::asio::io_service::work(ioService))
	,	udpSocket(ioService)
	,	clientEndpoint(boost::asio::ip::udp::v4(), PORT_SEND)
	,	recvEndpoint(boost::asio::ip::udp::v4(), PORT_RECV)
	,	aliveTimer(ioService)
	,	watchdogTimer(ioService)
	, asyncThread(bind(&Communicator::threadRoutine, this))
	,	connected(false)
	,	pixelCount(_pixelCount)
	,	nodeType(_nodeType)
	,	cbUpdate(_cbUpdate) {

	udpSocket.open(boost::asio::ip::udp::v4());

	//Bind the socket
	udpSocket.bind(recvEndpoint);

	startListening();
}

Communicator::~Communicator() {
	ioWork.reset();
	ioService.stop();

	asyncThread.join();
}

bool Communicator::isConnected() {
	return connected;
}

void Communicator::startListening() {
	udpSocket.async_receive_from(boost::asio::buffer(readBuf),
		recvEndpoint,
		std::bind(&Communicator::handleReceive, this,
			std::placeholders::_1, std::placeholders::_2));
}

void Communicator::handleReceive(const boost::system::error_code& error,
	size_t bytesTransferred) {

	if(error) {
		cout << "[Error] handleReceive: " << error.message() << endl;
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

						cout << "[Info] Client connected" << endl;
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
					cout << "[Error] Unimplemented packet id received: "
						<< static_cast<int>(readId) << endl;
				break;
			}
		}
		else { //The packet header was not correct
			cout << "[Error] Packet received with incorrect header: "
				<< (int)readHeader << endl;
		}
	}

	startListening();
}

void Communicator::threadRoutine() {
		ioService.run();
}

void Communicator::cbAliveTimer(const boost::system::error_code& ec) {
	if(ec) {
		if(ec.value() == boost::system::errc::operation_canceled) {
			return;
		}
		else {
			cout << "[Error] cbAliveTimer: " << ec.message() << endl;
		}
	}
	else if(!connected) {
		return;
	}
	else {
		vector<unsigned char> message;

		//Push the header
		message.push_back( (HEADER >> 8) & 0xFF );
		message.push_back( HEADER & 0xFF );

		//Push the identification
		message.push_back(static_cast<unsigned char>(PacketID::ALIVE));

		try {
			udpSocket.send_to(boost::asio::buffer(message), clientEndpoint);
		}
		catch(exception& e) {
			cerr << "sendAck exception caught: " << e.what() << endl;
		}
	}

	startAliveTimer();
}

void Communicator::cbWatchdogTimer(const boost::system::error_code& ec) {
	if(ec.value() != boost::system::errc::operation_canceled) {
		aliveTimer.cancel();
		connected = false;

		cout << "[Info] Client timed out" << endl;
	}
}

void Communicator::startAliveTimer() {
	aliveTimer.expires_from_now(boost::posix_time::milliseconds(ALIVE_TIMEOUT));
	aliveTimer.async_wait(std::bind(&Communicator::cbAliveTimer, this,
		std::placeholders::_1));
}

void Communicator::feedWatchdog() {
	watchdogTimer.cancel();
	
	watchdogTimer.expires_from_now(
		boost::posix_time::milliseconds(WATCHDOG_TIMEOUT));
	watchdogTimer.async_wait(std::bind(&Communicator::cbWatchdogTimer, this,
		std::placeholders::_1));
}


void Communicator::sendAck(const boost::asio::ip::udp::endpoint& endpoint) {
	vector<unsigned char> message;

	//Push the header
	message.push_back( (HEADER >> 8) & 0xFF );
	message.push_back( HEADER & 0xFF );

	//Push the identification
	message.push_back(static_cast<unsigned char>(PacketID::ACK));

	try {
		udpSocket.send_to(boost::asio::buffer(message), endpoint);
	}
	catch(exception& e) {
		cerr << "sendAck exception caught: " << e.what() << endl;
	}
}

void Communicator::sendNack(const boost::asio::ip::udp::endpoint& endpoint,
	Communicator::PacketID id) {
	vector<unsigned char> message;

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
	catch(exception& e) {
		cerr << "sendAck exception caught: " << e.what() << endl;
	}
}

void Communicator::sendInfo(const boost::asio::ip::udp::endpoint& endpoint) {
	vector<unsigned char> message;

	//Push the header
	message.push_back( (HEADER >> 8) & 0xFF );
	message.push_back( HEADER & 0xFF );

	//Push the identification
	message.push_back(static_cast<unsigned char>(PacketID::INFO));

	//Push the type
	message.push_back(static_cast<unsigned char>(nodeType));

	//Push the pixel count
	message.push_back( (pixelCount >> 8) & 0xFF );
	message.push_back( pixelCount & 0xFF );

	try {
		std::cout << "[Info] Communicator::sendInfo: Sending info to "
			<< endpoint << std::endl;

		udpSocket.send_to(boost::asio::buffer(message), endpoint);
	}
	catch(exception& e) {
		cerr << "sendConfigMessage exception caught: " << e.what() << endl;
	}
}

bool Communicator::processUpdate(int bytesTransferred) {
	int payloadLength = bytesTransferred - 3;

	if(payloadLength != (3*pixelCount)) {
		cerr << "processUpdate error: invalid payload length "
			<< payloadLength << endl;

		return false;
	}

	vector<Color> pixels;

	for(int i = 3; i < bytesTransferred; i += 3) {
		pixels.emplace_back(readBuf[i], readBuf[i+1], readBuf[i+2]);
	}

	try {
		//Call external callback for update event
		cbUpdate(pixels);
	}
	catch(exception &e) {
		cerr << "processUpdate error: " << e.what() << endl;
	}

	return true;
}
