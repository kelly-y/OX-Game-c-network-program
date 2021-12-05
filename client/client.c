#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>

#define PORT "8080"
char Buffer[4097];  // Buffer of "recv"
char buffer[1025];  // Buffer of "send"

fd_set master, readFds;
int sockfd, fdMax, myNum, state[9];

void division()
{
    printf("---------------------------------------------\n\n");
    return;
}

void logIn()
{
    char account[30], password[30];
    printf("Your account: \n");
    scanf("%s", account);
    printf("Your password: \n");
    scanf("%s", password);
    sprintf(buffer, "Login: \n%s:%s\n", account, password);
    send(sockfd, buffer, strlen(buffer), 0);
    printf("\n");
    return;
}

void showList()
{
    char list[5][30], oppo[30];
    char *line = strstr(Buffer, "\n") + 1;
    char *line2 = strstr(line, ";");
    int cnt=0, i;

    while( line2 )
    {
        strncpy(list[cnt], line, line2-line);
        list[cnt++][line2-line] = '\0';
        line = line2 + 1;
        line2 = strstr(line, ";");
    }

    printf("Here are online players. Enter \"Invite [xxx]\" to invite opponent.\n=> ");
    for (i=0;i<cnt-1;i++)
        printf("%s, ", list[i]);
    if (cnt > 0)
        printf("%s", list[i]);
    printf("\n\n");

    return;
}

void invite()
{
    division();
    char *start = strstr(Buffer, " ") + 1;
    char *end = strstr(Buffer, "\n");
    char name[30];
    strncpy(name, start, end-start);
    name[end-start] = '\0';

    sprintf(buffer, "Invite: \n%s\n", name);
    send(sockfd, buffer, strlen(buffer), 0);

    // 收 Invite 的回覆訊息
    memset(Buffer, 0, 4096);
    int length = recv(sockfd, Buffer, 4096, 0);
    if (length==0 || length==-1)
        exit(3);
    start = Buffer + 15;
    printf("%s\n", start);

    return;
}

void gotInvite()
{
    division();
    char *start = Buffer + 12;
    char *end = strstr(Buffer, "\n") + 1;
    char msg[100];
    strncpy(msg, start, end-start);
    msg[end-start] = '\0';
    printf("Here comes a invitation!!\n%s\n", msg);
    printf("Enter \"Accept [xxx]\" to accept the invitation\n");
    return;
}

void acceptInvite()
{
    char msg[100] = "Invite accept: ";
    char *start = Buffer + 7;
    char *end = strstr(Buffer, "\n");
    strncat(msg, start, end-start);
    sprintf(buffer, "%s\n", msg);
    send(sockfd, buffer, strlen(buffer), 0);
    return;
}

void graphic()
{
    for(int i=0;i<3;i++)
    {
        char a, b, c;
        if (state[3*i] == 0) a = ' ';
        else if (state[3*i] == 1) a = 'O';
        else if (state[3*i] == -1) a = 'X';
        if (state[3*i+1] == 0) b = ' ';
        else if (state[3*i+1] == 1) b = 'O';
        else if (state[3*i+1] == -1) b = 'X';
        if (state[3*i+2] == 0) c = ' ';
        else if (state[3*i+2] == 1) c = 'O';
        else if (state[3*i+2] == -1) c = 'X';

        printf(" %c │", a);
        printf(" %c │", b);
        printf(" %c \n", c);
        if (i != 2) printf("———+———+———\n");
    }
    return;
}

void startGame()
{
    division();
    char *start = Buffer + 12;
    char *end = strstr(Buffer, "\n");
    char num = *(end-5);
    *end = '\0';
    char inst[100];
    strcpy(inst, start);
    int number = num - 48;
    if (number == 1)
    {
        myNum = 1;
        strcat(inst, ", you go first. Enter \"Go [0-8]\"");
    }
    else
    {
        myNum = -1;
        strcat(inst, ", wait for your opponent.");
    }
    printf("%s\n", inst);

    for(int i=0;i<9;i++)
        state[i] = 0;

    if (number == 1)
    {
        char inst[50];
        fgets(inst, 49, stdin);
        while( strncmp(inst, "Go ", 3) != 0 || !isdigit(inst[3]))
            fgets(inst, 49, stdin);
        sprintf(buffer, "Go: %d\n", inst[3]-48);
        send(sockfd, buffer, strlen(buffer), 0);
        state[inst[3]-48] = myNum;
        graphic();
    }

    return;
}

