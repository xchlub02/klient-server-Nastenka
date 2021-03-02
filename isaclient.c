#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdbool.h>

#include "stringFunctions.h"

#define BUF_SIZE 2048


bool checkArgs(int count, char *args[], long *port, char *host);


int main(int argc, char *argv[])
{
    int my_sck;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char host[100]="";
    char buffer[4096];
    char content[3800]="";
    const char *command;
    char *correctContent;
    long port;
    int contentStartIndex = -1;

    if (!checkArgs(argc,argv,&port,host))
    {
        fprintf(stderr, "Spatne zadane argumenty programu\n");
        return -1;
    }

    if((command=CreateHead(argv,argc, &contentStartIndex)) == NULL)
    {
        fprintf(stderr, "Spatne zadane argumenty programu\n");
        return -1;
    }

    if(contentStartIndex > -1 && argc > contentStartIndex)
    {
        sprintf(content,"%s",argv[contentStartIndex]);
        correctContent = ContentEscSeq(content);
    }


    bzero(buffer,sizeof(buffer));

    sprintf(buffer, "%s HTTP/1.1\r\nHost: %s\r\n"
                    "Content-Type: text/plain\r\nContent-Length: %ld\r\n\r\n%s",
            command, host, strlen(content),correctContent);
    free(correctContent);
    my_sck = socket(AF_INET, SOCK_STREAM, 0);

    if (my_sck < 0)
    {
        fprintf(stderr,"Chyba pri otevirani socketu\n");
        close(my_sck);
        return -1;
    }
    server = gethostbyname(host);

    if (server == NULL) {
        fprintf(stderr,"Chybne zadany host\n");
        close(my_sck);
        return -1;
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    bcopy(server->h_addr, &serv_addr.sin_addr.s_addr, server->h_length);

    if (connect(my_sck,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    {
        fprintf(stderr,"Chyba pri spojeni se serverem\n");
        close(my_sck);
        return -1;
    }

    if (send(my_sck,buffer, strlen(buffer),0) < 0)
    {
        fprintf(stderr,"Chyba pri odesilani souboru\n");
        return -1;
    }
    bzero(buffer,sizeof(buffer));
    recv(my_sck, buffer, sizeof(buffer),0);
    close(my_sck);
    return PrintMessage(&buffer[0]);
}

bool checkArgs(int count, char *args[], long *port, char *host)
{
    int opt;
    char *err;
    bool hostDeclared, portDeclared;
    hostDeclared = portDeclared = false;



    while((opt = getopt(count, args, "H:p:h")) != -1)
    {
        switch(opt)
        {
            case 'H':
                strcpy(host, optarg);
                hostDeclared = true;

                break;
            case 'p':
                *port = (long) strtol(optarg,&err,10);

                if (*err != '\0')
                {
                    fprintf(stderr,"Spatne zadany port\n");
                    return false;
                }
                portDeclared = true;
                break;

            case 'h':
                fprintf(stdout, "Klient aplikace se spousti nasledovne:\n./isaclient -H <host> -p <port> <command>\n"
                        "kde <host> je server, na ktery se pripojujeme,\n"
                        " <port> je cislo oznacujici na kterem portu server bude naslouchat, "
                        "toto cislo musi byt v rozsahu 49152 az 65535.\n"
                        "<command> muze byt nasledujici: \n"
                        "boards\n"
                        "board add <name>\n"
                        "board delete <name>\n"
                        "board list <name>\n"
                        "item add <name> <content>\n"
                        "item delete <name> <id>\n"
                        "item update <name> <id>\n"
                        "\n");


                exit(EXIT_SUCCESS);
        }
    }

    if(count <= 5)
        return false;

    return hostDeclared && portDeclared;
}