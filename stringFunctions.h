#define BOARD_GET_ALL 0
#define BOARD_GET 1
#define BOARD_ADD 2
#define BOARD_DELETE 3
#define POST_ADD 4
#define POST_EDIT 5
#define POST_DELETE 6


int ScanRequest(char *httpMsg);
int CheckContentLength(char *httpMsg);
int PrintMessage(char *httpMsg);
char *LowCase(char *str);
char *GetFirstLine(char *string);
char *GetName(char *httpMsg);
char *GetBody(char *httpMsg);
char *GetHeader(char *httpMsg);
char *CreateReturnCode(int ret);
bool CheckContentType(char *httpMsg);
bool GetNameAndID(char *httpMsg, char ** name, long *id);
int requestRecognize(char *request);
const char *CreateHead(char *argv[], int argc, int *contentStartIndex);
const char *CreateBody(char *argv[], int argc);
const char *CreateCommand(char *request, char *argv[], int argc, int *contentStartIndex);
char EscSeq(char c);
char * ContentEscSeq(char content[]);


