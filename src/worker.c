#include<stdio.h>
#include<string.h>
#include<include/util.h>

void add_neighbour(neighbour *my_neighbour) {
    printf("The port is: %d\n", my_neighbour->port);
}

int process_connection_package(package *my_package) {
    int ip_num;
    int port_num;
    neighbour new_neighbour;

    memcpy(&ip_num, &(my_package->message[3]), 1);
    memcpy(&port_num, &(my_package->message[4]), 2);

    new_neighbour.ip = ntohs(ip_num);
    new_neighbour.port = ntohs(port_num);

    add_neighbour(&new_neighbour);
}

// process a message
int process_package(package *my_package) {
    switch(my_package->type){
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
            process_connection_package(my_package);
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