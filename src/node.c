#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>

// create a node -> listen on a certain port for incoming connections
int create_node(int port) {
    int sockfd;
    struct sockaddr_in serv_addr;

    // create
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0){
        error("ERROR, konnte Socket nicht öffnen\n");
		return -1;
	}
    bzero((char *) & serv_addr, sizeof (serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    // listen on TCP socket
    if (bind(sockfd, (struct sockaddr *) & serv_addr,sizeof (serv_addr)) < 0){
        error("ERROR, konnte Socket nicht anbinden\n");
		return -1;
	}
    listen(sockfd, 5);

    return sockfd;
}

// connect a connection from another node to this one by accepting connection attempts
int wait_for_connection(int sockfd) {
    int clilen, newsockfd;
    struct sockaddr_in cli_addr;

    clilen = sizeof(cli_addr);

    // accept client and create TCP connection
    newsockfd = accept(sockfd, (struct sockaddr *) & cli_addr, &clilen);
    if (newsockfd < 0){
        error("ERROR, konnte Client nicht anbinden");
		return -1;
	}

	return newsockfd;
}
