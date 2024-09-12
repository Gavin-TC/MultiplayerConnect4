#ifndef BOARD_H
#define BOARD_H

#include <stdio.h>
#include <stdlib.h>

#define WIDTH 7
#define HEIGHT 6

#define PLAYER1_CHAR '@'
#define PLAYER2_CHAR 'X'

int** getNewBoard();
int placePiece(int** board, int spot, int player);
int checkWin(int** board, int playerID);
void printBoard(int** board);
void freeBoard(int** board);

#endif
