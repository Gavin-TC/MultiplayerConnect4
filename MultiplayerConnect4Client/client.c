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
	printf("\n\n");

	int playerID = atoi(receiveAnyMessage(&clientSocket)); // Player number given by server.
	printf("You are player %i!\n", playerID);

	bool clientRunning = !receiveSpecificMessage(&clientSocket, "start");

	int** board = getNewBoard();
	if (board == NULL) {
		printf("[ERROR] Error creating board! Client closing...\n");
		clientRunning = false;
	}

	bool isMyTurn = false;
	bool isFirstTurn = true;

	while (clientRunning) {
		printBoard(board);
		if (isMyTurn) {
			int spot = NULL;
			char spotstr[32];
			
			while (spot == NULL || placePiece(board, spot, playerID)) {
				spot = -1;
				printf("\n\n");

				printf("Your turn!\n");
				printf("Pick a column: ");
				scanf("%i", &spot);

				itoa(spot, spotstr, 10); // Convert spot to a str to send off to the server
			}

			isMyTurn = false;
			isFirstTurn = false;
			
			if (send(clientSocket, spotstr, strlen(spotstr), 0) == SOCKET_ERROR) {
				printf("[ERROR] Error when trying to send spot (%s): %d\n", spotstr, WSAGetLastError());
			}
		} else {
			if (!isFirstTurn) {
				char* message = receiveAnyMessage(&clientSocket);
				int opponentSpot = atoi(message);
				printf("[DEBUG] Received opponents turn!\n");
				if (playerID == 1) {
					placePiece(board, opponentSpot, 2);
					printf("[DEBUG] Opponent placed piece at %d.\n", opponentSpot);
				} else {
					placePiece(board, opponentSpot, 1);
					printf("[DEBUG] Opponent placed piece at %d.\n", opponentSpot);
				}
			}

			printf("Waiting for opponent to make turn...\n");
			printf("[DEBUG] Waiting for my turn...\n");
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

	// Initialize WinSock2
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("[ERROR] WSAStartup failed: %d\n", WSAGetLastError());
		return 1;
	}

	*clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (*clientSocket == INVALID_SOCKET) {
		printf("[ERROR] Error binding client socket: %d\n", WSAGetLastError());
		closesocket(*clientSocket);
		WSACleanup();
		return 1;
	}

	if (connect(*clientSocket, (struct sockaddr*)serverAddr, sizeof(*serverAddr)) != 0) {
		printf("[ERROR] Error connecting to server: %d\n", WSAGetLastError());
		closesocket(*clientSocket);
		WSACleanup();
		return 1;
	}

	printf("[DEBUG] Successfully connected to the server!\n");
	return 0;
}

int receiveSpecificMessage(SOCKET* clientSocket, char* message) {
	printf("[DEBUG] Attempting to receive message [%s]...\n", message);

	char buffer[1024];
	int messageBytes = recv(*clientSocket, buffer, sizeof(buffer), 0);
	if (messageBytes <= 0) {
		printf("[ERROR] Error receiving message [%s]!: %d\n", message, WSAGetLastError());
		return 1;
	}

	buffer[messageBytes] = '\0';

	if (strcmp(buffer, message) == 0) {
		printf("[DEBUG] Successfully received message [%s]!\n", message);
		return 0;
	}
	printf("[DEBUG] Received message [%s] did not match expected message [%s]!: %d\n", buffer, message, WSAGetLastError());
	return 1;
}

char* receiveAnyMessage(SOCKET* clientSocket) {
	printf("[DEBUG] Listening for any message...\n");

	char buffer[1024];
	int messageBytes = recv(*clientSocket, buffer, sizeof(buffer), 0);
	buffer[messageBytes] = '\0';

	printf("[DEBUG] Received message [%s]!\n", buffer);

	return buffer;
}
