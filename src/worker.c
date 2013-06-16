#include<stdio.h>
#include<include/util.h>

// parse a message
int parse_message(int sockfd) {
    char dbg_str[80];
    package current_package;
    FILE *read_stream;

    // open reading stream
    read_stream = fdopen(sockfd, "r");
    if(read_stream == NULL)
    {
        error("ERROR, Konnte ReadStream nicht erstellen.");
		return -1;
    }

    // parse stream
    if (stream_to_package(read_stream, &current_package) < 0) {
        error("ERROR, Ungültiges Packet erhalten.");
    }

    sprintf(dbg_str, "Received package with type %c", current_package.type);
    dbg(dbg_str);

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