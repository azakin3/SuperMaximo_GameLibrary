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

#define STR_BUFFER_SIZE 128
#define STR_SIZE 32
#define INT_SIZE 8
#define WAIT_TIME 10

bool initNetworking();
void quitNetworking();

class NetworkService {
	bool serverStart, clientStart;
	IPaddress serverIP, clientIP, localResolveIP;
	std::string clientAddress, localHostName, name_;
	SDLNet_SocketSet serverTCPSocketSet, serverUDPSocketSet, clientTCPSocketSet, clientUDPSocketSet;
	std::vector<TCPsocket> serverTCPSockets, clientTCPSockets;
	TCPsocket serverTCPSocket;
	UDPsocket serverUDPSocket, clientUDPSocket;
	std::vector<IPaddress *> serverUDPAddresses, clientUDPAddresses;
	std::string recvStrBuffer, sendStrBuffer;
	int recvIntBuffer, sendIntBuffer, clientID, maxSockets, serverPort, clientPort;
	UDPpacket * recvIntPacket, * sendIntPacket, * recvStrPacket, * sendStrPacket;
	Uint32 localAddress_;
public:
	NetworkService(std::string newName);
	~NetworkService();

	std::string name();

	bool serverStarted();
	bool clientStarted();

	bool startServer(int newMaxSockets, int newPort);
	void closeServer();
	bool restartServer();
	Uint32 newLocalAddress();
	Uint32 localAddress();
	int checkForNewClient(bool useUDP = true);
	bool clientExists(int ID);
	int totalClients();
	void kickClient(int ID);

	bool startClient(std::string newAddress, int newPort);
	void closeClient();
	bool restartClient();
	bool connectToServer(bool useUDP = true);
	int clientNumber();

	bool sendStrTCP(std::string data, int ID = 0, bool isServer = false, int size = 0);
	std::string recvStrTCP(int ID = 0, bool isServer = false, int size = 0);
	bool sendIntTCP(int data, int ID = 0, bool isServer = false);
	int recvIntTCP(int ID = 0, bool isServer = false);

	bool sendStrUDP(std::string data, int ID = 0, bool isServer = false);
	std::string recvStrUDP(bool isServer = false);
	std::string recvStrUDP(int * IDBuffer, bool isServer = false);
	int recvStrUDP(std::string * stringBuffer, bool isServer = false);
	bool sendIntUDP(int data, int ID = 0, bool isServer = false);
	int recvIntUDP(bool isServer = false);
	int recvIntUDP(int * IDBuffer, bool isServer = false);
	int recvIntUDPID(int * intBuffer, bool isServer = false);
};

NetworkService * networkService(std::string searchName);

NetworkService * addNetworkService(std::string newName);

NetworkService * addNetworkService(std::string newName);

void destroyNetworkService(std::string searchName);

void destroyAllNetworkServices();

}

#endif /* NETWORKSERVICE_H_ */
