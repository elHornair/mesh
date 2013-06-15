#include<stdio.h>
#include<pthread.h>

#define NUM_WORKERS 5

void *worker_init(void *sockfd_ptr)
{
    int sockfd = (int)sockfd_ptr;
    printf("Thread wartet auf Nachricht von node #%d!\n", sockfd);

    while (1) {
        wait_for_message(sockfd);
    }

    pthread_exit(NULL);
}

// TODO: get port from command line argument
int main(int argc, char *argv[]) {
    int sockfd = create_node(3003);
    int thread_counter = 0;
    pthread_t workers[NUM_WORKERS];

    while (1) {
        int newsockfd = wait_for_connection(sockfd);// wait for a new node to connect
        printf("Verbunden mit Node #%d\n", newsockfd);
        pthread_create(&workers[thread_counter], NULL, worker_init, (void *)newsockfd);// create a new thread for handling this connection
        thread_counter += 1;
    }

    return 0;
}