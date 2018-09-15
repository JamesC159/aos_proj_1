#include "node.h"
#include "parser.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>

//sockets headers
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

//#define PORT "3490" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 

// server headers
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

#define BACKLOG 10	 // how many pending connections queue will hold

// Break the problem into pieces

// After setting up the topology you need a distributed algorithm that finds the k hop neights (communication thorugh sockets and 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void sigchld_handler(int s)
{
	(void)s; // quiet unused variable warning

	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

int main(int argc, char** argv)
{
	if(argc != 3)
	{
		std::cerr << "usage: ./main config node_id" << std::endl; 
	 	return -1;
	}
	
	int node_id_process = std::stoi(argv[2]);
	std::cout << "node_id_process " << node_id_process << std::endl;

	Parser p1(argv[1]);
	p1.Parse_Config();

	for (const auto& n: p1.node_map)
	{
		std::cout << n.second << std::endl;
	}
	// NEED TO SET PORT
	//p1.node_map[node_id_process].port;
	
	//Server
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, p1.node_map[node_id_process].port.c_str(), &hints, &servinfo)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
		{
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) 
		{
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
		{
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  
	{
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) 
	{
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) 
	{
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while(1) 
	{  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) 
		{
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		printf("server: got connection from %s\n", s);

		if (!fork()) 
		{ // this is the child process
			close(sockfd); // child doesn't need the listener
			if (send(new_fd, "Hello, world!", 13, 0) == -1)
			{
				perror("send");
			}
			close(new_fd);
			exit(0);
		}
		close(new_fd);  // parent doesn't need this
		// END Server
	}
//	// Sockets
//	
//	int sockfd, numbytes;  
//	char buf[MAXDATASIZE];
//	struct addrinfo hints, *servinfo, *p;
//	int rv;
//	char s[INET6_ADDRSTRLEN];
//
//	//if (argc != 2) {
//	//    fprintf(stderr,"usage: client hostname\n");
//	//    exit(1);
//	//}
//	
//
//	memset(&hints, 0, sizeof hints);
//	hints.ai_family = AF_UNSPEC;
//	hints.ai_socktype = SOCK_STREAM;
//
//	if ((rv = getaddrinfo(argv[1], port, &hints, &servinfo)) != 0) 
//	{
//		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
//		return 1;
//	}
//
//	// loop through all the results and connect to the first we can
//	for(p = servinfo; p != NULL; p = p->ai_next) 
//	{
//		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
//		{
//			perror("client: socket");
//			continue;
//		}
//
//		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
//		{
//			perror("client: connect");
//			close(sockfd);
//			continue;
//		}
//
//		break;
//	}
//
//	if (p == NULL) 
//	{
//		fprintf(stderr, "client: failed to connect\n");
//		return 2;
//	}
//
//	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
//	printf("client: connecting to %s\n", s);
//
//	freeaddrinfo(servinfo); // all done with this structure
//
//	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) 
//	{
//	    perror("recv");
//	    exit(1);
//	}
//
//	buf[numbytes] = '\0';
//
//	printf("client: received '%s'\n",buf);
//
//	close(sockfd);
}


