typedef struct {
    short id;
    char target;
    char type;
    char message[128];
} package;

typedef struct {
    long ip;
    int port;
} neighbour;

void dbg (char* msg);
int stream_to_package(FILE *stream, package *current_package);
