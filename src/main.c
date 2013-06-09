#include<stdio.h>

int main(int argc, char *argv[]) {

    // TODO: get port from command line argument

    int sockfd = create_node(3003);
    int newsockfd = connect_node(sockfd);// this is blocking. TODO: do it in a thread

    return 0;
}