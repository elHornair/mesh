#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<include/node.h>
#include<include/worker.h>
#include<include/util.h>

#define NUM_WORKERS 5

const char ROLE_DEFAULT = 'N';
const char ROLE_SOURCE = 'Q';
const char ROLE_GOAL = 'Z';

const char TYPE_CONTENT = 'C';
const char TYPE_OK = 'O';
const char TYPE_NEIGHBOUR = 'N';

int port = 3333;// the port this node runs on
int role;// the role of this node
struct router *my_router;// the routing table of this node


// TODO: liste führen mit daten-packages, die schon geforwarded wurden. dann packet nur einmal forwarden
// TODO: für ok-packages ist es kein problem, da wir dann die route schon wissen

pthread_mutex_t mutex_neighbours;
pthread_mutex_t mutex_router;

int parse_config(int argc, char *argv[]) {
    int opt;

    // default value for role
    role = ROLE_DEFAULT;

    while ((opt = getopt (argc, argv, "-qzh")) != -1)
        switch (opt) {
            case 'q':
                role = ROLE_SOURCE;
                break;
            case 'z':
                role = ROLE_GOAL;
                break;
            case 1:
                port = atoi(argv[1]);
                break;
            case 'h':
            default:
                printf("Format: mesh [port] [-q (for source node)|-z (for goal node)|-h (for displaying help)]\n");
                return 0;
    }

    return 1;
}

int main(int argc, char *argv[]) {
    int thread_counter = 0;
    int sockfd;
    int newsockfd;
    pthread_t workers[NUM_WORKERS];
    my_router = malloc(sizeof(struct router));

    // parse config
    if (!parse_config(argc, argv)) {
        return -1;
    }

    // set up this node
    sockfd = create_node(port);

    if (sockfd == -1) {
        return -1;
    }
    dbg("Erstellt");

    // init list of all connected neighbours
    LIST_INIT(&neighbour_head);

    // TODO: refactor so it reuses the threads
    // TODO: create the threads upfront
    while (1) {
        newsockfd = wait_for_connection(sockfd);// wait for a new node to connect
        pthread_create(&workers[thread_counter], NULL, worker_init, (void *)newsockfd);// create a new thread for handling this connection
        thread_counter += 1;
    }

    return 0;
}