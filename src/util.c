#include <stdio.h>

extern int port;

void dbg (char* msg) {
    // TODO: auch Z und Q ausgeben
    printf("Knoten \t%d:\t%s\n", port, msg);
}

int stream_to_package(FILE *stream) {
    short package_id;
    int num_items_read;
    char target;
    char type;
    char message[128];
    char dbg_str[80];

    // package id
    num_items_read = fread(&package_id, 2, 1, stream);
    if (num_items_read <= 0) {
		return -1;
    }

    // target of the package
    num_items_read = fread(&target, 1, 1, stream);
    if (num_items_read <= 0) {
		return -1;
    }

    // package type
    num_items_read = fread(&type, 1, 1, stream);
    if (num_items_read <= 0) {
		return -1;
    }

    // message of the package
    num_items_read = fread(&message, 128, 1, stream);
    if (num_items_read <= 0) {
		return -1;
    }

    sprintf(dbg_str, "Received package with type %c", type);
    dbg(dbg_str);
}
