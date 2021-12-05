#include "game.h"

Game* gameInit(int player1, int player2)
{
    Game *newGame = (Game*)malloc( sizeof(Game) );
    for(int i=0;i<9;i++)
        newGame->num[i] = 0;
    newGame->player1 = player1;
    newGame->player2 = player2;
    return newGame;
}

int winner(Game *game)
{
    if (game->num[0]==game->num[1] && game->num[1]==game->num[2])
        return game->num[0];
    if (game->num[3]==game->num[4] && game->num[4]==game->num[5])
        return game->num[3];
    if (game->num[6]==game->num[7] && game->num[7]==game->num[8])
        return game->num[6];
    if (game->num[0]==game->num[3] && game->num[3]==game->num[6])
        return game->num[0];
    if (game->num[1]==game->num[4] && game->num[4]==game->num[7])
        return game->num[1];
    if (game->num[2]==game->num[5] && game->num[5]==game->num[8])
        return game->num[2];
    if (game->num[0]==game->num[4] && game->num[4]==game->num[8])
        return game->num[0];
    if (game->num[2]==game->num[4] && game->num[4]==game->num[6])
        return game->num[2];
    return 0;
}

int winPb(int a, int b, int c)
{
    if (a == 1)
        if (b==-1 || c==-1)
            return 0;
    if (b == 1)
        if (a==-1 || c==-1)
            return 0;
    if (c == 1)
        if (a==-1 || b==-1)
            return 0;
    
    if (a == -1)
        if (b==1 || c==1)
            return 0;
    if (b == -1)
        if (a==1 || c==1)
            return 0;
    if (c == -1)
        if (a==1 || b==1)
            return 0;

    return 1;
}

int isGameEnd(Game *game)
{
    // 有輸贏的情況
    if (game->num[0]==game->num[1] && game->num[1]==game->num[2] && game->num[0]!=0)
        return 1;
    if (game->num[3]==game->num[4] && game->num[4]==game->num[5] && game->num[3]!=0)
        return 1;
    if (game->num[6]==game->num[7] && game->num[7]==game->num[8] && game->num[6]!=0)
        return 1;
    if (game->num[0]==game->num[3] && game->num[3]==game->num[6] && game->num[0]!=0)
        return 1;
    if (game->num[1]==game->num[4] && game->num[4]==game->num[7] && game->num[1]!=0)
        return 1;
    if (game->num[2]==game->num[5] && game->num[5]==game->num[8] && game->num[2]!=0)
        return 1;
    if (game->num[0]==game->num[4] && game->num[4]==game->num[8] && game->num[0]!=0)
        return 1;
    if (game->num[2]==game->num[4] && game->num[4]==game->num[6] && game->num[2]!=0)
        return 1;

    // 還沒人贏但還有可能贏
    if ( winPb(game->num[0], game->num[1], game->num[2]) )
        return 0;
    if ( winPb(game->num[3], game->num[4], game->num[5]) )
        return 0;
    if ( winPb(game->num[6], game->num[7], game->num[8]) )
        return 0;
    if ( winPb(game->num[0], game->num[3], game->num[6]) )
        return 0;
    if ( winPb(game->num[1], game->num[4], game->num[7]) )
        return 0;
    if ( winPb(game->num[2], game->num[5], game->num[8]) )
        return 0;
    if ( winPb(game->num[0], game->num[4], game->num[8]) )
        return 0;
    if ( winPb(game->num[2], game->num[4], game->num[6]) )
        return 0;

    // 平手
    return 1;
}