#include<stdio.h>

int main(int argc, char *argv[]) {

    // TODO: get port from command line argument

    int sockfd = create_node(3004);
    int newsockfd = wait_for_connection(sockfd);// this is blocking

    printf("Accepted client with descriptor number %d now\n", newsockfd);

    // wait for a message as long as it takes
    while (1) {
        wait_for_message(newsockfd);
    }

    return 0;
}