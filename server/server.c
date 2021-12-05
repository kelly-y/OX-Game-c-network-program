#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/select.h>
#include "game.h"

#define PORT "8080"
char Buffer[4097];  // Buffer of "recv"
char buffer[1025];  // Buffer of "send"

fd_set master, readFds;
int sockfd, fdMax;

// 使用者資訊
char Account[5][30] = {"apple", "minion", "kabi", "penguin", "pika"};
char Passwd[5][30] = {"apple", "minion", "kabi", "penguin", "pika"};
int isOnline[5] = {0};
int isPlaying[5] = {0};
int pair[5] = {-1, -1, -1, -1, -1};

// 遊戲
Game *game1, *game2;

int findUsr(int client)
{
    for(int i=0;i<5;i++)
        if (pair[i] == client)
            return i;
    return -1;
}

void logIn(int client)
{
    char *start = strstr(Buffer, "\n") + 1;
    char *end = strstr(start, "\n");
    *end = '\0';
    char ground[70], data[70];
    int correct = -1;
    for(int i=0;i<5;i++)
    {
        sprintf(ground, "%s:%s", Account[i], Passwd[i]);
        sprintf(data, "%s", start);
        if ( strcmp(ground, data) == 0 ) {
            isOnline[i] = 1;
            pair[i] = client;
            sprintf(buffer, "Login Successfully: \n");
            send(client, buffer, strlen(buffer), 0);
            return;
        }
    }
    sprintf(buffer, "Login Failed: \n");
    send(client, buffer, strlen(buffer), 0);
    return;
}

void showList(int client)
{
    char msg[40] = "";
    for(int i=0;i<5;i++)
    {
        if (isOnline[i] && !isPlaying[i] && pair[i]!=client)
            strcat(strcat(msg, Account[i]), ";");
    }
    sprintf(buffer, "List: \n%s\n", msg);
    send(client, buffer, strlen(buffer), 0);
    return;
}

void invite(int client)
{
    int oppoID, ID = findUsr(client);
    char *start = strstr(Buffer, "\n") + 1;
    char *end = strstr(start, "\n");
    char name[30];
    strncpy(name, start, end-start);
    name[end-start] = '\0';

    oppoID = -1;
    for(int i=0;i<5;i++)
    {
        if ( strncmp(name, Account[i], end-start) == 0 ) {
            oppoID = i;
            break;
        }
    }

    if (oppoID == -1)
    {
        sprintf(buffer, "Invite Result: Failed, %s isn't exist.\n", name);
        send(client, buffer, strlen(buffer), 0);
    }
    else if ( !isOnline[oppoID] )
    {
        sprintf(buffer, "Invite Result: Failed, %s isn't online.\n", name);
        send(client, buffer, strlen(buffer), 0);
    }
    else if ( isPlaying[oppoID] )
    {
        sprintf(buffer, "Invite Result: Failed, %s is in another game.\n", name);
        send(client, buffer, strlen(buffer), 0);
    }
    else
    {
        sprintf(buffer, "Invite Result: Success, wait for %s's reply.\n", name);
        send(client, buffer, strlen(buffer), 0);
        sprintf(buffer, "Invite Msg: Have a game with %s?\n", Account[ID]);
        send(pair[oppoID], buffer, strlen(buffer), 0);
    }

    return;
}

void startGame(int client)  // Invite accept
{
    int oppoID=-1, ID=findUsr(client);
    char oppo[30];
    char *start = Buffer + 15;
    char *end = strstr(Buffer, "\n");
    strncpy(oppo, start, end-start);
    for(int j=0;j<5;j++)
        if ( strncmp(Account[j], oppo, end-start) == 0 )
            oppoID = j;

    isPlaying[oppoID] = 1;
    isPlaying[ID] = 1;
    Game *newGame = gameInit(oppoID, ID);
    if ( !game1 )
        game1 = newGame;
    else    // !game2
        game2 = newGame;

    sprintf(buffer, "Start game: you are player1 'O'\n");
    send(pair[newGame->player1], buffer, strlen(buffer), 0);
    sprintf(buffer, "Start game: you are player2 'X'\n");
    send(pair[newGame->player2], buffer, strlen(buffer), 0);

    return;
}

