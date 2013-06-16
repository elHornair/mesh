#include<stdio.h>
#include<include/util.h>

// process a message
int process_package(package *current_package) {
    switch(current_package->type){
        case 'C':
            dbg("Datenpaket erhalten");
            // TODO: handle
            break;
        case 'O':
            dbg("OK-Paket erhalten");
            // TODO: handle
            break;
        case 'N':
            dbg("Verbindungspaket erhalten");
            // TODO: handle
            break;
        default:
            break;
    }
}

// parse a message
int parse_message(int sockfd) {
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

    // close stream
    fclose(read_stream);

    // process package
    process_package(&current_package);
}

void *worker_init(void *sockfd_ptr)
{
    int sockfd = (int)sockfd_ptr;
    dbg("TCP-Verbindung hergestellt. Warte auf Packet...");
    parse_message(sockfd);
    pthread_exit(NULL);
}