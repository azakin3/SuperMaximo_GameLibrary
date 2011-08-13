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

NetworkService::NetworkService(string newName) {
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
		serverTCPSockets.push_back(NULL);
		serverUDPAddresses.push_back(NULL);
	}
	cout << "Attempting to start server on port " << serverPort << endl;
	serverTCPSocketSet = SDLNet_AllocSocketSet(maxSockets+2);
	serverUDPSocketSet = SDLNet_AllocSocketSet(maxSockets+1);
	if ((serverTCPSocketSet == NULL) || (serverUDPSocketSet == NULL)) {
		cout << "Socketsets could not be initalised" << endl;
		cout << SDLNet_GetError() << endl;
		if (serverTCPSocketSet != NULL) SDLNet_FreeSocketSet(serverTCPSocketSet);
		if (serverUDPSocketSet != NULL) SDLNet_FreeSocketSet(serverUDPSocketSet);
		result = false;
	} else if (SDLNet_ResolveHost(&serverIP, NULL, serverPort) == -1) {
		cout << "Host could not be resolved for initialising server" << endl;
		cout << SDLNet_GetError() << endl;
		SDLNet_FreeSocketSet(serverTCPSocketSet);
		SDLNet_FreeSocketSet(serverUDPSocketSet);
		result = false;
	} else {
		serverTCPSocket = SDLNet_TCP_Open(&serverIP);
		serverUDPSocket = SDLNet_UDP_Open(serverPort);
		if ((serverTCPSocket == NULL) || (serverUDPSocket == NULL)) {
			cout << "TCP or UDP sockets could not be opened" << endl;
			cout << SDLNet_GetError() << endl;
			closeServer();
			result = false;
		} else if (SDLNet_TCP_AddSocket(serverTCPSocketSet, serverTCPSocket) == -1) {
			cout << "Server TCP socket could not be added to TCP socketset" << endl;
			cout << SDLNet_GetError() << endl;
			closeServer();
			result = false;
		} else if (SDLNet_UDP_AddSocket(serverUDPSocketSet, serverUDPSocket) == -1) {
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
	serverTCPSockets.clear();
	serverUDPAddresses.clear();
	if (serverTCPSocket != NULL) SDLNet_TCP_Close(serverTCPSocket);
	if (serverUDPSocket != NULL) SDLNet_UDP_Close(serverUDPSocket);
	if (serverTCPSocketSet != NULL) SDLNet_FreeSocketSet(serverTCPSocketSet);
	if (serverUDPSocketSet != NULL) SDLNet_FreeSocketSet(serverUDPSocketSet);

	cout << "Server shut down" << endl;
}

bool NetworkService::restartServer() {
	closeServer();
	return startServer(maxSockets, serverPort);
}

Uint32 NetworkService::newLocalAddress() {
	localHostName = SDLNet_ResolveIP(&localResolveIP);
	if (serverStart) SDLNet_ResolveHost(&localResolveIP, localHostName.c_str(), serverPort);
		else SDLNet_ResolveHost(&localResolveIP, localHostName.c_str(), 9001);
	localAddress_ = localResolveIP.host;
	return SDLNet_Read32(&localAddress_);
}

Uint32 NetworkService::localAddress() {
	if (localAddress_ == 0) newLocalAddress();
	return SDLNet_Read32(&localAddress_);
}

int NetworkService::checkForNewClient(bool useUDP) {
	int newClientID = -1;
	if (SDLNet_CheckSockets(serverTCPSocketSet, 0) > 0) {
		if (SDLNet_SocketReady((SDLNet_GenericSocket)serverTCPSocket)) {
			for (int i = 0; i < maxSockets; i++) {
				if (serverTCPSockets[i] == NULL) {
					serverTCPSockets[i] = SDLNet_TCP_Accept(serverTCPSocket);
					if (serverTCPSockets[i] != NULL) {
						if (SDLNet_TCP_AddSocket(serverTCPSocketSet, serverTCPSockets[i]) == -1) {
							cout << "Client TCP socket could not be added to TCP socketset (the socket may be NULL or"
									"the socketset may be full" << endl;
							SDLNet_TCP_Close(serverTCPSockets[i]);
							serverTCPSockets[i] = NULL;
						} else newClientID = i;
					}
					break;
				} else {
					if (i == maxSockets-1) {
						cout << "No free TCP sockets" << endl;
						serverTCPSockets[i+1] = SDLNet_TCP_Accept(serverTCPSocket);
						SDLNet_TCP_AddSocket(serverTCPSocketSet, serverTCPSockets[i+1]);
						do {} while (!sendStrTCP("DISCONNECTED", i+1, true));
						kickClient(i+1);
					}
				}
			}
		}
	}
	if (useUDP && (newClientID > -1)) {
		bool quit = false;
		int num, newID;
		do {
			if (recvStrTCP(newClientID, true) == "DISCONNECTED") {
				kickClient(newClientID);
				return -1;
			}
			cout << "Waiting for message via UDP" << endl;
			num = recvIntUDP(&newID, true);
			if (keyPressed(27)) quit = true;
		} while (!(((num = 123) && (newID < 0)) || quit));
		if (quit) return -1;
		serverUDPAddresses[newClientID] = new IPaddress;
		*serverUDPAddresses[newClientID] = recvIntPacket->address;
		do {
			if (keyPressed(27)) quit = true;
		} while (!(sendStrTCP("CAPTURED", newClientID, true) || quit));
		if (quit) return -1;
		do {
			if (sendIntUDP(123, newClientID, true)) cout << "Sending message via UDP to new client" << endl;
			recvStrTCP(newClientID, true);
			if (keyPressed(27)) recvStrBuffer = "DISCONNECTED";
			if (recvStrBuffer == "DISCONNECTED") break;
		} while (recvStrBuffer != "CONNECTED");
		if (recvStrBuffer == "DISCONNECTED") {
			kickClient(newClientID);
			return -1;
		} else {
			do {
				cout << "Allocating ID to new client" << endl;
				recvIntTCP(newClientID, true);
				if (keyPressed(27)) {
					kickClient(newClientID);
					return -1;
				}
			} while (!sendIntTCP(newClientID+100, newClientID, true));
			if (SDLNet_UDP_Bind(serverUDPSocket, newClientID, serverUDPAddresses[newClientID]) < 0) {
				kickClient(newClientID);
				return -1;
			}
			cout << "Client " << newClientID << " has connected" << endl;
		}
	}
	return newClientID;
}

bool NetworkService::clientExists(int ID) {
	if (serverTCPSockets[ID] != NULL) return false; else return true;
}

int NetworkService::totalClients() {
	int count = 0;
	for (unsigned int i = 0; i < serverTCPSockets.size(); i++) {
		if (serverTCPSockets[i] != NULL) count += 1;
	}
	return count;
}

void NetworkService::kickClient(int ID) {
	bool sockPresent = false;
	if (serverTCPSockets[ID] != NULL) {
		SDLNet_TCP_DelSocket(serverTCPSocketSet, serverTCPSockets[ID]);
		sockPresent = true;
	}
	if (serverTCPSockets[ID] != NULL) {
		SDLNet_TCP_Close(serverTCPSockets[ID]);
		sockPresent = true;
	}
	serverTCPSockets[ID] = NULL;
	delete serverUDPAddresses[ID];
	serverUDPAddresses[ID] = NULL;
	SDLNet_UDP_Unbind(serverUDPSocket, ID);
	if ((ID != maxSockets) && (sockPresent)) cout << "Client " << ID << " disconnected" << endl;
}

bool NetworkService::startClient(string newAddress, int newPort) {
	clientStart = true, clientID = -1, clientAddress = newAddress, clientPort = newPort, clientUDPSocket = NULL;
	clientTCPSockets.push_back(NULL);
	clientUDPAddresses.push_back(NULL);
	clientTCPSocketSet = SDLNet_AllocSocketSet(1);
	clientUDPSocketSet = SDLNet_AllocSocketSet(1);
	if ((clientTCPSocketSet == NULL) || (clientUDPSocketSet == NULL)) {
		cout << "Could not initialise client socketsets" << endl;
		closeClient();
		return false;
	} else {
		if (SDLNet_ResolveHost(&clientIP, clientAddress.c_str(), clientPort) == -1) {
			cout << "Could not resolve host" << endl;
			closeClient();
			return false;
		}
	}
	return true;
}

void NetworkService::closeClient() {
	clientStart = false;
	if (clientUDPAddresses[0] != NULL) delete clientUDPAddresses[0];
	clientUDPAddresses.clear();
	if (clientTCPSockets[0] != NULL) {
		SDLNet_TCP_Close(clientTCPSockets[0]);
		cout << "Closing TCP connection to server" << endl;
	}
	clientTCPSockets[0] = NULL;
	clientTCPSockets.clear();
	if (clientUDPSocket != NULL) {
		SDLNet_UDP_Close(clientUDPSocket);
		cout << "Closing UDP connections on port " << clientPort << endl;
	}
	clientUDPSocket = NULL;
	if (clientTCPSocketSet != NULL) SDLNet_FreeSocketSet(clientTCPSocketSet);
	clientTCPSocketSet = NULL;
	if (clientUDPSocketSet != NULL) SDLNet_FreeSocketSet(clientUDPSocketSet);
	clientUDPSocketSet = NULL;
	cout << "Client shut down" << endl;
}

bool NetworkService::restartClient() {
	closeClient();
	return startClient(clientAddress, clientPort);
}

bool NetworkService::connectToServer(bool useUDP) {
	clientTCPSockets[0] = SDLNet_TCP_Open(&clientIP);
	if (clientTCPSockets[0] != NULL) {
		SDLNet_TCP_AddSocket(clientTCPSocketSet, clientTCPSockets[0]);
		if (useUDP) {
			clientUDPSocket = SDLNet_UDP_Open(0);
			if (clientUDPSocket != NULL) {
				SDLNet_TCP_AddSocket(clientUDPSocketSet, clientUDPSocket);
				clientUDPAddresses[0] = new IPaddress;
				*clientUDPAddresses[0] = clientIP;
			} else {
				cout << "UDP socket could not be opened" << endl;
				return false;
			}
		}
	} else {
		cout << "TCP socket with server could not be opened" << endl;
		return false;
	}
	if (!useUDP) return true;
	do {
		if (sendIntUDP(123)) cout << "Sending message via UDP" << endl;
		recvStrTCP();
		if (keyPressed(27)) recvStrBuffer = "DISCONNECTED";
		if (recvStrBuffer == "DISCONNECTED") break;
	} while (recvStrBuffer != "CAPTURED");
	if (recvStrBuffer == "DISCONNECTED") {
		closeClient();
		return false;
	} else {
		int num, newID;
		do {
			cout << "Waiting for response from server..." << endl;
			num = recvIntUDP(&newID);
			if (keyPressed(27)) num = -1;
			if (num == -1) break;
		} while (!((num == 123) && (newID == -1)));
		if (recvStrBuffer != "DISCONNECTED") {
			do {
				cout << "Sending success message to server via TCP" << endl;
				if (keyPressed(27)) {
					closeClient();
					return false;
				}
			} while(!sendStrTCP("CONNECTED"));
			cout << "Waiting for response from server..." << endl;
			do {
				if (keyPressed(27)) {
					closeClient();
					return false;
				}
				cout << "Waiting for client ID allocation from server via TCP" << endl;
				clientID = recvIntTCP();
				cout << clientID << " received from server" << endl;
				if (clientID < 100) sendStrTCP("NEEDID");
			} while (!((clientID == -1) || (clientID > 99)));
			if (clientID > 99) {
				clientID -= 100;
				cout << "Connected to server" << endl << "ID for this client is " << clientID << endl;
				SDLNet_UDP_Bind(clientUDPSocket, clientID, clientUDPAddresses[0]);
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
	return clientID;
}

bool NetworkService::sendStrTCP(string data, int ID, bool isServer, int size) {
	if (size < 1) size = STR_SIZE;
	if (isServer) SDLNet_CheckSockets(serverTCPSocketSet, WAIT_TIME);
		else SDLNet_CheckSockets(clientTCPSocketSet, WAIT_TIME);
	if (ID >= 0) {
		TCPsocket sock = NULL;
		if (isServer) sock = serverTCPSockets[ID]; else sock = clientTCPSockets[0];
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

string NetworkService::recvStrTCP(int ID, bool isServer, int size) {
	string returnStr = "";
	if (size < 1) size = STR_SIZE;
	int i = -1;
	if (isServer) i = SDLNet_CheckSockets(serverTCPSocketSet, WAIT_TIME);
		else i = SDLNet_CheckSockets(clientTCPSocketSet, WAIT_TIME);
	if (i > -1) {
		TCPsocket sock = NULL;
		if (isServer) sock = serverTCPSockets[ID]; else sock = clientTCPSockets[0];
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
		if (sendIntTCP(ID+100, ID, true)) cout << "Sending ID to a client via TCP" << endl; else {
			do; while (recvStrTCP(ID, isServer, size) != "");
			if (sendIntTCP(ID+100, ID, true)) cout << "Sending ID to a client via TCP" << endl;
		}
		returnStr = "";
	}
	return returnStr;
}

bool NetworkService::sendIntTCP(int data, int ID, bool isServer) {
	if (isServer) SDLNet_CheckSockets(serverTCPSocketSet, WAIT_TIME);
		else SDLNet_CheckSockets(clientTCPSocketSet, WAIT_TIME);
	if (ID >= 0) {
		TCPsocket sock = NULL;
		if (isServer) sock = serverTCPSockets[ID]; else sock = clientTCPSockets[0];
		sendIntBuffer = data;
		if (sock != NULL) {
			if (!SDLNet_SocketReady((SDLNet_GenericSocket)sock)) {
				if (SDLNet_TCP_Send(sock, &sendIntBuffer, INT_SIZE) < INT_SIZE) return false;
			} else return false;
		} else return false;
	} else return false;
	return true;
}

int NetworkService::recvIntTCP(int ID, bool isServer) {
	int i = -1;
	if (isServer) i = SDLNet_CheckSockets(serverTCPSocketSet, WAIT_TIME);
		else i = SDLNet_CheckSockets(clientTCPSocketSet, WAIT_TIME);
	if (i > -1) {
		TCPsocket sock = NULL;
		if (isServer) sock = serverTCPSockets[ID]; else sock = clientTCPSockets[0];
		if (sock != NULL) {
			if (SDLNet_SocketReady((SDLNet_GenericSocket)sock)) {
				if (SDLNet_TCP_Recv(sock, &recvIntBuffer, INT_SIZE) < 1) return -1; else return recvIntBuffer;
			}
		}
	}
	return 0;
}

bool NetworkService::sendStrUDP(string data, int ID, bool isServer) {
	if (isServer) SDLNet_CheckSockets(serverUDPSocketSet, WAIT_TIME);
		else SDLNet_CheckSockets(clientUDPSocketSet, WAIT_TIME);
	if (ID >= 0) {
		UDPsocket sock = NULL;
		if (isServer) sock = serverUDPSocket; else sock = clientUDPSocket;
		if (!SDLNet_SocketReady((SDLNet_GenericSocket)sock)) {
			if (sendStrPacket != NULL) {
				SDLNet_FreePacket(sendStrPacket);
				sendStrPacket = NULL;
			}
			IPaddress * addr = NULL;
			if (isServer) addr = serverUDPAddresses[ID]; else addr = clientUDPAddresses[0];
			sendStrPacket = SDLNet_AllocPacket(data.size());
			sendStrPacket->address = *addr;
			sendStrPacket->data = (Uint8 *)data.c_str();
			sendStrPacket->len = data.size();
			sendStrPacket->maxlen = data.size();
			int sendID;
			if (isServer) sendID = -1; else sendID = clientID;
			if (SDLNet_UDP_Send(sock, sendID, sendStrPacket) == 0) return false;
		} else {
			recvStrUDP(isServer);
			return sendStrUDP(data, ID, isServer);
		}
	} else return false;
	return true;
}

string NetworkService::recvStrUDP(bool isServer) {
	int i = -1;
	string returnStr = "";
	if (isServer) i = SDLNet_CheckSockets(serverUDPSocketSet, WAIT_TIME);
		else i = SDLNet_CheckSockets(clientUDPSocketSet, WAIT_TIME);
	if (i > -1) {
		UDPsocket sock = NULL;
		if (isServer) sock = serverUDPSocket; else sock = clientUDPSocket;
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

string NetworkService::recvStrUDP(int * IDBuffer, bool isServer) {
	int i = -1;
	string returnStr = "";
	if (isServer) i = SDLNet_CheckSockets(serverUDPSocketSet, WAIT_TIME);
		else i = SDLNet_CheckSockets(clientUDPSocketSet, WAIT_TIME);
	if (i > -1) {
		UDPsocket sock = NULL;
		if (isServer) sock = serverUDPSocket; else sock = clientUDPSocket;
		if (SDLNet_SocketReady((SDLNet_GenericSocket)sock)) {
			if (recvStrPacket != NULL) {
				SDLNet_FreePacket(recvStrPacket);
				recvStrPacket = NULL;
			}
			recvStrPacket = SDLNet_AllocPacket(STR_SIZE);
			if (SDLNet_UDP_Recv(sock, recvStrPacket) == -1) {
				returnStr = "ERROR";
				*IDBuffer = -1;
			} else {
				for (i = 0; i < recvStrPacket->len; i++) returnStr += (char)recvStrPacket->data[i];
				*IDBuffer = recvStrPacket->channel;
			}
		} else return "";
	}
	return returnStr;
}

int NetworkService::recvStrUDP(string * stringBuffer, bool isServer) {
	int i = -1;
	*stringBuffer = "";
	if (isServer) i = SDLNet_CheckSockets(serverUDPSocketSet, WAIT_TIME);
		else i = SDLNet_CheckSockets(clientUDPSocketSet, WAIT_TIME);
	if (i > -1) {
		UDPsocket sock = NULL;
		if (isServer) sock = serverUDPSocket; else sock = clientUDPSocket;
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

bool NetworkService::sendIntUDP(int data, int ID, bool isServer) {
	if (isServer) SDLNet_CheckSockets(serverUDPSocketSet, WAIT_TIME);
		else SDLNet_CheckSockets(clientUDPSocketSet, WAIT_TIME);
	if (ID >= 0) {
		UDPsocket sock = NULL;
		if (isServer) sock = serverUDPSocket; else sock = clientUDPSocket;
		if (!SDLNet_SocketReady((SDLNet_GenericSocket)sock)) {
			if (sendIntPacket != NULL) {
				SDLNet_FreePacket(sendIntPacket);
				sendIntPacket = NULL;
			}
			IPaddress * addr = NULL;
			if (isServer) addr = serverUDPAddresses[ID]; else addr = clientUDPAddresses[0];
			sendIntPacket = SDLNet_AllocPacket(INT_SIZE);
			sendIntPacket->address = *addr;
			sendIntPacket->data[0] = data;
			sendIntPacket->len = INT_SIZE;
			sendIntPacket->maxlen = INT_SIZE;
			int sendID;
			if (isServer) sendID = -1; else sendID = clientID;
			if (SDLNet_UDP_Send(sock, sendID, sendIntPacket) == 0) return false;
		} else {
			recvIntUDP(isServer);
			return sendIntUDP(data, ID, isServer);
		}
	} else return false;
	return true;
}

int NetworkService::recvIntUDP(bool isServer) {
	int i = -1;
	if (isServer) i = SDLNet_CheckSockets(serverUDPSocketSet, WAIT_TIME);
		else i = SDLNet_CheckSockets(clientUDPSocketSet, WAIT_TIME);
	if (i > -1) {
		UDPsocket sock = NULL;
		if (isServer) sock = serverUDPSocket; else sock = clientUDPSocket;
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

int NetworkService::recvIntUDP(int * IDBuffer, bool isServer) {
	int i = -1;
	if (isServer) i = SDLNet_CheckSockets(serverUDPSocketSet, WAIT_TIME);
		else i = SDLNet_CheckSockets(clientUDPSocketSet, WAIT_TIME);
	if (i > -1) {
		UDPsocket sock = NULL;
		if (isServer) sock = serverUDPSocket; else sock = clientUDPSocket;
		if (SDLNet_SocketReady((SDLNet_GenericSocket)sock)) {
			if (recvIntPacket != NULL) {
				SDLNet_FreePacket(recvIntPacket);
				recvIntPacket = NULL;
			}
			recvIntPacket = SDLNet_AllocPacket(INT_SIZE);
			if (SDLNet_UDP_Recv(sock, recvIntPacket) == -1) {
				i = -1;
				*IDBuffer = -1;
			} else {
				i = recvIntPacket->data[0];
				*IDBuffer = recvIntPacket->channel;
			}
		} else return -1;
	}
	return i;
}

int NetworkService::recvIntUDPID(int * intBuffer, bool isServer) {
	int i = -1;
	if (isServer) i = SDLNet_CheckSockets(serverUDPSocketSet, WAIT_TIME);
		else i = SDLNet_CheckSockets(clientUDPSocketSet, WAIT_TIME);
	if (i > -1) {
		UDPsocket sock = NULL;
		if (isServer) sock = serverUDPSocket; else sock = clientUDPSocket;
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

NetworkService * networkService(string searchName) {
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

NetworkService * addNetworkService(string newName) {
	int letter = numCharInAlphabet(newName[0]);
	NetworkService * newNetworkService = new NetworkService(newName);
	allNetworkServices[letter].push_back(newNetworkService);
	return newNetworkService;
}

void destroyNetworkService(string searchName) {
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
