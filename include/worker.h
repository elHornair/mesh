void *worker_init(void *sockfd_ptr);

int process_package(package *my_package);
int process_connection_package(package *my_package);
int process_data_package(package *my_package);
int process_ok_package(package *my_package);

int open_connection(int receiver_port);
int send_package(package *my_package, int receiver_port);
int forward_package(package *my_package);

int is_neighbour (struct node *my_node);
int add_neighbour(struct node *neighbour_to_add);

int parse_message(int sockfd);
void update_routing_table(package *my_package);