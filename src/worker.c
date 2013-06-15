#include<stdio.h>

void *worker_init(void *sockfd_ptr)
{
    int sockfd = (int)sockfd_ptr;
    printf("Thread wartet auf Nachricht von node #%d!\n", sockfd);

    while (1) {
        wait_for_message(sockfd);
    }

    pthread_exit(NULL);
}