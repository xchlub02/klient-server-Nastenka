struct board;
typedef struct board board_t;

struct post;
typedef struct post post_t;


int InsertNewPost(board_t * board, char *content);
int DeletePost(board_t * board, int index);
int EditPost(board_t * board, int index, char *text);
int InsertNewBoard(board_t ** head, char *name);
int DeleteBoard(board_t ** head, char *name);
char * ListBoards(board_t **head);
char * ListPosts(board_t *board);
void DeletePostsFromBoard(board_t * board);
void DeleteAllBoards(board_t ** head);
board_t * GetBoard(board_t **head, char *name);




