#include<sys/queue.h>

// ------------------------- structs -------------------------

struct router {
    short goal_neighbour;// the port of the neighbour that packages with the target "goal" are to be forwarded
    short source_neighbour;// the port of the neighbour that packages with the target "source" are to be forwarded
    // for both properties, an entry of 0 means that the respective neighbour is not yet known
};

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

// ----------------------- functions ------------------------

void dbg (char* msg);

int package_to_stream(package *my_package, FILE *stream);
int stream_to_package(FILE *stream, package *current_package);

int package_message_to_node(package *my_package, struct node *new_node, int start_byte);
int node_to_package_message(struct node *my_node, package *my_package, int start_byte);