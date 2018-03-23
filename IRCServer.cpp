
const char * usage =
"                                                               \n"
"IRCServer:                                                   \n"
"                                                               \n"
"Simple server program used to communicate multiple users       \n"
"                                                               \n"
"To use it in one window type:                                  \n"
"                                                               \n"
"   IRCServer <port>                                          \n"
"                                                               \n"
"Where 1024 < port < 65536.                                     \n"
"                                                               \n"
"In another window type:                                        \n"
"                                                               \n"
"   telnet <host> <port>                                        \n"
"                                                               \n"
"where <host> is the name of the machine where talk-server      \n"
"is running. <port> is the port number you used when you run    \n"
"daytime-server.                                                \n"
"                                                               \n";

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include <string>
#include <iostream>
#include <fstream>
#include <bits/stdc++.h>
#include <vector>
#include <algorithm>
using namespace std;

#include "IRCServer.h"

int QueueLength = 5;


//test

int
IRCServer::open_server_socket(int port) {

	// Set the IP address and port for this server
	struct sockaddr_in serverIPAddress; 
	memset( &serverIPAddress, 0, sizeof(serverIPAddress) );
	serverIPAddress.sin_family = AF_INET;
	serverIPAddress.sin_addr.s_addr = INADDR_ANY;
	serverIPAddress.sin_port = htons((u_short) port);
  
	// Allocate a socket
	int masterSocket =  socket(PF_INET, SOCK_STREAM, 0);
	if ( masterSocket < 0) {
		perror("socket");
		exit( -1 );
	}

	// Set socket options to reuse port. Otherwise we will
	// have to wait about 2 minutes before reusing the sae port number
	int optval = 1; 
	int err = setsockopt(masterSocket, SOL_SOCKET, SO_REUSEADDR, 
			     (char *) &optval, sizeof( int ) );
	
	// Bind the socket to the IP address and port
	int error = bind( masterSocket,
			  (struct sockaddr *)&serverIPAddress,
			  sizeof(serverIPAddress) );
	if ( error ) {
		perror("bind");
		exit( -1 );
	}
	
	// Put socket in listening mode and set the 
	// size of the queue of unprocessed connections
	error = listen( masterSocket, QueueLength);
	if ( error ) {
		perror("listen");
		exit( -1 );
	}

	return masterSocket;
}

void
IRCServer::runServer(int port)
{
	int masterSocket = open_server_socket(port);

	initialize();
	
	while ( 1 ) {
		
		// Accept incoming connections
		struct sockaddr_in clientIPAddress;
		int alen = sizeof( clientIPAddress );
		int slaveSocket = accept( masterSocket,
					  (struct sockaddr *)&clientIPAddress,
					  (socklen_t*)&alen);
		
		if ( slaveSocket < 0 ) {
			perror( "accept" );
			exit( -1 );
		}
		
		// Process request.
		processRequest( slaveSocket );		
	}
}

int
main( int argc, char ** argv )
{
	// Print usage if not enough arguments
	if ( argc < 2 ) {
		fprintf( stderr, "%s", usage );
		exit( -1 );
	}
	
	// Get the port from the arguments
	int port = atoi( argv[1] );

	IRCServer ircServer;

	// It will never return
	ircServer.runServer(port);
	
}

//
// Commands:
//   Commands are started y the client.
//
//   Request: ADD-USER <USER> <PASSWD>\r\n
//   Answer: OK\r\n or DENIED\r\n
//
//   REQUEST: GET-ALL-USERS <USER> <PASSWD>\r\n
//   Answer: USER1\r\n
//            USER2\r\n
//            ...
//            \r\n
//
//   REQUEST: CREATE-ROOM <USER> <PASSWD> <ROOM>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: LIST-ROOMS <USER> <PASSWD>\r\n
//   Answer: room1\r\n
//           room2\r\n
//           ...
//           \r\n
//
//   Request: ENTER-ROOM <USER> <PASSWD> <ROOM>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: LEAVE-ROOM <USER> <PASSWD>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: SEND-MESSAGE <USER> <PASSWD> <MESSAGE> <ROOM>\n
//   Answer: OK\n or DENIED\n
//
//   Request: GET-MESSAGES <USER> <PASSWD> <LAST-MESSAGE-NUM> <ROOM>\r\n
//   Answer: MSGNUM1 USER1 MESSAGE1\r\n
//           MSGNUM2 USER2 MESSAGE2\r\n
//           MSGNUM3 USER2 MESSAGE2\r\n
//           ...\r\n
//           \r\n
//
//    REQUEST: GET-USERS-IN-ROOM <USER> <PASSWD> <ROOM>\r\n
//    Answer: USER1\r\n
//            USER2\r\n
//            ...
//            \r\n
//

