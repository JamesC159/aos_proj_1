#ifndef SERVER_H
#define SERVER_H

#include "node.h"
#include "client.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include <string>
#include <cstddef>
#include <fstream>
#include <unordered_map>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>

#define BACKLOG 100	 // how many pending connections queue will hold
#define MAXDATASIZE 100 // max number of bytes we can get at once 

class Server
{
	public:
		int sockfd, newsockfd;  // listen on sock_fd, new connection on newsockfd
		struct addrinfo hints, *servinfo, *p;
		struct sockaddr_storage their_addr; // connector's address information
		socklen_t sin_size;
		struct sigaction sa;
		int yes=1;
		char s[INET6_ADDRSTRLEN];
		int rv;
		Node serv;
		int error_num;
		static int num_nodes;
		Server(const Node& serv);
		int Listen();
		void ProcessMessage(const char* buffer);
		void *get_in_addr(struct sockaddr *sa);
};

void sigchld_handler(int s);

#endif // SERVER_H
