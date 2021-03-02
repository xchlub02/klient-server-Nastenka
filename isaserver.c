#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <signal.h>

#include "listFunctions.h"
#include "stringFunctions.h"

#define BUF_SIZE 4096

bool checkArgs(int count, char *args[], long *port);
int ScanRequest(char *httpReq);
int ExecuteGetBoards(char ** body);
int ExecuteGetBoardContent(char *httpReq, char ** body);
int ExecuteAddBoard(char *httpReq);
int ExecuteDeleteBoard(char *httpReq);
int ExecuteAddPost(char *httpReq);
int ExecuteEditPost(char *httpReq);
int ExecuteDeletePost(char *httpReq);
char *ExecuteRequest(char *httpReq);void intHandler();


board_t *head = NULL;
int my_sck;


int main(int argc,char *argv[])
{
    signal(SIGINT, intHandler);

    int cl_sck;
    long portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    if(checkArgs(argc, argv, &portno))

    my_sck = socket(AF_INET, SOCK_STREAM, 0);

    if (my_sck < 0)
    {
        fprintf(stderr,"Chyba pri otevirani socketu\n");
        close(my_sck);
        return -1;
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(my_sck, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        fprintf(stderr,"Chyba pri bindovani socketu\n");
        close(my_sck);
        return -1;
    }

    listen(my_sck,5);
    clilen = sizeof(cli_addr);

    while(true)
    {
        cl_sck = accept(my_sck, (struct sockaddr *) &cli_addr, &clilen);

        if (cl_sck < 0)
        {
            fprintf(stderr,"Chyba pri otevirani socketu\n");
            close(my_sck);
        }

        char data[BUF_SIZE];

        bzero(data,BUF_SIZE);
        read(cl_sck,data,BUF_SIZE);

        char *response = ExecuteRequest(data);

        if (send(cl_sck,response, strlen(response),0) < 0)
        {
            fprintf(stderr,"Chyba pri odesilani souboru\n");
        }

        free(response);
    }
}

bool checkArgs(int count, char *args[], long *port)
{
    int opt;
    char *err;
    bool portDeclared = false;

    if(count > 3)
        return false;

    while((opt = getopt(count, args, "p:h")) != -1)
    {
        switch(opt) {
            case 'p':
                *port = (long) strtol(optarg, &err, 10);

                if (*err != '\0') {
                    fprintf(stderr, "Spatne zadany port\n");
                    return false;
                }
                portDeclared = true;
                break;

            case 'h':
                fprintf(stdout, "Server aplikace se spousti nasledovne:\n./isaserver -p <port>\n"
                        "kde <port> je cislo oznacujici na kterem portu server bude naslouchat.\n"
                        "Toto cislo musi byt v rozsahu 49152 az 65535.\n");


                exit(EXIT_SUCCESS);
        }
    }

    return portDeclared;
}

void intHandler()
{
    DeleteAllBoards(&head);
    close(my_sck);
    exit(0);
}

char *ExecuteRequest(char *httpReq)
{
    int retVal = -1;
    char *body = NULL;
    char httpHead[500];
    char *httpResponse;
    char *code;

    switch(ScanRequest(httpReq))
    {
        case BOARD_GET_ALL:
            retVal = ExecuteGetBoards(&body);
            break;
        case BOARD_GET:
            retVal = ExecuteGetBoardContent(httpReq, &body);
            break;
        case BOARD_ADD:
            retVal = ExecuteAddBoard(httpReq);
            break;
        case BOARD_DELETE:
            retVal = ExecuteDeleteBoard(httpReq);
            break;
        case POST_ADD:
            retVal = ExecuteAddPost(httpReq);
            break;
        case POST_EDIT:
            retVal = ExecuteEditPost(httpReq);
            break;
        case POST_DELETE:
            retVal = ExecuteDeletePost(httpReq);
            break;
        default:
            retVal = 404;
    }

    code = CreateReturnCode(retVal);

    if(body == NULL)
    {
        sprintf(httpHead, "HTTP/1.1 %s\r\n\r\n", code);
        httpResponse = malloc(strlen(httpHead) + 1);
        strcpy(httpResponse, httpHead);
    }
    else
    {
        int length = strlen(body);
        sprintf(httpHead, "HTTP/1.1 %s\r\n"
                        "Content-Type: text/plain\nContent-Length: %d\r\n\r\n%s",
                code, length, body);
        httpResponse = malloc(strlen(httpHead) + strlen(body) + 1);
        strcpy(httpResponse, httpHead);
        free(body);
    }

    free(code);

    return httpResponse;
}

int ExecuteGetBoards(char ** body)
{
    if((*body = ListBoards(&head)) != NULL)
        return 200;

    return 200;
}

int ExecuteGetBoardContent(char *httpReq, char ** body)
{
    char *name = GetName(httpReq);
    board_t *board = GetBoard(&head, name);
    free(name);


    if((*body = ListPosts(board)) == NULL)
        return 404;

    return 200;
}

int ExecuteAddBoard(char *httpReq)
{
    char *name = GetName(httpReq);
    int ret = InsertNewBoard(&head, name);
    free(name);

    return ret;
}

int ExecuteDeleteBoard(char *httpReq)
{
    char *name = GetName(httpReq);
    int ret = DeleteBoard(&head, name);
    free(name);

    return ret;
}

int ExecuteAddPost(char *httpReq)
{
    if(CheckContentLength(httpReq) <= 0 || !CheckContentType(httpReq))
        return 400;

    char *name = GetName(httpReq);
    board_t *board = GetBoard(&head, name);
    char *reqBody = GetBody(httpReq);
    int ret = InsertNewPost(board, reqBody);

    free(reqBody);
    free(name);

    return ret;
}

int ExecuteEditPost(char *httpReq)
{
    if(CheckContentLength(httpReq) <= 0 || !CheckContentType(httpReq))
        return 400;

    char *name = NULL;
    long id;
    if(!GetNameAndID(httpReq, &name, &id))
        return 404;

    board_t *board = GetBoard(&head, name);
    char *reqBody = GetBody(httpReq);
    int ret = EditPost(board, id, reqBody);
    free(reqBody);
    free(name);

    return ret;
}

int ExecuteDeletePost(char *httpReq)
{
    char *name = NULL;
    long id;
    if(!GetNameAndID(httpReq, &name, &id))
        return 404;

    board_t *board = GetBoard(&head, name);

    int ret = DeletePost(board, id);

    free(name);

    return ret;
}