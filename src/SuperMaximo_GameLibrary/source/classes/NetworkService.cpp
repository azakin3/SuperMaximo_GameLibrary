//============================================================================
// Name        : NetworkService.cpp
// Author      : Max Foster
// Created on  : 30 May 2011
// Version     : 1.0
// Copyright   : http://creativecommons.org/licenses/by/3.0/
// Description : SuperMaximo GameLibrary NetworkService class for networking
//============================================================================

#include <iostream>
#include <vector>
using namespace std;
#include <SDL/SDL_net.h>
#include "../../headers/classes/NetworkService.h"
#include "../../headers/Input.h"
#include "../../headers/Utils.h"
using namespace SuperMaximo;

vector<NetworkService *> allNetworkServices[27];

namespace SuperMaximo {

bool initNetworking() {
	if (SDLNet_Init() == 0) return true; else cout << SDLNet_GetError() << endl;
	return false;
}

void quitNetworking() {
	SDLNet_Quit();
}

NetworkService::NetworkService(const string & newName) {
	serverStart = false, clientStart = false, localAddress_ = 0;
	recvIntPacket = NULL, sendIntPacket = NULL, recvStrPacket = NULL, sendStrPacket = NULL;
	name_ = newName;
}

NetworkService::~NetworkService() {
	if (sendIntPacket != NULL) {
		SDLNet_FreePacket(sendIntPacket);
		sendIntPacket = NULL;
	}
	if (recvIntPacket != NULL) {
		SDLNet_FreePacket(recvIntPacket);
		recvIntPacket = NULL;
	}
	if (sendStrPacket != NULL) {
		SDLNet_FreePacket(sendStrPacket);
		sendStrPacket = NULL;
	}
	if (recvStrPacket != NULL) {
		SDLNet_FreePacket(recvStrPacket);
		recvStrPacket = NULL;
	}
	if (clientStart) closeClient();
	if (serverStart) closeServer();
}

string NetworkService::name() {
	return name_;
}

bool NetworkService::serverStarted() {
	return serverStart;
}

bool NetworkService::clientStarted() {
	return clientStart;
}

bool NetworkService::startServer(int newMaxSockets, int newPort) {
	maxSockets = newMaxSockets, serverPort = newPort;
	bool result = true;
	for (int i  = 0; i < maxSockets; i++) {
		serverTcpSockets.push_back(NULL);
		serverUdpAddresses.push_back(NULL);
	}
	cout << "Attempting to start server on port " << serverPort << endl;
	serverTcpSocketSet = SDLNet_AllocSocketSet(maxSockets+2);
	serverUdpSocketSet = SDLNet_AllocSocketSet(maxSockets+1);
	if ((serverTcpSocketSet == NULL) || (serverUdpSocketSet == NULL)) {
		cout << "Socketsets could not be initalised" << endl;
		cout << SDLNet_GetError() << endl;
		if (serverTcpSocketSet != NULL) SDLNet_FreeSocketSet(serverTcpSocketSet);
		if (serverUdpSocketSet != NULL) SDLNet_FreeSocketSet(serverUdpSocketSet);
		result = false;
	} else if (SDLNet_ResolveHost(&serverIp, NULL, serverPort) == -1) {
		cout << "Host could not be resolved for initialising server" << endl;
		cout << SDLNet_GetError() << endl;
		SDLNet_FreeSocketSet(serverTcpSocketSet);
		SDLNet_FreeSocketSet(serverUdpSocketSet);
		result = false;
	} else {
		serverTcpSocket = SDLNet_TCP_Open(&serverIp);
		serverUdpSocket = SDLNet_UDP_Open(serverPort);
		if ((serverTcpSocket == NULL) || (serverUdpSocket == NULL)) {
			cout << "TCP or UDP sockets could not be opened" << endl;
			cout << SDLNet_GetError() << endl;
			closeServer();
			result = false;
		} else if (SDLNet_TCP_AddSocket(serverTcpSocketSet, serverTcpSocket) == -1) {
			cout << "Server TCP socket could not be added to TCP socketset" << endl;
			cout << SDLNet_GetError() << endl;
			closeServer();
			result = false;
		} else if (SDLNet_UDP_AddSocket(serverUdpSocketSet, serverUdpSocket) == -1) {
			cout << "Server UDP socket could not be added to UDP socketset" << endl;
			cout << SDLNet_GetError() << endl;
			closeServer();
			result = false;
		}
	}
	if (result) {
		cout << "Server started successfully" << endl;
		cout << "Listening on port " << serverPort << endl;
		serverStart = true;
	}
	return result;
}

void NetworkService::closeServer() {
	serverStart = false;
	for (int i = 0; i < maxSockets; i++) {
		if (clientExists(i)) kickClient(i);
	}
	serverTcpSockets.clear();
	serverUdpAddresses.clear();
	if (serverTcpSocket != NULL) SDLNet_TCP_Close(serverTcpSocket);
	if (serverUdpSocket != NULL) SDLNet_UDP_Close(serverUdpSocket);
	if (serverTcpSocketSet != NULL) SDLNet_FreeSocketSet(serverTcpSocketSet);
	if (serverUdpSocketSet != NULL) SDLNet_FreeSocketSet(serverUdpSocketSet);

	cout << "Server shut down" << endl;
}

bool NetworkService::restartServer() {
	closeServer();
	return startServer(maxSockets, serverPort);
}

Uint32 NetworkService::newLocalAddress() {
	localHostName = SDLNet_ResolveIP(&localResolveIp);
	if (serverStart) SDLNet_ResolveHost(&localResolveIp, localHostName.c_str(), serverPort);
		else SDLNet_ResolveHost(&localResolveIp, localHostName.c_str(), 9001);
	localAddress_ = localResolveIp.host;
	return SDLNet_Read32(&localAddress_);
}

Uint32 NetworkService::localAddress() {
	if (localAddress_ == 0) newLocalAddress();
	return SDLNet_Read32(&localAddress_);
}

int NetworkService::checkForNewClient(bool useUdp) {
	int newClientId = -1;
	if (SDLNet_CheckSockets(serverTcpSocketSet, 0) > 0) {
		if (SDLNet_SocketReady((SDLNet_GenericSocket)serverTcpSocket)) {
			for (int i = 0; i < maxSockets; i++) {
				if (serverTcpSockets[i] == NULL) {
					serverTcpSockets[i] = SDLNet_TCP_Accept(serverTcpSocket);
					if (serverTcpSockets[i] != NULL) {
						if (SDLNet_TCP_AddSocket(serverTcpSocketSet, serverTcpSockets[i]) == -1) {
							cout << "Client TCP socket could not be added to TCP socketset (the socket may be NULL or"
									"the socketset may be full" << endl;
							SDLNet_TCP_Close(serverTcpSockets[i]);
							serverTcpSockets[i] = NULL;
						} else newClientId = i;
					}
					break;
				} else {
					if (i == maxSockets-1) {
						cout << "No free TCP sockets" << endl;
						serverTcpSockets[i+1] = SDLNet_TCP_Accept(serverTcpSocket);
						SDLNet_TCP_AddSocket(serverTcpSocketSet, serverTcpSockets[i+1]);
						do {} while (!sendStrTcp("DISCONNECTED", i+1, true));
						kickClient(i+1);
					}
				}
			}
		}
	}
	if (useUdp && (newClientId > -1)) {
		bool quit = false;
		int num, newId;
		do {
			if (recvStrTcp(newClientId, true) == "DISCONNECTED") {
				kickClient(newClientId);
				return -1;
			}
			cout << "Waiting for message via UDP" << endl;
			num = recvIntUdp(&newId, true);
			if (keyPressed(27)) quit = true;
		} while (!(((num = 123) && (newId < 0)) || quit));
		if (quit) return -1;
		serverUdpAddresses[newClientId] = new IPaddress;
		*serverUdpAddresses[newClientId] = recvIntPacket->address;
		do {
			if (keyPressed(27)) quit = true;
		} while (!(sendStrTcp("CAPTURED", newClientId, true) || quit));
		if (quit) return -1;
		do {
			if (sendIntUdp(123, newClientId, true)) cout << "Sending message via UDP to new client" << endl;
			recvStrTcp(newClientId, true);
			if (keyPressed(27)) recvStrBuffer = "DISCONNECTED";
			if (recvStrBuffer == "DISCONNECTED") break;
		} while (recvStrBuffer != "CONNECTED");
		if (recvStrBuffer == "DISCONNECTED") {
			kickClient(newClientId);
			return -1;
		} else {
			do {
				cout << "Allocating ID to new client" << endl;
				recvIntTcp(newClientId, true);
				if (keyPressed(27)) {
					kickClient(newClientId);
					return -1;
				}
			} while (!sendIntTcp(newClientId+100, newClientId, true));
			if (SDLNet_UDP_Bind(serverUdpSocket, newClientId, serverUdpAddresses[newClientId]) < 0) {
				kickClient(newClientId);
				return -1;
			}
			cout << "Client " << newClientId << " has connected" << endl;
		}
	}
	return newClientId;
}

bool NetworkService::clientExists(int id) {
	if (serverTcpSockets[id] != NULL) return false; else return true;
}

int NetworkService::totalClients() {
	int count = 0;
	for (unsigned int i = 0; i < serverTcpSockets.size(); i++) {
		if (serverTcpSockets[i] != NULL) count += 1;
	}
	return count;
}

void NetworkService::kickClient(int id) {
	bool sockPresent = false;
	if (serverTcpSockets[id] != NULL) {
		SDLNet_TCP_DelSocket(serverTcpSocketSet, serverTcpSockets[id]);
		sockPresent = true;
	}
	if (serverTcpSockets[id] != NULL) {
		SDLNet_TCP_Close(serverTcpSockets[id]);
		sockPresent = true;
	}
	serverTcpSockets[id] = NULL;
	delete serverUdpAddresses[id];
	serverUdpAddresses[id] = NULL;
	SDLNet_UDP_Unbind(serverUdpSocket, id);
	if ((id != maxSockets) && (sockPresent)) cout << "Client " << id << " disconnected" << endl;
}

bool NetworkService::startClient(const string & newAddress, int newPort) {
	clientStart = true, clientId = -1, clientAddress = newAddress, clientPort = newPort, clientUdpSocket = NULL;
	clientTcpSockets.push_back(NULL);
	clientUdpAddresses.push_back(NULL);
	clientTcpSocketSet = SDLNet_AllocSocketSet(1);
	clientUdpSocketSet = SDLNet_AllocSocketSet(1);
	if ((clientTcpSocketSet == NULL) || (clientUdpSocketSet == NULL)) {
		cout << "Could not initialise client socketsets" << endl;
		closeClient();
		return false;
	} else {
		if (SDLNet_ResolveHost(&clientIp, clientAddress.c_str(), clientPort) == -1) {
			cout << "Could not resolve host" << endl;
			closeClient();
			return false;
		}
	}
	return true;
}

void NetworkService::closeClient() {
	clientStart = false;
	if (clientUdpAddresses[0] != NULL) delete clientUdpAddresses[0];
	clientUdpAddresses.clear();
	if (clientTcpSockets[0] != NULL) {
		SDLNet_TCP_Close(clientTcpSockets[0]);
		cout << "Closing TCP connection to server" << endl;
	}
	clientTcpSockets[0] = NULL;
	clientTcpSockets.clear();
	if (clientUdpSocket != NULL) {
		SDLNet_UDP_Close(clientUdpSocket);
		cout << "Closing UDP connections on port " << clientPort << endl;
	}
	clientUdpSocket = NULL;
	if (clientTcpSocketSet != NULL) SDLNet_FreeSocketSet(clientTcpSocketSet);
	clientTcpSocketSet = NULL;
	if (clientUdpSocketSet != NULL) SDLNet_FreeSocketSet(clientUdpSocketSet);
	clientUdpSocketSet = NULL;
	cout << "Client shut down" << endl;
}

bool NetworkService::restartClient() {
	closeClient();
	return startClient(clientAddress, clientPort);
}

bool NetworkService::connectToServer(bool useUdp) {
	clientTcpSockets[0] = SDLNet_TCP_Open(&clientIp);
	if (clientTcpSockets[0] != NULL) {
		SDLNet_TCP_AddSocket(clientTcpSocketSet, clientTcpSockets[0]);
		if (useUdp) {
			clientUdpSocket = SDLNet_UDP_Open(0);
			if (clientUdpSocket != NULL) {
				SDLNet_UDP_AddSocket(clientUdpSocketSet, clientUdpSocket);
				clientUdpAddresses[0] = new IPaddress;
				*clientUdpAddresses[0] = clientIp;
			} else {
				cout << "UDP socket could not be opened" << endl;
				return false;
			}
		}
	} else {
		cout << "TCP socket with server could not be opened" << endl;
		return false;
	}
	if (!useUdp) return true;
	do {
		if (sendIntUdp(123)) cout << "Sending message via UDP" << endl;
		recvStrTcp();
		if (keyPressed(27)) recvStrBuffer = "DISCONNECTED";
		if (recvStrBuffer == "DISCONNECTED") break;
	} while (recvStrBuffer != "CAPTURED");
	if (recvStrBuffer == "DISCONNECTED") {
		closeClient();
		return false;
	} else {
		int num, newId;
		do {
			cout << "Waiting for response from server..." << endl;
			num = recvIntUdp(&newId);
			if (keyPressed(27)) num = -1;
			if (num == -1) break;
		} while (!((num == 123) && (newId == -1)));
		if (recvStrBuffer != "DISCONNECTED") {
			do {
				cout << "Sending success message to server via TCP" << endl;
				if (keyPressed(27)) {
					closeClient();
					return false;
				}
			} while(!sendStrTcp("CONNECTED"));
			cout << "Waiting for response from server..." << endl;
			do {
				if (keyPressed(27)) {
					closeClient();
					return false;
				}
				cout << "Waiting for client ID allocation from server via TCP" << endl;
				clientId = recvIntTcp();
				cout << clientId << " received from server" << endl;
				if (clientId < 100) sendStrTcp("NEEDID");
			} while (!((clientId == -1) || (clientId > 99)));
			if (clientId > 99) {
				clientId -= 100;
				cout << "Connected to server" << endl << "ID for this client is " << clientId << endl;
				SDLNet_UDP_Bind(clientUdpSocket, clientId, clientUdpAddresses[0]);
				return true;
			} else {
				closeClient();
				return false;
			}
		} else {
			closeClient();
			return false;
		}
	}
}

int NetworkService::clientNumber() {
	return clientId;
}

bool NetworkService::sendStrTcp(const string & data, int id, bool isServer, int size) {
	if (size < 1) size = STR_SIZE;
	if (isServer) SDLNet_CheckSockets(serverTcpSocketSet, WAIT_TIME);
		else SDLNet_CheckSockets(clientTcpSocketSet, WAIT_TIME);
	if (id >= 0) {
		TCPsocket sock = NULL;
		if (isServer) sock = serverTcpSockets[id]; else sock = clientTcpSockets[0];
		sendStrBuffer = data;
		if (sock != NULL) {
			if (!SDLNet_SocketReady((SDLNet_GenericSocket)sock)) {
				char buffer[size];
				for (int i = 0; i < size; i++) buffer[i] = 0;
				for (unsigned int i = 0; i < sendStrBuffer.size(); i++) buffer[i] = sendStrBuffer[i];
				if (SDLNet_TCP_Send(sock, &buffer, size) < size) return false;
			} else return false;
		} else return false;
	} else return false;
	return true;
}

string NetworkService::recvStrTcp(int id, bool isServer, int size) {
	string returnStr = "";
	if (size < 1) size = STR_SIZE;
	int i = -1;
	if (isServer) i = SDLNet_CheckSockets(serverTcpSocketSet, WAIT_TIME);
		else i = SDLNet_CheckSockets(clientTcpSocketSet, WAIT_TIME);
	if (i > -1) {
		TCPsocket sock = NULL;
		if (isServer) sock = serverTcpSockets[id]; else sock = clientTcpSockets[0];
		if (sock != NULL) {
			if (SDLNet_SocketReady((SDLNet_GenericSocket)sock)) {
				char buffer[STR_BUFFER_SIZE];
				if (SDLNet_TCP_Recv(sock, &buffer, size) < 1) {
					recvStrBuffer = "DISCONNECTED";
					returnStr = "DISCONNECTED";
				} else {
					recvStrBuffer = "";
					for (int i = 0; buffer[i] != 0; i++) recvStrBuffer += buffer[i];
					returnStr = recvStrBuffer;
				}
			}
		}
	}
	if (isServer && (returnStr == "NEEDID")) {
		if (sendIntTcp(id+100, id, true)) cout << "Sending ID to a client via TCP" << endl; else {
			do; while (recvStrTcp(id, isServer, size) != "");
			if (sendIntTcp(id+100, id, true)) cout << "Sending ID to a client via TCP" << endl;
		}
		returnStr = "";
	}
	return returnStr;
}

bool NetworkService::sendIntTcp(int data, int id, bool isServer) {
	if (isServer) SDLNet_CheckSockets(serverTcpSocketSet, WAIT_TIME);
		else SDLNet_CheckSockets(clientTcpSocketSet, WAIT_TIME);
	if (id >= 0) {
		TCPsocket sock = NULL;
		if (isServer) sock = serverTcpSockets[id]; else sock = clientTcpSockets[0];
		sendIntBuffer = data;
		if (sock != NULL) {
			if (!SDLNet_SocketReady((SDLNet_GenericSocket)sock)) {
				if (SDLNet_TCP_Send(sock, &sendIntBuffer, INT_SIZE) < INT_SIZE) return false;
			} else return false;
		} else return false;
	} else return false;
	return true;
}

int NetworkService::recvIntTcp(int id, bool isServer) {
	int i = -1;
	if (isServer) i = SDLNet_CheckSockets(serverTcpSocketSet, WAIT_TIME);
		else i = SDLNet_CheckSockets(clientTcpSocketSet, WAIT_TIME);
	if (i > -1) {
		TCPsocket sock = NULL;
		if (isServer) sock = serverTcpSockets[id]; else sock = clientTcpSockets[0];
		if (sock != NULL) {
			if (SDLNet_SocketReady((SDLNet_GenericSocket)sock)) {
				if (SDLNet_TCP_Recv(sock, &recvIntBuffer, INT_SIZE) < 1) return -1; else return recvIntBuffer;
			}
		}
	}
	return 0;
}

bool NetworkService::sendStrUdp(const string & data, int id, bool isServer) {
	if (isServer) SDLNet_CheckSockets(serverUdpSocketSet, WAIT_TIME);
		else SDLNet_CheckSockets(clientUdpSocketSet, WAIT_TIME);
	if (id >= 0) {
		UDPsocket sock = NULL;
		if (isServer) sock = serverUdpSocket; else sock = clientUdpSocket;
		if (!SDLNet_SocketReady((SDLNet_GenericSocket)sock)) {
			if (sendStrPacket != NULL) {
				SDLNet_FreePacket(sendStrPacket);
				sendStrPacket = NULL;
			}
			IPaddress * addr = NULL;
			if (isServer) addr = serverUdpAddresses[id]; else addr = clientUdpAddresses[0];
			sendStrPacket = SDLNet_AllocPacket(data.size());
			sendStrPacket->address = *addr;
			sendStrPacket->data = (Uint8 *)data.c_str();
			sendStrPacket->len = data.size();
			sendStrPacket->maxlen = data.size();
			int sendId;
			if (isServer) sendId = -1; else sendId = clientId;
			if (SDLNet_UDP_Send(sock, sendId, sendStrPacket) == 0) return false;
		} else {
			recvStrUdp(isServer);
			return sendStrUdp(data, id, isServer);
		}
	} else return false;
	return true;
}

string NetworkService::recvStrUdp(bool isServer) {
	int i = -1;
	string returnStr = "";
	if (isServer) i = SDLNet_CheckSockets(serverUdpSocketSet, WAIT_TIME);
		else i = SDLNet_CheckSockets(clientUdpSocketSet, WAIT_TIME);
	if (i > -1) {
		UDPsocket sock = NULL;
		if (isServer) sock = serverUdpSocket; else sock = clientUdpSocket;
		if (SDLNet_SocketReady((SDLNet_GenericSocket)sock)) {
			if (recvStrPacket != NULL) {
				SDLNet_FreePacket(recvStrPacket);
				recvStrPacket = NULL;
			}
			recvStrPacket = SDLNet_AllocPacket(STR_SIZE);
			if (SDLNet_UDP_Recv(sock, recvStrPacket) == -1) returnStr = "ERROR"; else {
				for (i = 0; i < recvStrPacket->len; i++) returnStr += (char)recvStrPacket->data[i];
			}
		} else return "";
	}
	return returnStr;
}

string NetworkService::recvStrUdp(int * idBuffer, bool isServer) {
	int i = -1;
	string returnStr = "";
	if (isServer) i = SDLNet_CheckSockets(serverUdpSocketSet, WAIT_TIME);
		else i = SDLNet_CheckSockets(clientUdpSocketSet, WAIT_TIME);
	if (i > -1) {
		UDPsocket sock = NULL;
		if (isServer) sock = serverUdpSocket; else sock = clientUdpSocket;
		if (SDLNet_SocketReady((SDLNet_GenericSocket)sock)) {
			if (recvStrPacket != NULL) {
				SDLNet_FreePacket(recvStrPacket);
				recvStrPacket = NULL;
			}
			recvStrPacket = SDLNet_AllocPacket(STR_SIZE);
			if (SDLNet_UDP_Recv(sock, recvStrPacket) == -1) {
				returnStr = "ERROR";
				*idBuffer = -1;
			} else {
				for (i = 0; i < recvStrPacket->len; i++) returnStr += (char)recvStrPacket->data[i];
				*idBuffer = recvStrPacket->channel;
			}
		} else return "";
	}
	return returnStr;
}

int NetworkService::recvStrUdp(string * stringBuffer, bool isServer) {
	int i = -1;
	*stringBuffer = "";
	if (isServer) i = SDLNet_CheckSockets(serverUdpSocketSet, WAIT_TIME);
		else i = SDLNet_CheckSockets(clientUdpSocketSet, WAIT_TIME);
	if (i > -1) {
		UDPsocket sock = NULL;
		if (isServer) sock = serverUdpSocket; else sock = clientUdpSocket;
		if (SDLNet_SocketReady((SDLNet_GenericSocket)sock)) {
			if (recvStrPacket != NULL) {
				SDLNet_FreePacket(recvStrPacket);
				recvStrPacket = NULL;
			}
			recvStrPacket = SDLNet_AllocPacket(STR_SIZE);
			if (SDLNet_UDP_Recv(sock, recvStrPacket) == -1) {
				*stringBuffer = "ERROR";
				i = -1;
			} else {
				for (i = 0; i < recvStrPacket->len; i++) *stringBuffer += (char)recvStrPacket->data[i];
				i = recvStrPacket->channel;
			}
		} else return -1;
	}
	return i;
}

bool NetworkService::sendIntUdp(int data, int id, bool isServer) {
	if (isServer) SDLNet_CheckSockets(serverUdpSocketSet, WAIT_TIME);
		else SDLNet_CheckSockets(clientUdpSocketSet, WAIT_TIME);
	if (id >= 0) {
		UDPsocket sock = NULL;
		if (isServer) sock = serverUdpSocket; else sock = clientUdpSocket;
		if (!SDLNet_SocketReady((SDLNet_GenericSocket)sock)) {
			if (sendIntPacket != NULL) {
				SDLNet_FreePacket(sendIntPacket);
				sendIntPacket = NULL;
			}
			IPaddress * addr = NULL;
			if (isServer) addr = serverUdpAddresses[id]; else addr = clientUdpAddresses[0];
			sendIntPacket = SDLNet_AllocPacket(INT_SIZE);
			sendIntPacket->address = *addr;
			sendIntPacket->data[0] = data;
			sendIntPacket->len = INT_SIZE;
			sendIntPacket->maxlen = INT_SIZE;
			int sendId;
			if (isServer) sendId = -1; else sendId = clientId;
			if (SDLNet_UDP_Send(sock, sendId, sendIntPacket) == 0) return false;
		} else {
			recvIntUdp(isServer);
			return sendIntUdp(data, id, isServer);
		}
	} else return false;
	return true;
}

int NetworkService::recvIntUdp(bool isServer) {
	int i = -1;
	if (isServer) i = SDLNet_CheckSockets(serverUdpSocketSet, WAIT_TIME);
		else i = SDLNet_CheckSockets(clientUdpSocketSet, WAIT_TIME);
	if (i > -1) {
		UDPsocket sock = NULL;
		if (isServer) sock = serverUdpSocket; else sock = clientUdpSocket;
		if (SDLNet_SocketReady((SDLNet_GenericSocket)sock)) {
			if (recvIntPacket != NULL) {
				SDLNet_FreePacket(recvIntPacket);
				recvIntPacket = NULL;
			}
			recvIntPacket = SDLNet_AllocPacket(INT_SIZE);
			if (SDLNet_UDP_Recv(sock, recvIntPacket) == -1) i = -1; else i = recvIntPacket->data[0];
		} else return -1;
	}
	return i;
}

int NetworkService::recvIntUdp(int * idBuffer, bool isServer) {
	int i = -1;
	if (isServer) i = SDLNet_CheckSockets(serverUdpSocketSet, WAIT_TIME);
		else i = SDLNet_CheckSockets(clientUdpSocketSet, WAIT_TIME);
	if (i > -1) {
		UDPsocket sock = NULL;
		if (isServer) sock = serverUdpSocket; else sock = clientUdpSocket;
		if (SDLNet_SocketReady((SDLNet_GenericSocket)sock)) {
			if (recvIntPacket != NULL) {
				SDLNet_FreePacket(recvIntPacket);
				recvIntPacket = NULL;
			}
			recvIntPacket = SDLNet_AllocPacket(INT_SIZE);
			if (SDLNet_UDP_Recv(sock, recvIntPacket) == -1) {
				i = -1;
				*idBuffer = -1;
			} else {
				i = recvIntPacket->data[0];
				*idBuffer = recvIntPacket->channel;
			}
		} else return -1;
	}
	return i;
}

int NetworkService::recvIntUdpId(int * intBuffer, bool isServer) {
	int i = -1;
	if (isServer) i = SDLNet_CheckSockets(serverUdpSocketSet, WAIT_TIME);
		else i = SDLNet_CheckSockets(clientUdpSocketSet, WAIT_TIME);
	if (i > -1) {
		UDPsocket sock = NULL;
		if (isServer) sock = serverUdpSocket; else sock = clientUdpSocket;
		if (SDLNet_SocketReady((SDLNet_GenericSocket)sock)) {
			if (recvIntPacket != NULL) {
				SDLNet_FreePacket(recvIntPacket);
				recvIntPacket = NULL;
			}
			recvIntPacket = SDLNet_AllocPacket(INT_SIZE);
			if (SDLNet_UDP_Recv(sock, recvIntPacket) == -1) {
				*intBuffer = -1;
				i = -1;
			} else {
				*intBuffer = recvIntPacket->data[0];
				i = recvIntPacket->channel;
			}
		} else return -1;
	}
	return i;
}

NetworkService * networkService(const string & searchName) {
	int letter = numCharInAlphabet(searchName[0]);
	NetworkService * returnNetworkService = NULL;
	if (allNetworkServices[letter].size() > 0) {
		for (unsigned int i = 0; i < allNetworkServices[letter].size(); i++) {
			if (allNetworkServices[letter][i]->name() == searchName) {
				returnNetworkService = allNetworkServices[letter][i];
				break;
			}
		}
	}
	return returnNetworkService;
}

NetworkService * addNetworkService(const string & newName) {
	int letter = numCharInAlphabet(newName[0]);
	NetworkService * newNetworkService = new NetworkService(newName);
	allNetworkServices[letter].push_back(newNetworkService);
	return newNetworkService;
}

void destroyNetworkService(const string & searchName) {
	int letter = numCharInAlphabet(searchName[0]);
	if (allNetworkServices[letter].size() > 0) {
		for (unsigned int i = 0; i < allNetworkServices[letter].size(); i++) {
			if (allNetworkServices[letter][i]->name() == searchName) {
				delete allNetworkServices[letter][i];
				allNetworkServices[letter].erase(allNetworkServices[letter].begin()+i);
				break;
			}
		}
	}
}

void destroyAllNetworkServices() {
	for (int i = 0; i < 27; i++) {
		if (allNetworkServices[i].size() > 0) {
			for (unsigned int j = 0; j < allNetworkServices[i].size(); j++) delete allNetworkServices[i][j];
			allNetworkServices[i].clear();
		}
	}
}

}
