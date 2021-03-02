#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>

#include "listFunctions.h"

/**
 * Struktura reprezentujici prispevky nastenky
 * Uchovava se v linearím jednosměrně vázaném seznamu
 */
typedef struct post {
    char *text;
    struct post * next;
} post_t;

/**
 * Struktura reprezentujici nastenku s ukazatelem na prvni prispevek
 * Uchovava se v linearím jednosměrně vázaném seznamu
 */
typedef struct board {
    char *name;
    struct post * firstPost;
    struct board * next;
} board_t;


/**
 * Funkce pro vlozeni nove tabulky do seznamu
 * @param head - ukazatel na prvni nastenku seznamu
 * @param name - jmeno nove vytvarene tabulky
 * @return vraci 201 pri uspechu. V pripadě, ze tabulka se stejnym
 *      uz existuje, navrati 409
 */
int InsertNewBoard(board_t ** head, char *name)
{
    board_t *node = (*head);
    board_t *prev = (*head);
    board_t *newBoard;
    if(*head == NULL)
    {
        newBoard = malloc(sizeof(board_t));
        newBoard->firstPost = NULL;
        newBoard->next = NULL;
        newBoard->name = malloc(strlen(name)+1);
        strcpy(newBoard->name, name);
        *head = newBoard;

        return 201;
    }

    while(node != NULL)
    {
        if(strcmp(name, node->name) == 0)
            return 409;

        prev = node;
        node = node->next;
    }

    newBoard = malloc(sizeof(board_t));
    newBoard->firstPost = NULL;
    newBoard->next = NULL;
    newBoard->name = malloc(strlen(name)+1);
    strcpy(newBoard->name, name);
    prev->next = newBoard;

    return 201;
}


/**
 * Funkce pro odstraneni nastenky ze seznamu
 * @param head - ukazatel na prvni nastenku seznamu
 * @param name - jmeno nastenky, kterou chceme smazat
 * @return navraci 200 v pripadě uspechu. navrati 404 pokud tato
 *      tabulka nebyla nalezena
 */
int DeleteBoard(board_t ** head, char *name)
{
    board_t *previousBoard = (*head);

    if((*head) == NULL)
        return 404;
    else if(strcmp((*head)->name, name) == 0)
    {
        board_t *nextBoard = (*head)->next;
        DeletePostsFromBoard((*head));
        free((*head)->name);
        free(*head);
        *head = nextBoard;
        return 200;
    }

    board_t *board = (*head)->next;
    while(board != NULL)
    {
        if(strcmp(name, board->name) == 0)
        {
            previousBoard->next = board->next;
            DeletePostsFromBoard(board);
            free(board->name);
            free(board);
            return 200;
        }

        previousBoard = board;
        board = board->next;
    }

    return 404;
}


/**
 * Funkce pro ziskani vsech nastenek
 * @param head - ukazatel na prvni nastenku seznamu
 * @return Navraci text s nazvy nastenek
 */
char * ListBoards(board_t **head)
{
    int count = 0;
    char *boards = malloc(sizeof(char));
    board_t *board = (*head);
    strcpy(boards, "");

    while(board != NULL)
    {
        boards = realloc(boards, strlen(boards) + strlen(board->name)+2);
        strcat(boards,board->name);
        strcat(boards,"\n");
        board = board->next;
        count++;
    }

    if(count > 0)
        return boards;

    return boards;
}


/**
 * Funkce pro smazani vsech nastenek vcetne jejich obsahu
 * @param head - ukazatel na prvni nastenku seznamu
 */
void DeleteAllBoards(board_t ** head)
{
    board_t *board = (*head);

    while(board != NULL)
    {
        board_t *temporaryBoard = board->next;

        DeletePostsFromBoard(board);
        free(board->name);
        free(board);

        board = temporaryBoard;
    }
}


/**
 * Vyhledani tabulky s konkretnim pojmenovanim
 * @param head - ukazatel na prvni nastenku seznamu
 * @param name - Jmeno hledane tabulky
 * @return Navrati vyhledanou tabulku. Popripade NULL, pokud nebyla nalezena
 */
