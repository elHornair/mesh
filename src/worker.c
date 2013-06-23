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
extern pthread_mutex_t mutex_router;
extern pthread_mutex_t mutex_blacklist;
extern int role;
extern int port;
extern struct router *my_router;
extern char package_id_blacklist[65536];

extern const char ROLE_SOURCE;
extern const char ROLE_GOAL;
extern char TYPE_CONTENT;
extern char TYPE_OK;
extern char TYPE_NEIGHBOUR;

// checks if a certain node is already registered as a neighbour
//      returns 1 if my_node is a neighbour
//      returns 0 if my_node is not a neighbour
int is_neighbour (struct node *my_node) {
    struct node *neighbour_item = malloc(sizeof(struct node));

    // lock neighbours list
    pthread_mutex_lock(&mutex_neighbours);

    // loop through existing neighbours
    LIST_FOREACH(neighbour_item, &neighbour_head, entries) {
        if (my_node->port == neighbour_item->port) {
            // unlock neighbours list
            pthread_mutex_unlock(&mutex_neighbours);
            return 1;
        }
    }

    // unlock neighbours list
    pthread_mutex_unlock(&mutex_neighbours);

    return 0;
}

// add a neighbour to this node
//      returns 1 if neighbour was added
//      returns 0 if neighbour was already registered
int add_neighbour(struct node *neighbour_to_add) {
    int already_in_list = is_neighbour(neighbour_to_add);
    int neighbour_added;
    char dbg_message[100];

    if (!already_in_list) {
        pthread_mutex_lock(&mutex_neighbours);// lock neighbours list
        LIST_INSERT_HEAD(&neighbour_head, neighbour_to_add, entries);// add new neighbour
        pthread_mutex_unlock(&mutex_neighbours);// unlock neighbours list

        sprintf(dbg_message,
                "Nachbar mit Port %d hinzugefügt",
                neighbour_to_add->port);
        neighbour_added = 1;
    } else {
        sprintf(dbg_message,
                "Nachbar mit Port %d ist schon registriert. Unternehme nichts",
                neighbour_to_add->port);
        neighbour_added = 0;
    }

    dbg(dbg_message);

    return neighbour_added;
}

// update the routing table with the information contained in a package
void update_routing_table(package *my_package) {
    struct node *sender_node = malloc(sizeof(struct node));

    package_message_to_node(my_package, sender_node, 122);

    // lock router
    pthread_mutex_lock(&mutex_router);

    // only add routes to known neighbours
    if (is_neighbour(sender_node)) {
        if (my_package->target == 0) {
            // package is for source (and therefore comes from goal)
            if(my_router->goal_neighbour == 0) {
                my_router->goal_neighbour = sender_node->port;
            }
        } else {
            // package is for goal (and therefore comes from source)
            if(my_router->source_neighbour == 0) {
                my_router->source_neighbour = sender_node->port;
            }
        }
    }

    // unlock router
    pthread_mutex_unlock(&mutex_router);
}

