#include<stdio.h>
#include<pthread.h>
#include<include/node.h>
#include<include/worker.h>

#define NUM_WORKERS 5

int main(int argc, char *argv[]) {
    int port = 3333;
    int thread_counter = 0;
    int sockfd;

    // parse options
    if (argc > 1) {
        port = atoi(argv[1]);
    }

    // set up this node
    sockfd = create_node(port);
    pthread_t workers[NUM_WORKERS];

    while (1) {
        int newsockfd = wait_for_connection(sockfd);// wait for a new node to connect
        printf("Verbunden mit Node #%d\n", newsockfd);
        pthread_create(&workers[thread_counter], NULL, worker_init, (void *)newsockfd);// create a new thread for handling this connection
        thread_counter += 1;
    }

    return 0;
}