#include<stdio.h>
#include<pthread.h>

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

    int sockfd = create_node(3004);
    pthread_t msg_thread;

    // TODO: create array of threads (threadpool)
    while (1) {
        int newsockfd = wait_for_connection(sockfd);// wait for a new node to connect
        printf("Verbunden mit Node #%d\n", newsockfd);
        pthread_create(&msg_thread, NULL, worker_init, (void *)newsockfd);// create a new thread for handling this connection
    }

    return 0;
}