// process a connection package
int process_connection_package(package *my_package) {
    int ip_num;
    int port_num;
    int neighbour_added;
    struct node *new_neighbour = malloc(sizeof(struct node));
    struct node *this_node = malloc(sizeof(struct node));

    package_message_to_node(my_package, new_neighbour, 0);
    neighbour_added = add_neighbour(new_neighbour);

    if (neighbour_added) {
        // Send connection package to neighbour, since mesh connections are bidirectional
        this_node->port = port;
        node_to_package_message(this_node, my_package, 0);
        send_package(my_package, new_neighbour->port);
    }
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

// send a package
int send_package(package *my_package, int receiver_port) {
    int sockfd;
    FILE *write_stream;
    char dbg_message[100];
    struct node *this_node = malloc(sizeof(struct node));

    // write information about current node into the package so the neighbour will know where it came from
    this_node->port = port;
    node_to_package_message(this_node, my_package, 122);

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
        perror("ERROR, Paket kann nicht in Stream umgewandelt werden\n");
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

    // make sure we forward a package with a certain id only once
    // we make an exception for OK-packages, since they have the same id as their corresponding data package
    if (my_package->type != TYPE_OK) {

        pthread_mutex_lock(&mutex_blacklist);// lock id blacklist

        if (package_id_blacklist[my_package->id % 256] == my_package->id) {
            pthread_mutex_unlock(&mutex_blacklist);// unlock id blacklist
            dbg("Dieses Paket habe ich bereits einmal weitergeleitet. Verwerfe es");
            return -1;
        }

        // we hash the package id by only considering the 8 lowest bits
        package_id_blacklist[my_package->id % 256] = my_package->id;
        pthread_mutex_unlock(&mutex_blacklist);// unlock id blacklist
    }

    // check if we know to what neighbour to forward the package
    if (my_package->target == 0) {
        // the package needs to go to the source
        if (my_router->source_neighbour != 0) {
            dbg("Leite Paket gezielt weiter");
            send_package(my_package, my_router->source_neighbour);
            return 0;
        }
    } else {
        // the package needs to go to the goal
        if (my_router->goal_neighbour != 0) {
            dbg("Leite Paket gezielt weiter");
            send_package(my_package, my_router->goal_neighbour);
            return 0;
        }
    }

    // If we don't know to what neighbour to forward the package, we flood the network
    // TODO: we could still be a bit smarter here and not send packages to nodes that are registered in the router
    dbg("Leite Paket weiter indem ich Netzwerk flute");
    pthread_mutex_lock(&mutex_neighbours);// lock neighbours list

    // loop through existing neighbours
    LIST_FOREACH(neighbour_item, &neighbour_head, entries) {
        send_package(my_package, neighbour_item->port);
    }

    pthread_mutex_unlock(&mutex_neighbours);// unlock neighbours list

    return 0;
}

// process a data package
int process_data_package(package *my_package) {
    struct node *sender_node = malloc(sizeof(struct node));// the node that sent this package

    package_message_to_node(my_package, sender_node, 122);

    if (my_package->target == 1 && role == ROLE_GOAL ||
        my_package->target == 0 && role == ROLE_SOURCE) {

        int i;
        int message_length = 128;
        char the_message[message_length];

        memcpy(&the_message, &(my_package->message), 122);

        dbg("Nachricht erhalten. Sende OK-Paket");

        // for some reason we need to do a padding on the message, since the testscript seems to eat away some chars on
        // every output to standard out
        fprintf(
            stdout,
            "%s----------------------------------------------------------------------------------------------------------",
            the_message);
        fflush(stdout);

        // the new target is the other end of the network
        if (my_package->target == 1) {
            my_package->target = 0;
        } else {
            my_package->target = 1;
        }

        // send ok-package back
        my_package->type = TYPE_OK;
        send_package(my_package, sender_node->port);
    } else {
        forward_package(my_package);
    }
}

// process an OK package
int process_ok_package(package *my_package) {
    if (my_package->target == 1 && role == ROLE_GOAL ||
        my_package->target == 0 && role == ROLE_SOURCE) {
        dbg("OK-Paket hat ursprünglichen Sender erreicht. Alles ist gut :)");
    } else {
        forward_package(my_package);
    }
}

// process a message
int process_package(package *my_package) {
    if (my_package->type == TYPE_CONTENT){
        dbg("Datenpaket erhalten");
        update_routing_table(my_package);
        process_data_package(my_package);
    } else if (my_package->type == TYPE_OK) {
        dbg("OK-Paket erhalten");
        update_routing_table(my_package);
        process_ok_package(my_package);
    } else if (my_package->type == TYPE_NEIGHBOUR) {
        dbg("Verbindungspaket erhalten");
        process_connection_package(my_package);
    } else {
        dbg("Paket mit unbekanntem Typ erhalten");
        fprintf(stderr, "Typ:%c\n", my_package->type);
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
        perror("ERROR, Ungültiges Paket erhalten.");
    }

    // close stream
    fclose(read_stream);

    // process package
    process_package(&current_package);
}

void *worker_init(void *sockfd_ptr)
{
    int sockfd = (int)sockfd_ptr;
    //dbg("TCP-Verbindung hergestellt. Warte auf Paket...");
    parse_message(sockfd);
    pthread_exit(NULL);
}