board_t * GetBoard(board_t **head, char *name)
{
    board_t *board = (*head);

    while (board != NULL) {
        if(strcmp(name, board->name) == 0)
        {
            return board;
        }
        board = board->next;
    }

    return NULL;
}


/**
 * Zobrazi obsah konkretni nastenky
 * @param board - nastenka ktera ma byt zobrazena
 * @return Navrací text s obsahem dane nastenky
 */
char * ListPosts(board_t *board)
{
    if(board == NULL)
        return NULL;

    char *posts;
    post_t *post = board->firstPost;
    int i = 1;

    posts = malloc(strlen(board->name)+5);
    strcpy(posts, "[");
    strcat(posts,board->name);
    strcat(posts,"]\n");

    while(post != NULL)
    {
        posts = realloc(posts, strlen(posts) + strlen(post->text)+5);
        char number[12];
        sprintf(number, "%d. ", i);
        strcat(posts,number);
        strcat(posts,post->text);
        strcat(posts,"\n");
        post = post->next;
        i++;
    }

    return posts;
}


/**
 * Vlozi novy prospek do nastenky
 * @param board - nastenka, do ktere ma byt prospevek vlozen
 * @param content - Text prispevku
 * @return v pripade uspechu vrati 201. Pokud nastenka neexistuje, vrati 404
 */
int InsertNewPost(board_t * board, char *content)
{
    if(board == NULL)
        return 404;

    post_t *post = board->firstPost;
    post_t *newPost = malloc(sizeof(post_t));
    newPost->next = NULL;
    newPost->text = malloc(strlen(content)+1);
    strcpy(newPost->text, content);

    if(board->firstPost == NULL)
    {
        board->firstPost = newPost;
        return 201;
    }

    while(post->next != NULL)
    {
        post = post->next;
    }

    post->next = newPost;

    return 201;
}


/**
 * Funkce smaze prispevek na pozici v dane nastence
 * @param board - Nastenka, kde ma but prispevek smazan
 * @param index - Poradi prispevku, kterz ma byt smazan
 * @return V pripade uspechu navraci 200. Pokud tabulka neexistuje,
 *      nebo index je mimo rozmezi poctu prispevku, vraci 404
 */
int DeletePost(board_t * board, int index)
{
    if(board == NULL || board->firstPost == NULL)
        return 404;

    post_t *previousPost = board->firstPost;
    post_t *post = board->firstPost;
    int i = 1;

    while(i < index)
    {
        previousPost = post;
        post = post->next;
        i++;

        if(post == NULL)
            return 404;
    }

    if(index == 1)
        board->firstPost = post->next;
    else
        previousPost->next = post->next;

    free(post->text);
    free(post);

    return 200;
}


/**
 * Funkce zmeni text prispevku na pozici v dane nastence
 * @param board - Nastenka, kde ma but prispevek smazan
 * @param index - Poradi prispevku, ktery ma byt zmenen
 * @param text - text, ktery ma byt prispevku prirazen
 * @return V pripade uspechu navraci 200. Pokud tabulka neexistuje,
 *      nebo index je mimo rozmezi poctu prispevku, vraci 404
 */
int EditPost(board_t * board, int index, char *text)
{
    if(board == NULL || board->firstPost == NULL)
        return 404;

    post_t *post = board->firstPost;
    int i = 1;

    while(i < index)
    {
        post = post->next;
        i++;

        if(post == NULL)
            return 404;
    }

    free(post->text);
    post->text = malloc(strlen(text)+1);
    strcpy(post->text, text);

    return 200;
}


/**
 * Smaze veskere prispevky v dane nastence
 * @param board - nastenka, ktera ma být promazana
 */
void DeletePostsFromBoard(board_t * board)
{
    post_t *post = board->firstPost;

    while(post != NULL)
    {
        post_t *temporaryPost = post->next;
        free(post->text);
        free(post);
        post = temporaryPost;
    }
}