void
IRCServer::processRequest( int fd )
{
	// Buffer used to store the comand received from the client
	const int MaxCommandLine = 1024;
	char commandLine[ MaxCommandLine + 1 ];
	int commandLineLength = 0;
	int n;
	
	// Currently character read
	unsigned char prevChar = 0;
	unsigned char newChar = 0;
	
	//
	// The client should send COMMAND-LINE\n
	// Read the name of the client character by character until a
	// \n is found.
	//

	// Read character by character until a \n is found or the command string is full.
	while ( commandLineLength < MaxCommandLine &&
		read( fd, &newChar, 1) > 0 ) {

		if (newChar == '\n' && prevChar == '\r') {
			break;
		}
		
		commandLine[ commandLineLength ] = newChar;
		commandLineLength++;

		prevChar = newChar;
	}
	
	// Add null character at the end of the string
	// Eliminate last \r
	commandLineLength--;
        commandLine[ commandLineLength ] = 0;

	printf("RECEIVED: %s\n", commandLine);

	printf("The commandLine has the following format:\n");
	printf("COMMAND <user> <password> <arguments>. See below.\n");
	printf("You need to separate the commandLine into those components\n");
	printf("For now, command, user, and password are hardwired.\n");


	char command[100];
	char user[100];
	char password[100];
	char args[100];
	fill_n(command, 100, 0);
        fill_n(user, 100, 0);
        fill_n(password, 100, 0);
        fill_n(args, 100, 0);

	int ct = 0;
	int comc = 0;
	int pos = 1;
	while (commandLine[comc] != 0) {
		if (commandLine[comc] == ' ' && pos != 4) {
			if (pos == 1) {
				command[ct] = 0;
			}
			if (pos == 2) {
				user[ct] = 0;
			}
			if (pos == 3) {
				password[ct] = 0;
			}
			ct = 0;
			comc++;
			pos++;
			continue;
		}
		if (pos == 1) {
			command[ct] = commandLine[comc];
			ct++;
			comc++;
		}
		if (pos == 2) {
			user[ct] = commandLine[comc];
			ct++;
			comc++;
		}
		if (pos == 3) {
			password[ct] = commandLine[comc];
			ct++;
			comc++;
		}
		if (pos == 4) {
			args[ct] = commandLine[comc];
			ct++;
			comc++;
		}
	}
	if (ct != 0) {
		args[ct] = 0;
	}
	

	printf("command=%s\n", command);
	printf("user=%s\n", user);
	printf( "password=%s\n", password );
	printf("args=%s\n", args);

	if (!strcmp(command, "ADD-USER")) {
		addUser(fd, user, password, args);
	}
	else if (!strcmp(command, "ENTER-ROOM")) {
		enterRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "LEAVE-ROOM")) {
		leaveRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "SEND-MESSAGE")) {
		sendMessage(fd, user, password, args);
	}
	else if (!strcmp(command, "GET-MESSAGES")) {
		getMessages(fd, user, password, args);
	}
	else if (!strcmp(command, "GET-USERS-IN-ROOM")) {
		getUsersInRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "GET-ALL-USERS")) {
		getAllUsers(fd, user, password, args);
	}
	else if (!strcmp(command, "CREATE-ROOM")) {
		createRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "LIST-ROOMS")) {
		Listroom(fd, user, password, args);
	}
	else if (!strcmp(command, "LOGIN")) {
		login(fd, user, password, args);
	}
	else {
		const char * msg =  "UNKNOWN COMMAND\r\n";
		write(fd, msg, strlen(msg));
	}

	// Send OK answer
	//const char * msg =  "OK\n";
	//write(fd, msg, strlen(msg));

	close(fd);	
}


vector <string> users;
int userindex = 0;
vector <string> passs;
int passindex = 0;
struct room {
	vector <string> users;
	int userin;
	vector <string> messs;
	vector <string> whomes;
	int whomesi;
	int mesnum;
	char * name;
};
int roomnum = 0;
struct room rooms[100];

void
IRCServer::initialize()
{
	// Open password file
	string line;
	ifstream myfile("password.txt");
	
	int isu = 1;
	int isp = 0;
	if (myfile.is_open()) {
		while (getline(myfile, line)) {
			if (isu) {
				users.push_back(line);
				isu = 0;
				isp = 1;
				userindex++;
				continue;
			}
			if (isp) {
				passs.push_back(line);
				isp = 0;
				passindex++;
				continue;
			}
			
			if (isu == 0 && isp == 0) {
				isu = 1;
				continue;
			}
		}
	}
	// Initialize users in room

	struct room rooms[100];
	roomnum = 0;
	// Initalize message list

}

bool checkPassword(int fd, const char * user, const char * password);

void IRCServer::login(int fd, const char * user, const char * password, const char * args) {
	if (!checkPassword(fd, user, password)) {
		const char * mas = "D\r\n";
		write(fd, mas, strlen(mas));
		return;
	}
	const char * o = "OK\r\n";
	write(fd, o, strlen(o));
	return;
}

