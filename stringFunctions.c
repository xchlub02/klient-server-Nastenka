/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>
#include <ctype.h>

#include "stringFunctions.h"

/**
 * Vytvori zpravu pro danou navratovou hodnotu. Napr pro hodnotu
 *      201 prida do zpravy "201 Created"
 * @param ret - navratova hodnota
 * @return Text, se zpravou navratove hodnoty
 */
char *CreateReturnCode(int ret)
{
    char *retStr = malloc(sizeof(char) * 20);

    switch(ret)
    {
        case 200:
            strcpy(retStr, "200 OK");
            break;

        case 201:
            strcpy(retStr, "201 Created");
            break;

        case 400:
            strcpy(retStr, "400 Bad Request");
            break;

        case 404:
            strcpy(retStr, "404 Not Found");
            break;

        case 409:
            strcpy(retStr, "409 Conflict");
            break;

        default:
            strcpy(retStr, "");
    }

    return retStr;
}


/**
 * Navraci text tela http protokolu
 * @param httpMsg - obsah http protokolu
 * @return navraci text obsahujici http telo
 */
char *GetBody(char *httpMsg)
{
    int length = CheckContentLength(httpMsg);
    char *bodyFoo = strstr(httpMsg, "\r\n\r\n");
    char *body = malloc(sizeof(char) * length + 500000);
    if(!CheckContentType(httpMsg))
        return NULL;

    if(bodyFoo != NULL && strlen(bodyFoo) > 4)
        bodyFoo = bodyFoo + 4;
    else
        return NULL;

    memcpy(body, bodyFoo, 135);
    body[length] = 0;
    return body;
}


/**
 * Navraci text hlavicky http protokolu
 * @param httpMsg - obsah http protokolu
 * @return navraci text s obsahem http protokolu
 */
char *GetHeader(char *httpMsg)
{
    char *body = strstr(httpMsg, "\r\n\r\n");
    int length = body - httpMsg;
    char *header = malloc(length +1);

    strncpy(header, httpMsg, length);
    header[length] = 0;

    return header;
}


/**
 * Ziska nazev nastenky z http hlavicky
 * @param httpMsg - obsah http protokolu
 * @return navraci jmeno nastenky
 */
char *GetName(char *httpMsg)
{
    char *line;

    if((line = strstr(httpMsg, "boards/")) != NULL && strlen(line) > 7)
        line = line + 7;
    else if((line = strstr(httpMsg, "board/")) != NULL && strlen(line) > 6)
        line = line + 6;
    else
        return NULL;

    char *e = strchr(line, ' ');
    int index = (int)(e - line);
    char *name = malloc(sizeof(char)*index+2);
    strncpy(name, line, index);
    name[index] = 0;

    return name;
}


/**
 * Ziska jmeno nastenky a id prispevku z http hlavicky
 * @param httpMsg - obsah http protokolu
 * @param name - promena, ve ktere se predava jmeno nastenky
 * @param id - promena, ve ktere se predava id prispevku
 * @return navraci true pokud se tyto hodnoty v hlavicce nachazi.
 */
bool GetNameAndID(char *httpMsg, char ** name, long *id)
{
    char *line, *err;
    long idFoo = -1;

    if((line = strstr(httpMsg, "board/")) != NULL && strlen(line) > 6)
        line = line + 6;
    else
        return false;

    char *e = strchr(line, '/');
    int index = (int)(e - line);
    char *Foo = malloc(sizeof(char)*index+2);
    strncpy(Foo, line, index);
    Foo[index] = 0;
    *name = Foo;
    if((line = strstr(httpMsg, Foo)) != NULL && strlen(line) > strlen(Foo)+1)
        line = line + strlen(Foo)+1;
    else
    {

        free(Foo);
        return false;
    }

    e = strchr(line, ' ');
    index = (int)(e - line);
    Foo = malloc(sizeof(char)*index+2);
    strncpy(Foo, line, index);
    Foo[index] = 0;

    idFoo = (long) strtol(Foo,&err,10);
    if (*err != '\0' || idFoo < 0)
    {
        free(*name);
        free(Foo);
        return false;
    }

    free(Foo);
    *id = idFoo;
    return true;
}


