#include "node.h"
#include "client.h"
#include "parser.h"

#include <algorithm>
#include <cctype> 
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <thread>
#include <vector>

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

#define BACKLOG 100	 // how many pending connections queue will hold

// Break the problem into pieces

// After setting up the topology you need a distributed algorithm that finds the k hop neights (communication thorugh sockets and 

// get sockaddr, IPv4 or IPv6:
//void *get_in_addr(struct sockaddr *sa)
//{
//	if (sa->sa_family == AF_INET) {
//		return &(((struct sockaddr_in*)sa)->sin_addr);
//	}
//
//	return &(((struct sockaddr_in6*)sa)->sin6_addr);
//}
//
void sigchld_handler(int s)
{
	(void)s; // quiet unused variable warning

	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

int Server(std::string port);

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

	// I think you need this to be a multithreaded program
	// You want the server running at the same time the client is going
	// The server is going to be stuck in that loop and so the client code won't be reached
	// Server
	std::thread t1(Server, p1.node_map[node_id_process].port);

	// For one hop neighbors
//	// Send message to one hop neighbors 

	// Test with disocvering node 0 neighbors first then expand this logic
	if (node_id_process == 0)
	{
		Node n = p1.node_map[node_id_process];
		for (const auto& one_hop: n.one_hop_neighbors)
		{
			Client c1(p1.node_map[node_id_process], p1.node_map[one_hop]);
			std::cout << "one hop " << one_hop << std::endl;
			c1.Message(1);
			//c1.Close();
			//std::thread t2(Client, p1.node_map[one_hop]);
			//t2.detach();
		}
	}
	
	t1.join();
	//t1.detach();
}

// What are the arguments?
// Port
int Server(std::string port)
{
	//Server
	int sockfd, newsockfd;  // listen on sock_fd, new connection on newsockfd
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

	//std::cout << "Listening port: " << p1.node_map[node_id_process].port << std::endl;
	std::cout << "Listening port: " << port << std::endl;

	if ((rv = getaddrinfo(NULL, port.c_str(), &hints, &servinfo)) != 0) 
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

	char buffer[256];

	while(1) 
	{  // main accept() loop
		sin_size = sizeof their_addr;
		newsockfd= accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (newsockfd == -1) 
		{
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		printf("server: got connection from %s\n", s);

		if (!fork()) 
		{ // this is the child process
            memset(buffer, 0, 256);
            int read_rtn = read(newsockfd, buffer, 255);
           // if (read_rtn < 0)
           // {
           //     error("ERROR reading from socket");
           // }

            printf("Message: %s\n", buffer);
			close(sockfd); // child doesn't need the listener
			//if (send(newsockfd, "hello, world!", 13, 0) == -1)
			//{
			//	perror("send");
			//}
			close(newsockfd);
			exit(0);
		}
		close(newsockfd);  // parent doesn't need this
	}  // end server
}

