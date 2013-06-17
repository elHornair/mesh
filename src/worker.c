#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<include/util.h>

extern pthread_mutex_t mutex_neighbours;
extern int role;
extern const char ROLE_SOURCE;
extern const char ROLE_GOAL;

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

// process a connection package
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

// open connection to another node
int open_connection(int receiver_port) {
    int sockfd;
    struct hostent *host_addr;
    struct sockaddr_in server;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        perror("ERROR, konnte Socket nicht erstellen\n");
        return -1;
    }

    host_addr = gethostbyname("localhost");
    if (!host_addr) {
        perror("ERROR, konnte hostname nicht auflösen\n");
        return -1;
    }

    memcpy(&server.sin_addr, host_addr->h_addr_list[0], host_addr->h_length);
    server.sin_family = AF_INET;
    server.sin_port = htons(receiver_port);

    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server))) {
        perror("ERROR, konnte Socket nicht verbinden\n");
        return -1;
    }

    return sockfd;
}

// forward a package (either by knowing the direction or by flooding the network)
int forward_package(package *my_package) {
    // TODO: before flooding, first check if we know in what direction to send it already
    int sockfd;

    dbg("Leite Paket weiter...");

    sockfd = open_connection(3311);// TODO: use the IP from the neighbourstable

    if (!sockfd) {
        perror("ERROR, konnte keine Verbindung herstellen\n");
        return -1;
    }

    // TODO: now send data
}

// process a data package
int process_data_package(package *my_package) {
    if (my_package->target == 1 && role == ROLE_GOAL) {
        dbg("I'm Z and I got a message.");
    } else if (my_package->target == 0 && role == ROLE_SOURCE) {
        dbg("I'm Q and I got a message.");
    } else {
        forward_package(my_package);
    }
}

// process a message
int process_package(package *my_package) {
    switch(my_package->type){
        case 'C':
            dbg("Datenpaket erhalten");
            process_data_package(my_package);
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
            dbg("Paket mit unbekanntem Typ erhalten");
            printf("typ:%c\n", my_package->type);
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