void Listroom(int fd, const char * user, const char * password, const char * args);
void
IRCServer::Listroom(int fd, const char * user, const char * password, const char * args) {

	if (!checkPassword(fd, user, password)) {
		const char * mas = "ERROR (Wrong password)\r\n";
		write(fd, mas, strlen(mas));
		return;
	}
	if(roomnum == 0) {
		const char * m = "D\r\n";
		write(fd, m, strlen(m));
		return;
	}
	for (int i = 0; i < roomnum; i++) {
		write(fd, rooms[i].name, strlen(rooms[i].name));
		/*if (i == (roomnum - 1)) {
			const char * n = "\r\n";
			write(fd, n, strlen(n));
			return;
		}*/
		const char * m = "\r\n";
		write(fd, m, strlen(m));
	}
	return;
}


bool
IRCServer::checkPassword(int fd, const char * user, const char * password) {
	// Here check the password
	for (int i = 0; i < users.size(); i++) {
		if (!strcmp(user, users[i].c_str())) {
			if (!strcmp(password, passs[i].c_str())) {
				return true;
			}
		}
	}
	return false;
}



void
IRCServer::addUser(int fd, const char * user, const char * password, const char * args)
{
	// Here add a new user. For now always return OK.

	for (int i = 0; i < users.size(); i++) {
		if (!strcmp(user, users[i].c_str())) {
			const char * msg = "DENIED\r\n";
			write(fd, msg, strlen(msg));
			return;
		}
	}
	
	ofstream mf;
	mf.open("password.txt", ios_base::app);
	mf << user;
	mf << "\n";
	mf << password;
	mf << "\n\n";
	mf.close();
	users.push_back(user);
	userindex++;
	passs.push_back(password);
	passindex++;
	const char * m = "OK\r\n";
	write(fd, m, strlen(m));
	return;		
}

void
IRCServer::enterRoom(int fd, const char * user, const char * password, const char * args)
{
	if (!checkPassword(fd, user, password)) {
		const char * mas = "ERROR (Wrong password)\r\n";
		write(fd, mas, strlen(mas));
		return;
	}

	for (int i = 0; i < roomnum; i++) {
		if (!strcmp(rooms[i].name, args)) {
			for (int j = 0; j < rooms[i].users.size(); j++) {
				if (!strcmp(rooms[i].users[j].c_str(), user)) {
					const char * m = "OK\r\n";
					write(fd, m, strlen(m));
					return;
				}
			}
			rooms[i].users.push_back(user);
			rooms[i].userin++;
			const char * m = "OK\r\n";
			write(fd, m, strlen(m));
			return;
		}
	}
	const char * mas = "ERROR (No room)\r\n";
	write(fd, mas, strlen(mas));
	return;
}

void
IRCServer::leaveRoom(int fd, const char * user, const char * password, const char * args)
{
	if (!checkPassword(fd, user, password)) {
		const char * mas = "ERROR (Wrong password)\r\n";
		write(fd, mas, strlen(mas));
		return;
	}
	
	//int tf = 0;

	for (int i = 0; i < roomnum; i++) {
		if (!strcmp(rooms[i].name, args)) {
			for (int j = 0; j < rooms[i].users.size(); j++) {
				if (!strcmp(rooms[i].users[j].c_str(), user)) {
					rooms[i].users.erase(rooms[i].users.begin() + j);
					rooms[i].userin--;
					const char * m = "OK\r\n";
					write(fd, m, strlen(m));
					return;
				}
			}
		}
	}
	const char * mes = "ERROR (No user in room)\r\n";
	write(fd, mes, strlen(mes));
}

void
IRCServer::sendMessage(int fd, const char * user, const char * password, const char * args)
{
	if (!checkPassword(fd, user, password)) {
		const char * mas = "ERROR (Wrong password)\r\n";
		write(fd, mas, strlen(mas));
		return;
	}
	
	char ro[100];
	fill_n(ro, 100, 0);
	char me[100];
	fill_n(me, 100, 0);
	int ct = 0;
	int pos = 1;
	int car = 0;
	while (args[car] != 0) {
		if (args[car] == ' ' && pos != 2) {
			ro[ct] = 0;
			pos++;
			car++;
			ct = 0;
			continue;
		}
		if (pos == 1) {
			ro[ct] = args[car];
			ct++;
			car++;
		}
		if (pos == 2) {
			me[ct] = args[car];
			ct++;
			car++;
		}
		
	}
	me[ct] = 0;
	
	
	for (int i = 0; i < roomnum; i++) {
		if (!strcmp(ro, rooms[i].name)) {
			for (int j = 0; j < rooms[i].users.size(); j++) {
				if (!strcmp(rooms[i].users[j].c_str(), user)) {
					rooms[i].messs.push_back(me);
					rooms[i].mesnum++;
					rooms[i].whomes.push_back(user);
					rooms[i].whomesi++;
					const char * m = "OK\r\n";
					write(fd, m, strlen(m));
					return;
				}
			}
		}
	}
	const char * mas = "ERROR (user not in room)\r\n";
	write(fd, mas, strlen(mas));
	return;
}

