
int open_client_socket(char * host, int port);
int sendCommand(char * host, int port, char * command, char * user,
		char * password, char * args, char * response);
void printUsage();
void add_user();
void enter_room();
void leave_room();
void get_messages();
void send_message(char * msg);
void print_users_in_room();
void print_users();
void printPrompt();
void printHelp();
void * getMessagesThread(void * arg);
void startGetMessageThread();
void create_room();
void login();
