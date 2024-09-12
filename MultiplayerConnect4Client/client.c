#include "board.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <WinSock2.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable : 4996)

int connectToServer(SOCKET* clientSocket, struct sockaddr_in* serverAddr);
int receiveSpecificMessage(SOCKET* clientSocket, char* message);
char* receiveAnyMessage(SOCKET* clientSocket);

int main(void) {
	SOCKET clientSocket;
	struct sockaddr_in serverAddr;
	char address[64];

	printf("Enter server address: ");
	if (scanf("%63s", &address) != 1) {
		printf("Invalid address!\n");
		return 1;
	}
	address[strlen(address)] = '\0';
	printf("Connecting to server at %s...\n", address);

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(address);
	serverAddr.sin_port = htons(6006);

	if (connectToServer(&clientSocket, &serverAddr) != 0) {
		closesocket(clientSocket);
		return 1;
	}

	int playerID = atoi(receiveAnyMessage(&clientSocket)); // Player number given by server.
	printf("You are player %i!\n", playerID);
	bool clientRunning = !receiveSpecificMessage(&clientSocket, "start");
	int** board = getNewBoard();
	if (board == NULL) {
		printf("Error creating board! Client closing...\n");
		clientRunning = false;
	}
	bool isMyTurn = false;

	while (clientRunning) {
		if (isMyTurn) {
			int spot = NULL;
			char spotstr[32];
			
			while (spot == NULL || placePiece(board, spot, playerID)) {
				spot = -1;
				printf("Your turn!\n");
				printBoard(board);

				printf("Pick a column: ");
				scanf("%i", &spot);
				printf("\nYou picked spot %i\n", spot);
				itoa(spot, spotstr, 10);
			}

			isMyTurn = false;
			printf("Sending spot %s...\n", spotstr);
			printf("error: %d\n", send(clientSocket, spotstr, strlen(spotstr), 0));
		}
		else {
			if (receiveSpecificMessage(&clientSocket, "yourTurn") == 0) {
				isMyTurn = true;
			}
		}
	}
	system("PAUSE");
	closesocket(clientSocket);

	return 0;
}

int connectToServer(SOCKET* clientSocket, struct sockaddr_in* serverAddr) {
	WSADATA wsaData;

	// Initialize Winsock
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("WSAStartup failed: %d\n", WSAGetLastError());
		return 1;
	}

	*clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (*clientSocket == INVALID_SOCKET) {
		printf("Error binding client socket: %d\n", WSAGetLastError());
		closesocket(*clientSocket);
		WSACleanup();
		return 1;
	}

	if (connect(*clientSocket, (struct sockaddr*)serverAddr, sizeof(*serverAddr)) != 0) {
		printf("Error connecting to server: %d\n", WSAGetLastError());
		closesocket(*clientSocket);
		WSACleanup();
		return 1;
	}

	printf("Successfully connected to the server!\n");
	return 0;
}

int receiveSpecificMessage(SOCKET* clientSocket, char* message) {
	printf("Attempting to receive message [%s]...\n", message);

	char buffer[1024];
	int messageBytes = recv(*clientSocket, buffer, sizeof(buffer), 0);
	if (messageBytes <= 0) {
		printf("Error receiving message [%s]!: %d\n", message, WSAGetLastError());
		return 1;
	}

	buffer[messageBytes] = '\0';

	if (strcmp(buffer, message) == 0) {
		printf("Successfully received message [%s]!\n", message);
		return 0;
	}
	printf("Received message [%s] did not match expected message [%s]!: %d\n", buffer, message, WSAGetLastError());
	return 1;
}

char* receiveAnyMessage(SOCKET* clientSocket) {
	printf("Listening for any message...\n");

	char buffer[1024];
	int messageBytes = recv(*clientSocket, buffer, sizeof(buffer), 0);
	buffer[messageBytes] = '\0';

	printf("Successfully received message [%s]!\n", buffer);

	return buffer;
}
