#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<include/node.h>
#include<include/worker.h>
#include<include/util.h>

#define NUM_WORKERS 5
#define ROLE_DEFAULT 0
#define ROLE_SOURCE 1
#define ROLE_GOAL 2

int port = 3333;
int role = ROLE_DEFAULT;

pthread_mutex_t mutex_neighbours;

int parse_config(int argc, char *argv[]) {
    int opt;

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

    // parse config
    if (!parse_config(argc, argv)) {
        return -1;
    }

    // set up this node
    sockfd = create_node(port);
    dbg("Erstellt");

    // init list of all connected neighbours
    LIST_INIT(&neighbour_head);

    // TODO: refactor so it reuses the threads
    while (1) {
        newsockfd = wait_for_connection(sockfd);// wait for a new node to connect
        pthread_create(&workers[thread_counter], NULL, worker_init, (void *)newsockfd);// create a new thread for handling this connection
        thread_counter += 1;
    }

    return 0;
}