void
IRCServer::getMessages(int fd, const char * user, const char * password, const char * args)
{
	if (!checkPassword(fd, user, password)) {
		const char * mas = "ERROR (Wrong password)\r\n";
		write(fd, mas, strlen(mas));
		return;
	}

	char ro[100];
	fill_n(ro, 100, 0);
	char me[100];
	fill_n(me, 100, 0);
	int ct = 0;
	int pos = 1;
	int car = 0;
	while (args[car] != 0) {
		if (args[car] == ' ' && pos != 2) {
			ro[ct] = 0;
			pos++;
			car++;
			ct = 0;
			continue;
		}
		if (pos == 1) {
			ro[ct] = args[car];
			ct++;
			car++;
		}
		if (pos == 2) {
			me[ct] = args[car];
			ct++;
			car++;
		}
		
	}
	me[ct] = 0;	
	
	int h = atoi(ro);
	for (int i = 0; i < roomnum; i++) {
		if (!strcmp(me, rooms[i].name)) {
			for (int j = 0; j < rooms[i].users.size(); j++) {
				if (!strcmp(rooms[i].users[j].c_str(), user)) {
					int c = 0;
					int hasm = 1;
					for (int k = h + 1; k < rooms[i].messs.size(); k++) {
						hasm = 0;
						stringstream s;
						s << k;
						string nu = s.str();
						string w = rooms[i].whomes[k];
						string m = rooms[i].messs[k];
						string t = nu + " " + w + " " + m + "\r\n";
						char * nu1 = new char[100];
						strcpy(nu1, t.c_str());
						write(fd, nu1, strlen(nu1));
					}
					if (hasm) {
						const char * im = "NO-NEW-MESSAGES";
						write(fd, im, strlen(im));
					}
					write(fd, "\r\n", 2);
					return;
				}
			}
		}
	}

	const char * mas = "ERROR (User not in room)\r\n";
	write(fd, mas, strlen(mas));
	return;
}

void
IRCServer::getUsersInRoom(int fd, const char * user, const char * password, const char * args)
{
	if (!checkPassword(fd, user, password)) {
		const char * mas = "ERROR (Wrong password)\r\n";
		write(fd, mas, strlen(mas));
		return;
	}
	for (int i = 0; i < roomnum; i++) {
		if (!strcmp(rooms[i].name, args)) {
			vector <string> take = rooms[i].users;
			sort(take.begin(), take.end());
			for (int j = 0; j < take.size(); j++) {
				char * u = new char[100];
				strcpy(u, take[j].c_str());
				int l = strlen(u);
				u[l] = '\r';
				l++;
				u[l] = '\n';
				l++;
				u[l] = '\0';
				write(fd, u, strlen(u));
			}
		}
	}
	const char * m = "\r\n";
	write(fd, m, strlen(m));
	return;
}

void
IRCServer::getAllUsers(int fd, const char * user, const char * password,const  char * args)
{
	if (!checkPassword(fd, user, password)) {
		const char * mas = "ERROR (Wrong password)\r\n";
		write(fd, mas, strlen(mas));
		return;
	}
	vector <string> take = users;
	sort(take.begin(), take.end());
	for (int i = 0; i < take.size(); i++) {
		char * u = new char[100];
		strcpy(u, take[i].c_str());
		int l = strlen(u);
		u[l] = '\r';
		l++;
		u[l] = '\n';
		l++;
		u[l] = '\0';
		write(fd, u, strlen(u));
	}
	const char * m = "\r\n";
	write(fd, m, strlen(m));
	return;
}

void IRCServer::createRoom(int fd, const char * user, const char * password, const char * args) {
	if (!checkPassword(fd, user, password)) {
		const char * mas = "ERROR (Wrong password)\r\n";
		write(fd, mas, strlen(mas));
		return;
	}
	for (int i = 0; i < roomnum; i++) {
		if (!strcmp(rooms[i].name, args)) {
			const char * mas = "DENIED\r\n";
			write(fd, mas, strlen(mas));
			return;
		}
	}
	rooms[roomnum].name = new char[100];
	rooms[roomnum].whomesi = 0;
	rooms[roomnum].mesnum = 0;
	strcpy(rooms[roomnum].name, args);
	//rooms[roomnum].users.push_back(user);
	//rooms[roomnum].userin++;
	roomnum++;
	const char * m = "OK\r\n";
	write(fd, m, strlen(m));
	return;
}


