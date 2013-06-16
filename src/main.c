#include<stdio.h>
#include<pthread.h>
#include<include/node.h>
#include<include/worker.h>
#include<include/util.h>

#define NUM_WORKERS 5

int port = 3333;

int main(int argc, char *argv[]) {
    int thread_counter = 0;
    int sockfd;
    int newsockfd;
    pthread_t workers[NUM_WORKERS];

    // parse options
    if (argc > 1) {
        port = atoi(argv[1]);
    }

    // set up this node
    sockfd = create_node(port);
    dbg("Erstellt");

    // TODO: refactor so it reuses the threads
    while (1) {
        newsockfd = wait_for_connection(sockfd);// wait for a new node to connect
        pthread_create(&workers[thread_counter], NULL, worker_init, (void *)newsockfd);// create a new thread for handling this connection
        thread_counter += 1;
    }

    return 0;
}