/**
 * Funkce pro prevedeni textu do lowercase podoby
 * @param str - vstupni text, kterz ma byt zmenen
 * @return Navraci zmeneny text
 */
char *LowCase(char *str)
{
    int j = 0;
    char *newStr = malloc(sizeof(char) * strlen(str));

    for(int i = 0; str[i]; i++){
        if(str[i] != ' ')
        {
            newStr[j] = tolower(str[i]);
            j++;
        }
    }
    newStr[j] = 0;
    return newStr;
}


/**
 * Zknotroluje, zdali hlavicka obsahuje spravny Content-Type
 * @param httpMsg - Obsah http protokolu
 * @return Navraci, jestli obsahuje, nebo ne
 */
bool CheckContentType(char *httpMsg)
{
    char *newStr = LowCase(httpMsg);
    bool contains = strstr(newStr, "content-type:text/plain") != NULL;
    free(newStr);
    return contains;
}


/**
 * Navrati delku obsahu http tela
 * @param httpMsg - Obsah http protokolu
 * @return navraci int reprezentujici delku obsahu tela
 */
int CheckContentLength(char *httpMsg)
{
    char *newStr = LowCase(httpMsg);
    char *contentLength = strstr(newStr, "content-length:");
    if(contentLength == NULL)
        return -1;

    long length = 0;
    char *err;

    char *line = GetFirstLine(contentLength);
    char *lineNew = strstr(line, ":");
    free(newStr);

    if(lineNew != NULL && strlen(lineNew) > 1)
        lineNew = lineNew + 1;
    else
        return -1;

    length = (long) strtol(lineNew,&err,10);
    free(line);
    if (*err == '\0')
    {
        return length;
    }
    return -1;
}


/**
 * Zjiti o jaky typ pozadavku se jedna
 * @param httpMsg - Obsah http protokolu
 * @return - Navraci cislo reprezentujici jednotlive pozadavky
 */
int ScanRequest(char *httpMsg)
{
    char *request = GetFirstLine(httpMsg);
    int requestInt = -1;

    if (request == NULL)
        return -1;

    if (strstr(request, "GET /boards") != NULL) requestInt =  BOARD_GET_ALL;
    else if (strstr(request, "POST /boards") != NULL) requestInt =  BOARD_ADD;
    else if (strstr(request, "DELETE /boards") != NULL) requestInt =  BOARD_DELETE;
    else if (strstr(request, "GET /board") != NULL) requestInt =  BOARD_GET;
    else if (strstr(request, "POST /board") != NULL) requestInt =  POST_ADD;
    else if (strstr(request, "PUT /board") != NULL) requestInt =  POST_EDIT;
    else if (strstr(request, "DELETE /board") != NULL) requestInt =  POST_DELETE;

    free(request);
    return requestInt;
}


/**
 * vraci prvni radek retezce (text pred \r\n)
 * @param string - vstupni text
 * @return navrati text prvniho radku
 */
char *GetFirstLine(char *string)
{
    char *e = strchr(string, '\r');
    int index = (int)(e - string);
    char *line = malloc(sizeof(char)*index+2);
    strncpy(line, string, index);
    line[index] = 0;
    return line;
}


/**
 * Vytvori hlavisku s dotazem na server
 * @param argv - argumenty zadane pri spusteni klient aplikace
 * @param argc - pocet zadanych argumentu
 * @param contentStartIndex - Promena, ve ktere se uchovava na kterem
 *      indexu se nachazi text, ktery bude vlozen do tela
 * @return vraci text hlavicky
 */
const char *CreateHead(char *argv[], int argc, int *contentStartIndex)
{
    char *command;
    const char *head;

    if(argc > 6)
    {
        command = malloc(strlen(argv[5]) + strlen(argv[6]) + 1);
        strcpy(command, argv[5]);
        strcat(command, argv[6]);
        head = CreateCommand(command,argv,argc, contentStartIndex);
        free(command);
    }
    else
    {
        command = malloc(strlen(argv[5])+ 1);
        strcpy(command, "GET /boards");
        return command;
    }

    return head;
}


