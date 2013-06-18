#include<sys/queue.h>

typedef struct {
    short id;
    char target;
    char type;
    char message[128];
} package;

LIST_HEAD(neighbour_list, node) neighbour_head;
struct node {
    LIST_ENTRY(node) entries;
    long ip;
    int port;
};

void dbg (char* msg);
int stream_to_package(FILE *stream, package *current_package);
int package_to_stream(package *current_package, FILE *stream);