void gameIng(int client)
{
    int oppoID=-1, ID=findUsr(client), sign=0;
    Game *curr;
    if (game1 && game1->player1==ID)
    {
        sign = 1;
        curr = game1;
        oppoID = game1->player2;
    }
    else if (game1 && game1->player2==ID)
    {
        sign = -1;
        curr = game1;
        oppoID = game1->player1;
    }
    else if (game2 && game2->player1==ID)
    {
        sign = 1;
        curr = game2;
        oppoID = game2->player2;
    }
    else if (game2 && game2->player2==ID)
    {
        sign = -1;
        curr = game2;
        oppoID = game2->player1;
    }
    else return;

    char loc = Buffer[4];
    char nloc = loc - 48;
    curr->num[nloc] = sign;
    sprintf(buffer, "On: %d\n", nloc);
    send(pair[oppoID], buffer, strlen(buffer), 0);

    if ( isGameEnd(curr) )
    {
        int win = winner(curr);
        if (win == 1)
        {
            sprintf(buffer, "Game end: win\n");
            send(pair[curr->player1], buffer, strlen(buffer), 0);
            sprintf(buffer, "Game end: lose\n");
            send(pair[curr->player2], buffer, strlen(buffer), 0);
        }
        else if (win == 0)
        {
            sprintf(buffer, "Game end: tie\n");
            send(pair[ID], buffer, strlen(buffer), 0);
            send(pair[oppoID], buffer, strlen(buffer), 0);
        }
        else
        {
            sprintf(buffer, "Game end: win\n");
            send(pair[curr->player2], buffer, strlen(buffer), 0);
            sprintf(buffer, "Game end: lose\n");
            send(pair[curr->player1], buffer, strlen(buffer), 0);
        }

        isPlaying[ID] = 0;
        isPlaying[oppoID] = 0;
        if (curr == game1)
            game1 = NULL;
        else if (curr == game2)
            game2 = NULL;
        free(curr);
        
    }

    return;
}

int main(int argc, char *argv[])
{
    // 取得 addr info
    struct addrinfo info, *addr;
    memset(&info, 0, sizeof(info));
    info.ai_family = AF_INET;
    info.ai_socktype = SOCK_STREAM;
    info.ai_flags = AI_PASSIVE;
    int rtAddr = getaddrinfo(NULL, PORT, &info, &addr), i;
    if (rtAddr != 0)
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rtAddr));

    // 建立一個 socket
    sockfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if (sockfd == -1)
        perror("server socket");

    // 允許重新使用 port
    int yes = 1;
    if ( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1 )
        perror("setsockopt");

    // Bind
    int rtBd = bind(sockfd, addr->ai_addr, addr->ai_addrlen);
    if (rtBd == -1)
        perror("server bind");

    // Listen
    int rtLsn = listen(sockfd, 10);
    if (rtLsn == -1)
        perror("listen");

    FD_ZERO(&master);
    FD_ZERO(&readFds);
    FD_SET(sockfd, &master);
    fdMax = sockfd;
    while(1)
    {
        readFds = master;
        if ( select(fdMax+1, &readFds, NULL, NULL, NULL) == -1 )
            perror("select");

        for (i=0;i<=fdMax;i++)
        {
            if ( FD_ISSET(i, &readFds) )
            {
                if (i == sockfd)    // 有新連線
                {
                    socklen_t sockLen;
                    struct sockaddr client;
                    int newfd = accept(sockfd, &client, &sockLen);
                    if (newfd == -1)
                        perror("accept");

                    FD_SET(newfd, &master);
                    if (newfd > fdMax)
                        fdMax = newfd;
                }
                else    // 有 client 傳資料來，等待被讀取
                {
                    memset(Buffer, 0, 4096);
                    int length = recv(i, Buffer, 4096, 0);
                    if (length == 0)
                    {
                        // 清除 pair
                        int tmp=-1;
                        for(int j=0;j<5;j++)
                            if (pair[j] == i) {
                                tmp = j;
                                pair[j] = -1;
                            }
                        
                        isOnline[tmp] = 0;
                        printf("Connection ended by client %d\n", i);
                        FD_CLR(i, &master);
                    }
                    else if (length == -1)
                        perror("recv");

                    if ( strncmp("Login: ", Buffer, 7) == 0 )
                        logIn(i);
                    else if ( strncmp("List: ", Buffer, 6) == 0 )
                        showList(i);
                    else if ( strncmp("Invite: ", Buffer, 8) == 0 )
                        invite(i);
                    else if ( strncmp("Invite accept: ", Buffer, 15) == 0 )
                        startGame(i);
                    else if ( strncmp("Go: ", Buffer, 4) == 0 )
                        gameIng(i);
                }
            }
        }
    }

    return 0;
}