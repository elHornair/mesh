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
extern int port;
extern const char ROLE_SOURCE;
extern const char ROLE_GOAL;

void add_neighbour(struct node *neighbour_to_add) {
    struct node *neighbour_item = malloc(sizeof(struct node));
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
    struct node *new_neighbour = malloc(sizeof(struct node));

    package_message_to_node(my_package, new_neighbour);
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

int send_package(package *my_package, int receiver_port) {
    int sockfd;
    FILE *write_stream;
    char dbg_message[100];
    struct node *this_node = malloc(sizeof(struct node));

    // write information about current node into the package so the neighbour will know where it came from
    this_node->port = port;
    node_to_package_message(this_node, my_package);

    sockfd = open_connection(receiver_port);
    if (!sockfd) {
        perror("ERROR, konnte keine Verbindung herstellen\n");
        return -1;
    }

    // open stream
    write_stream = fdopen(dup(sockfd), "w");
    if(write_stream == NULL) {
        perror("ERROR, Konnte write stream nicht erstellen\n");
		return -1;
    }

    // convert package to stream
    if (package_to_stream(my_package, write_stream) < 0) {
        perror("ERROR, Packet kann nicht in Stream umgewandelt werden\n");
        return -1;
    }

    // send package
    fflush(write_stream);

    // close connection
    fclose(write_stream);
    close(sockfd);

    sprintf(dbg_message,
            "Paket mit id %d gesendet an Port %d",
            my_package->id,
            receiver_port);
    dbg(dbg_message);

    return 0;
}

// forward a package (either by knowing the direction or by flooding the network)
int forward_package(package *my_package) {
    struct node *neighbour_item = malloc(sizeof(struct node));

    dbg("Leite Paket weiter...");

    // TODO: before flooding, first check if we know in what direction to send it already

    // flood network
    if (1) {// TODO: this will say whether the searched node is in the routing table or not

        // lock neighbours list
        pthread_mutex_lock(&mutex_neighbours);

        // loop through existing neighbours
        LIST_FOREACH(neighbour_item, &neighbour_head, entries) {
            send_package(my_package, neighbour_item->port);
        }

        // unlock neighbours list
        pthread_mutex_unlock(&mutex_neighbours);
    }

    return 0;
}

// process a data package
int process_data_package(package *my_package) {
    struct node *sender_node = malloc(sizeof(struct node));
    package_message_to_node(my_package, sender_node);

    // TODO: Hier kann ich gleich den Absender in die Routingtabelle eintragen, weil ich weiss, dass er näher am
    // TODO: anderen Ende als das Ziel des Pakets dran ist, als ich. An alle anderen Nodes flute ich, es sei denn,
    // TODO: ich weiss genau, wo das paket hin muss

    if (my_package->target == 1 && role == ROLE_GOAL) {
        dbg("I'm Z and I got a message.");
        // TODO: send ok message
    } else if (my_package->target == 0 && role == ROLE_SOURCE) {
        // TODO: send ok message
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
            printf("Typ:%c\n", my_package->type);
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