/**
 * Ze vstupniho retezce zadaneho v argumentech klienta prida esc seqence
 * @param content - obsah textu
 * @return vraci zmeneny text
 */
char * ContentEscSeq(char content[])
{
    char *newContent = (char *) malloc(sizeof(char) * strlen(content));
    unsigned int j = 0;

    for(unsigned int i = 0; j < strlen(content); i++)
    {
        if(content[j] == '\\')
        {
            newContent[i] = EscSeq(content[j+1]);
            j++;
        }
        else
        {
            newContent[i] = content[j];
        }
        j++;
    }
    return newContent;
}

/**
 * Funkce vytvori obsah http pozadavku klienta
 * @param request - text argumentu aplikace klienta
 * @param argv - argumenty aplikace klienta
 * @param argc - pocet argumentu
 * @param contentStartIndex - Promena, ve ktere se uchovava na kterem
 *      indexu se nachazi text, ktery bude vlozen do tela
 * @return navraci pozadavek klienta, ktery se vlozi do hlavicky
 */
const char *CreateCommand(char *request, char *argv[], int argc, int *contentStartIndex)
{
    static char command[100];

    if(strcmp(request,"boardadd")==0 && argc > 7)
    {
        sprintf(command, "POST /boards/%s", argv[7]);
    }
    else if(strcmp(request,"boarddelete")==0 && argc > 7)
    {
        sprintf(command, "DELETE /boards/%s", argv[7]);
    }
    else if(strcmp(request,"boardlist")==0 && argc > 7)
    {
        sprintf(command, "GET /board/%s", argv[7]);
    }
    else if(strcmp(request,"itemadd")==0 && argc > 7)
    {
        sprintf(command, "POST /board/%s", argv[7]);
        *contentStartIndex = 8;
    }
    else if(strcmp(request,"itemdelete")==0 && argc > 8)
    {
        sprintf(command, "DELETE /board/%s/%s", argv[7], argv[8]);
    }
    else if(strcmp(request,"itemupdate")==0 && argc > 8)
    {
        sprintf(command, "PUT /board/%s/%s", argv[7], argv[8]);
        *contentStartIndex = 9;
    }
    else if(strcmp(request,"boards")==0 && argc > 6)
    {
        sprintf(command, "GET /boards");
    }
    else
        return NULL;

    return command;
}


/**
 * Navrati danou esc sekvenci podle vstupniho charu
 * @param c - znak za zpetnym lomitkem
 * @return vraci esc sekvenci
 */
char EscSeq(char c)
{
    switch (c)
    {
        case 't':
            return '\t';
        case '0':
            return '\0';
        case 'n':
            return '\n';
        case 'r':
            return '\r';
        case '\\':
            return '\\';
        case '\"':
            return '\"';
        case '\'':
            return '\'';
        default:
            return c;
    }
}


/**
 * Vytiskne obsah prichozi spravy od serveru http head na stderr a http body na stdout
 * @param httpMsg - obsah http protokolu
 * @return vraci uspech (0) nebo neuspech (-1) pozadavku klienta
 */
int PrintMessage(char *httpMsg)
{
    char *header = GetHeader(httpMsg);
    char *body;
    if(strstr(httpMsg,"200 OK") != NULL)
    {

        body = GetBody(httpMsg);
        fprintf(stderr, "%s\n", header);

        if(body != NULL)
        {
            fprintf(stdout, "%s\n", body);
        }

        free(body);
        free(header);
        return 0;
    }
    else if(strstr(httpMsg,"201 Created") != NULL)
    {
        fprintf(stderr, "%s\n", header);
        free(header);

        return 0;
    }
    else if(strstr(httpMsg,"400 Bad Request") != NULL)
    {
        fprintf(stderr, "%s\n", header);
        free(header);
        return -1;
    }
    else if(strstr(httpMsg,"404 Not Found") != NULL)
    {
        fprintf(stderr, "%s\n", header);
        free(header);

        return -1;
    }
    else if(strstr(httpMsg,"409 Conflict") != NULL)
    {
        fprintf(stderr, "%s\n", header);
        free(header);

        return -1;
    }

    return -1;
}