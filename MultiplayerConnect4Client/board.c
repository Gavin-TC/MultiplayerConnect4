#include "board.h"

int** getNewBoard() {
	int** board = (int**)malloc(sizeof(int*) * HEIGHT);
	if (board == NULL) {
		printf("[Error] allocating memory to board!\n");
		freeBoard(board);
		return NULL;
	}

	for (int y = 0; y < HEIGHT; y++) {
		board[y] = (int*)malloc(sizeof(int) * WIDTH);
		if (board[y] == NULL) {
			printf("[Error:2] allocating memory to board!\n");
			freeBoard(board);
			return NULL;
		}

		for (int x = 0; x < WIDTH; x++) {
			board[y][x] = 0;
		}
	}
	// Debug print to verify initialization
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			printf("board[%d][%d] = %d\n", y, x, board[y][x]);
		}
	}

	return board;
}

int placePiece(int** board, int spot, int playerID) {
	if (!spot) {
		return 1;
	}

	if (spot < 1 || spot > 7) {
		printf("That spot (%i) is out of bounds!\n", spot);
		return 1;
	}
	spot -= 1;

	if (board[0][spot] != 0) {
		printf("That spot (%i) is taken!\n", spot);
		printf("Piece at spot: %d\n", board[0][spot]);
		return 1;
	}


	for (int y = 0; y < HEIGHT; y++) {
		if (board[y][spot] != 0) {
			// No need to check if its OOB because of previous check
			board[y - 1][spot] = playerID;
			return 0;
		}
		else if (y == HEIGHT - 1) {
			board[y][spot] = playerID;
			return 0;
		}
	}
	printf("Error placing piece at spot %i!\n", spot);
	return 1;
}

// Returns 0 if no winners, 1 if there's a winner 
// Function to check for a win
int checkWin(int** board, int playerID) {
	// Iterate over the board
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			printf("num: %d\n", board[y][x]);
			// Check horizontal (to the right)
			if (x <= WIDTH - 4) {
				if (board[y][x] == playerID &&
					board[y][x + 1] == playerID &&
					board[y][x + 2] == playerID &&
					board[y][x + 3] == playerID) {
					return 1;
				}
			}

			// Check vertical (downwards)
			if (y <= HEIGHT - 4) {
				if (board[y][x] == playerID &&
					board[y + 1][x] == playerID &&
					board[y + 2][x] == playerID &&
					board[y + 3][x] == playerID) {
					return 1;
				}
			}

			// Check diagonal (\) (down-right)
			if (y <= HEIGHT - 4 && x <= WIDTH - 4) {
				if (board[y][x] == playerID &&
					board[y + 1][x + 1] == playerID &&
					board[y + 2][x + 2] == playerID &&
					board[y + 3][x + 3] == playerID) {
					return 1;
				}
			}

			// Check diagonal (/) (up-right)
			if (y >= 4 - 1 && x <= WIDTH - 4) {
				if (board[y][x] == playerID &&
					board[y - 1][x + 1] == playerID &&
					board[y - 2][x + 2] == playerID &&
					board[y - 3][x + 3] == playerID) {
					return 1;
				}
			}
		}
	}

	// If no win found, return 0
	printf("No wins found!\n");
	return 0;
}

void printBoard(int** board) {
	printf("1 2 3 4 5 6 7\n");
	printf("-------------\n");
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			switch (board[y][x]) {
			case 0: // Spot is empty
				printf("O ");
				break;

			case 1: // Client 1's
				printf("%c ", PLAYER1_CHAR);
				break;

			case 2: // Client 2's
				printf("%c ", PLAYER2_CHAR);
				break;
			}
		}
		printf("\n");
	}
}

void freeBoard(int** board) {
	for (int y = 0; y < HEIGHT; y++) {
		free(board[y]);
	}
	free(board);
}
