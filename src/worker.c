#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<include/util.h>

extern pthread_mutex_t mutex_neighbours;

void add_neighbour(struct neighbour *neighbour_to_add) {
    struct neighbour *neighbour_item = malloc(sizeof(struct neighbour));
    int already_in_list = 0;
    char dbg_message[100];

    // lock neighbours list
    pthread_mutex_lock(&mutex_neighbours);

    // loop through existing neighbours
    LIST_FOREACH(neighbour_item, &neighbour_head, entries) {
        if (neighbour_to_add->port == neighbour_item->port) {
            sprintf(dbg_message,
                    "Nachbar mit IP %lu und Port %d ist schon registriert",
                    neighbour_to_add->ip,
                    neighbour_to_add->port);
            dbg(dbg_message);
            already_in_list = 1;
        }
    }

    // add new neighbour
    if (!already_in_list) {
        sprintf(dbg_message,
                "Nachbar mit Port %d hinzugefügt",
                neighbour_to_add->port);
        dbg(dbg_message);
        LIST_INSERT_HEAD(&neighbour_head, neighbour_to_add, entries);
    }

    // unlock neighbours list
    pthread_mutex_unlock(&mutex_neighbours);
}

int process_connection_package(package *my_package) {
    int ip_num;
    int port_num;
    struct neighbour *new_neighbour = malloc(sizeof(struct neighbour));

    memcpy(&ip_num, &(my_package->message[3]), 1);// not sure if everything's correct here
    memcpy(&port_num, &(my_package->message[4]), 2);

    new_neighbour->ip = ntohs(ip_num);
    new_neighbour->port = ntohs(port_num);

    add_neighbour(new_neighbour);

    // TODO: should we send a connection package to the sender? Only if connections are bidirectional...
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
        perror("ERROR, Konnte read stream nicht erstellen.");
		return -1;
    }

    // parse stream
    if (stream_to_package(read_stream, &current_package) < 0) {
        perror("ERROR, Ungültiges Packet erhalten.");
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