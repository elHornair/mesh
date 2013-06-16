#include<stdio.h>

// parse a message
int parse_message(int sockfd) {
    // open reading stream
    FILE *read_stream = fdopen(sockfd, "r");
    if(read_stream == NULL)
    {
        error("ERROR, Konnte ReadStream nicht erstellen.");
		return -1;
    }

    // parse stream
    if (stream_to_package(read_stream) < 0) {
        error("ERROR, Ungültiges Packet erhalten.");
    }

    // close stream
    fclose(read_stream);
}

void *worker_init(void *sockfd_ptr)
{
    int sockfd = (int)sockfd_ptr;
    dbg("TCP-Verbindung hergestellt. Warte auf Packet...");
    parse_message(sockfd);
    pthread_exit(NULL);
}