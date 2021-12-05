#include<stdlib.h>
// 遊戲
struct game {
    int num[9];
    int player1, player2;   // ID
    // player1: O, 1
    // player2: X, -1
};
typedef struct game Game;

Game* gameInit(int player1, int player2);
int winner(Game *game);         // player1 or player2
int isGameEnd(Game *game);      // if the game is end
int winPb(int a, int b, int c); // Assist "isGameEnd"