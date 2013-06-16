#include<sys/queue.h>

typedef struct {
    short id;
    char target;
    char type;
    char message[128];
} package;

LIST_HEAD(neighbour_list, neighbour) neighbour_head;
struct neighbour {
    LIST_ENTRY(neighbour) entries;
    long ip;
    int port;
};

void dbg (char* msg);
int stream_to_package(FILE *stream, package *current_package);
