#include<stdio.h>
#include<string.h>
#include<include/util.h>

extern int port;
extern int role;

void dbg (char* msg) {
    fprintf(stderr, "Knoten \t%d \t%c:\t%s\n", port, role, msg);
}

int package_to_stream(package *my_package, FILE *stream) {
    int num_items_written;
    short id;

    // package id
    id = htons(my_package->id);
    num_items_written = fwrite(&id, 2, 1, stream);

    if (num_items_written <= 0) {
        perror("ERROR, Ungültiges Packet.");
        return -1;
    }

    // target of the package
    num_items_written = fwrite(&my_package->target, 1, 1, stream);
    if (num_items_written <= 0) {
        return -1;
        perror("ERROR, Ungültiges Packet.");
    }

    // package type
    num_items_written = fwrite(&my_package->type, 1, 1, stream);
    if (num_items_written <= 0) {
		return -1;
        perror("ERROR, Ungültiges Packet.");
    }

    // message of the package
    num_items_written = fwrite(&my_package->message, 128, 1, stream);
    if (num_items_written <= 0) {
		return -1;
        perror("ERROR, Ungültiges Packet.");
    }

    return 0;
}

int stream_to_package(FILE *stream, package *current_package) {
    int num_items_read;
    short id;

    // package id
    num_items_read = fread(&id, 2, 1, stream);
    current_package->id = ntohs(id);

    if (num_items_read <= 0) {
		return -1;
        perror("ERROR, Ungültiges Packet.");
    }

    // target of the package
    num_items_read = fread(&current_package->target, 1, 1, stream);
    if (num_items_read <= 0) {
		return -1;
        perror("ERROR, Ungültiges Packet.");
    }

    // package type
    num_items_read = fread(&current_package->type, 1, 1, stream);
    if (num_items_read <= 0) {
		return -1;
        perror("ERROR, Ungültiges Packet.");
    }

    // message of the package
    num_items_read = fread(&current_package->message, 128, 1, stream);
    if (num_items_read <= 0) {
		return -1;
        perror("ERROR, Ungültiges Packet.");
    }

    return 0;
}

int package_message_to_node(package *my_package, struct node *new_node) {
    int ip_num;
    int port_num;

    memcpy(&ip_num, &(my_package->message[3]), 1);// not sure if everything's correct here
    memcpy(&port_num, &(my_package->message[4]), 2);

    new_node->ip = ntohs(ip_num);
    new_node->port = ntohs(port_num);
}

int node_to_package_message(struct node *my_node, package *my_package) {
    int ip_num = htons(my_node->ip);
    int port_num = htons(my_node->port);

    memcpy(&(my_package->message[3]), &ip_num, 1);
    memcpy(&(my_package->message[4]), &port_num, 2);

    // TODO: das überschreibt einen teil der "echten" message bei datenpaketen -> echte message um 6 byte shiften?
}
