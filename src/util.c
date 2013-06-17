#include<stdio.h>
#include<include/util.h>

extern int port;
extern int role;

void dbg (char* msg) {
    printf("Knoten \t%d \t%c:\t%s\n", port, role, msg);
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