void loc()
{
    int location = Buffer[3] - 48;
    state[location] = myNum;
    sprintf(buffer, "Go: %d\n", location);
    send(sockfd, buffer, strlen(buffer), 0);
    graphic();
    return;
}

void gameEnd()
{
    char *start = Buffer + 10;

    if ( strncmp("win", start, 3) == 0 )
        printf("Congratulations!! You win!!!\n");
    else if ( strncmp("lose", start, 4) == 0 )
        printf("Oops... You lose the game.\n");
    else
        printf("Tie result.\n");
    printf("\nGame Over. Enter your instruction: (Ex: List)\n");
    division();
    return;
}

void locOn()
{
    printf("\n\n\n");
    int location = Buffer[4] - 48;
    if (myNum == 1)
        state[location] = -1;
    else
        state[location] = 1;
    graphic();
    printf("Your opponent put on %d, it's your turn. Enter \"Go [0-8]\"\n", location);

    char *line = strstr(Buffer, "\n") + 1;
    if ( strncmp(line, "Game end: ", 10) == 0 )
    {
        char *end = strstr(line, "\n");
        strncpy(Buffer, line, end-line);
        gameEnd();
    }
    return;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Please assign server IP\n");
        exit(3);
    }

    // 取得 addr info
    struct addrinfo info, *addr;
    memset(&info, 0, sizeof(info));
    info.ai_family = AF_INET;
    info.ai_socktype = SOCK_STREAM;
    int rtAddr = getaddrinfo(argv[1], PORT, &info, &addr);
    if (rtAddr != 0)
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rtAddr));

    // 建立一個 socket
    sockfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if (sockfd == -1)
        perror("server socket");

    // 連線
    int rtCont = connect(sockfd, addr->ai_addr, addr->ai_addrlen);
    if (rtCont == -1)
        perror("client: connect");

    FD_ZERO(&master);
    FD_ZERO(&readFds);
    FD_SET(fileno(stdin), &master);
    FD_SET(sockfd, &master);
    fdMax = sockfd;
    division();
    logIn();
    while(1)
    {
        readFds = master;
        if ( select(fdMax+1, &readFds, NULL, NULL, NULL) == -1 )
            perror("select");

        // 來自 server 的訊息
        if ( FD_ISSET(sockfd, &readFds) )
        {
            memset(Buffer, 0, 4096);
            int length = recv(sockfd, Buffer, 4096, 0);
            if (length==0 || length==-1)
                exit(3);
            
            if ( strncmp("Login ", Buffer, 6) == 0 )
            {
                if ( strncmp("Login Successfully: ", Buffer, 20) == 0 )
                {
                    printf("Log In Successfully!!\n");
                    sprintf(buffer, "List: \n");
                    send(sockfd, buffer, strlen(buffer), 0);
                    division();
                }
                else if ( strncmp("Login Failed: ", Buffer, 14) == 0 )
                {
                    printf("Log In Failed!!\n");
                    logIn();
                }
            }
            else if ( strncmp("List: ", Buffer, 6) == 0 )
                showList();
            else if ( strncmp("Invite Msg: ", Buffer, 12) == 0 )
                gotInvite();
            else if ( strncmp("Start game: ", Buffer, 12) == 0 )
                startGame();
            else if ( strncmp("On: ", Buffer, 4) == 0 )
                locOn();
            else if ( strncmp("Game end: ", Buffer, 10) == 0 )
                gameEnd();
        }

        // 來自 stdin 的輸入
        if ( FD_ISSET(fileno(stdin), &readFds) )
        {
            memset(Buffer, 0, 4096);
            fgets(Buffer, 4096, stdin);

            if ( strncmp("List", Buffer, 4) == 0 )
            {
                sprintf(buffer, "List: \n");
                send(sockfd, buffer, strlen(buffer), 0);
            }
            else if ( strncmp("Invite ", Buffer, 7) == 0 )
                invite();
            else if ( strncmp("Accept ", Buffer, 7) == 0 )
                acceptInvite();
            else if ( strncmp("Go ", Buffer, 3) == 0 )
                loc();
        }
    }

	close(sockfd);
    return 0;
}