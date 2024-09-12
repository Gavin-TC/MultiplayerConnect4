#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <WinSock2.h>

#include "../MultiplayerConnect4Client/board.h"

#pragma comment(lib, "Ws2_32.lib")

int initializeServer(SOCKET* serverSocket, struct sockaddr_in* serverAddr);
int handleClient(SOCKET* clientSocket, int spot);
SOCKET acceptClient(SOCKET serverSocket, const char* clientName);
void serverCleanup(SOCKET* serverSocket, SOCKET* clientSocket, SOCKET* client2Socket);
int sendMessage(char* message, SOCKET* clientSocket, const char* clientName);

int main(void) {
	WSADATA wsaData;
	SOCKET serverSocket;
	SOCKET clientSocket, client2Socket;
	struct sockaddr_in serverAddr;

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(6006);

	if (initializeServer(&serverSocket, &serverAddr) != 0) {
		return 1;
	}

	clientSocket = acceptClient(serverSocket, "Client 1");
	if (clientSocket == INVALID_SOCKET) {
		printf("Error accepting client 1: %d\n", WSAGetLastError());
		serverCleanup(&serverSocket, &clientSocket, &client2Socket);
		return 1;
	}
	printf("Client 1 connected!\n");

	client2Socket = acceptClient(serverSocket, "Client 2");
	if (client2Socket == INVALID_SOCKET) {
		printf("Error accepting client 2: %d\n", WSAGetLastError());
		serverCleanup(&serverSocket, &clientSocket, &client2Socket);
		return 1;
	}
	printf("Client 2 connected!\n");

	enum GAMESTATE {
		TURN_1,				// Client 1's turn
		TURN_2,				// Client 2's turn
		END_GAME,			// End game state
		GAME_STATE_COUNT	// Number of game states
	};
	int gameState = TURN_1;
	bool serverRunning = true;
	int wonPlayerID = 0;

	printf("Sending start message to clients...\n");

	sendMessage("1", clientSocket, "Client 1");
	sendMessage("2",  client2Socket, "Client 2");

	serverRunning = !sendMessage("start", clientSocket, "Client 1");
	serverRunning = !sendMessage("start", client2Socket, "Client 2");

	int** board = NULL;
	if (serverRunning) {
		board = getNewBoard();
		if (board == NULL) {
			printf("Error creating board! Shutting down server!\n");
			serverRunning = false;
			return 1;
		}
		else {
			printf("Board creation successful!\n");
		}
	}

	while (serverRunning) {
		int spot = 0; // Players chosen spot (1-7)

		switch (gameState) {
			case TURN_1:
				if (clientSocket == INVALID_SOCKET) {
					serverRunning = false;
					break;
				}
				sendMessage("yourTurn", clientSocket, "Client 1");

				printf("Client 1's turn...\n");
				spot = handleClient(clientSocket, spot);
				printf("Spot now: %d\n", spot);

				// Place piece on server side board
				placePiece(board, spot, 1);
				gameState = TURN_2;
				break;

			case TURN_2:
				if (client2Socket == INVALID_SOCKET) {
					serverRunning = false;
					break;
				}
				sendMessage("yourTurn", client2Socket, "Client 2");

				printf("Client 2's turn...\n");
				spot = handleClient(client2Socket, spot);
				printf("Spot now: %d\n", spot);

				// Place piece on server side board
				placePiece(board, spot, 2);
				gameState = TURN_1;
				break;

			case END_GAME:
				printf("Game ended!");
				serverRunning = false;
				break;
		}
		printBoard(board);
		if (wonPlayerID == 0) {
			if (checkWin(board, 1) == 1) {
				printf("Player 1 won!\n");
				gameState = END_GAME;
			} else if (checkWin(board, 2) == 1) {
				printf("Player 2 won!\n");
				gameState = END_GAME;
			}
		}
	}
	if (board) {
		freeBoard(board);
	}
	serverCleanup(&serverSocket, &clientSocket, &client2Socket);

	system("PAUSE");

	return 0;
}

int initializeServer(SOCKET* serverSocket, struct sockaddr_in* serverAddr) {
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("WSAStartup failed: %d\n", WSAGetLastError());
		return 1;
	}

	*serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (*serverSocket == INVALID_SOCKET) {
		printf("Error binding server socket: %d\n", WSAGetLastError());
		closesocket(*serverSocket);
		WSACleanup();
		return 1;
	}

	printf("Binding server socket...\n");
	if (bind(*serverSocket, (struct sockaddr*)serverAddr, sizeof(*serverAddr)) != 0) {
		printf("Error binding server socket: %d\n", WSAGetLastError());
		closesocket(*serverSocket);
		WSACleanup();
		return 1;
	}

	printf("Server socket listening on port 6006...\n");
	if (listen(*serverSocket, 5) == SOCKET_ERROR) {
		printf("Error listening: %d\n", WSAGetLastError());
		closesocket(*serverSocket);
		WSACleanup();
		return 1;
	}
	printf("Server successfully initialized!\n");
	return 0;
}

int handleClient(SOCKET* clientSocket, int spot) {
	char buffer[1024];
	printf("Receiving from client...\n");
	int result = recv(clientSocket, buffer, sizeof(buffer), 0);

	if (result == 0 || clientSocket == INVALID_SOCKET) {
		printf("Client disconnected...\n\n");
		closesocket(clientSocket);
		clientSocket = INVALID_SOCKET;
		return -1;
	}
	else if (result == SOCKET_ERROR) {
		printf("Error receiving data: %d\n\n", WSAGetLastError());
		closesocket(clientSocket);
		clientSocket = INVALID_SOCKET;
		return -1;
	}
	else {
		buffer[result] = '\0';
		printf("Received data: %s\n\n", buffer);
		return buffer[0] - '0';
	}
	return -1;
}

SOCKET acceptClient(SOCKET serverSocket, const char* clientName) {
	printf("Waiting for %s...\n", clientName);
	SOCKET clientSocket = accept(serverSocket, NULL, NULL);
	if (clientSocket == INVALID_SOCKET) {
		printf("Error accepting %s: %d\n\n", clientName, WSAGetLastError());
	}
	else {
		printf("%s connected!\n\n", clientName);
	}
	return clientSocket;
}

void serverCleanup(SOCKET* serverSocket, SOCKET* clientSocket, SOCKET* client2Socket) {
	if (*clientSocket != INVALID_SOCKET)  closesocket(*clientSocket);
	if (*client2Socket != INVALID_SOCKET) closesocket(*client2Socket);
	if (*serverSocket != INVALID_SOCKET)  closesocket(*serverSocket);
	WSACleanup();
}

int sendMessage(char* message, SOCKET* clientSocket, const char* clientName) {
	printf("Sending message [%s] to %s...\n", message, clientName);
	if (send(clientSocket, message, strlen(message), 0) == SOCKET_ERROR) {
		printf("Error sending message [%s] to %s: %d\n", message, clientName, WSAGetLastError());
		return 1;
	}
	printf("Successfully sent message [%s] to %s!\n\n", message, clientName);
	return 0;
}
