typedef struct {
    short package_id;
    char target;
    char type;
    char message[128];
} package;

void dbg (char* msg);
int stream_to_package(FILE *stream, package *current_package);
