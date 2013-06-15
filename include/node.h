int create_node(int port);
void wait_for_connection(int sockfd);
int wait_for_message(int sockfd);

void kill(int port);
void rm_client(int port);