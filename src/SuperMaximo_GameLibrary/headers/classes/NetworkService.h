//============================================================================
// Name        : NetworkService.h
// Author      : Max Foster
// Created on  : 30 May 2011
// Version     : 1.0
// Copyright   : http://creativecommons.org/licenses/by/3.0/
// Description : SuperMaximo GameLibrary NetworkService class for networking
//============================================================================

#ifndef NETWORKSERVICE_H_
#define NETWORKSERVICE_H_

#include <iostream>
#include <vector>

#include <SDL/SDL_net.h>

namespace SuperMaximo {

const int STR_BUFFER_SIZE = 128;
const int STR_SIZE = 32;
const int INT_SIZE = 8;
const int WAIT_TIME = 10;

bool initNetworking();
void quitNetworking();

class NetworkService {
	bool serverStarted_, clientStarted_;
	IPaddress serverIp, clientIp, localResolveIp;
	std::string clientAddress, localHostName, name_;
	SDLNet_SocketSet serverTcpSocketSet, serverUdpSocketSet, clientTcpSocketSet, clientUdpSocketSet;
	std::vector<TCPsocket> serverTcpSockets, clientTcpSockets;
	TCPsocket serverTcpSocket;
	UDPsocket serverUdpSocket, clientUdpSocket;
	std::vector<IPaddress *> serverUdpAddresses, clientUdpAddresses;
	std::string recvStrBuffer, sendStrBuffer;
	int recvIntBuffer, sendIntBuffer, clientId, maxSockets, serverPort, clientPort;
	UDPpacket * recvIntPacket, * sendIntPacket, * recvStrPacket, * sendStrPacket;
	Uint32 localAddress_;

public:
	NetworkService(const std::string & name);
	~NetworkService();

	const std::string & name() const;

	bool serverStarted() const;
	bool clientStarted() const;

	bool startServer(int maxSockets, int port);
	void closeServer();
	bool restartServer();
	Uint32 newLocalAddress();
	Uint32 localAddress();
	int checkForNewClient(bool useUdp = true);
	bool clientExists(int id) const;
	int totalClients() const;
	void kickClient(int id);

	bool startClient(const std::string & address, int port);
	void closeClient();
	bool restartClient();
	bool connectToServer(bool useUdp = true);
	int clientNumber() const;

	bool sendStrTcp(const std::string & data, int id = 0, bool isServer = false, int size = 0);
	std::string recvStrTcp(int id = 0, bool isServer = false, int size = 0);
	bool sendIntTcp(int data, int id = 0, bool isServer = false);
	int recvIntTcp(int id = 0, bool isServer = false);

	bool sendStrUdp(const std::string & data, int id = 0, bool isServer = false);
	std::string recvStrUdp(bool isServer = false);
	std::string recvStrUdp(int * idBuffer, bool isServer = false);
	int recvStrUdp(std::string * stringBuffer, bool isServer = false);
	bool sendIntUdp(int data, int id = 0, bool isServer = false);
	int recvIntUdp(bool isServer = false);
	int recvIntUdp(int * idBuffer, bool isServer = false);
	int recvIntUdpId(int * intBuffer, bool isServer = false);
};

}

#endif /* NETWORKSERVICE_H_ */
