#include "server.h"
Server::Server(const Node& serv)
{
	this -> serv = serv;
}

int Server::Listen()
{
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	std::cout << "Listening port: " << serv.port << std::endl;

	if ((rv = getaddrinfo(NULL, serv.port.c_str(), &hints, &servinfo)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		error_num = 1;
		exit(1);
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
		//printf("server: got connection from %s\n", s);

		if (!fork()) 
		{ // this is the child process
            memset(buffer, 0, 256);
            int read_rtn = read(newsockfd, buffer, 255);
			if (read_rtn >= 0)
			{
				ProcessMessage(buffer);
			}

           // if (read_rtn < 0)
           // {
           //     error("ERROR reading from socket");
           // }
		   
			// Need some way to process message
			// Have it used by client connection

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

void Server::ProcessMessage(const char* buffer)
{
	printf("%s\n", buffer);
	// Setup next set of messages
	// Need to read message
	// Lets just have it be space delimeted for now
	// %d %d %d
	std::string b(buffer);
	std::cout << b << std::endl; 
	std::istringstream iss(b);
	std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},std::istream_iterator<std::string>{}};
	int src_node_id = std::stoi(tokens[0]);
	int dest_node_id = std::stoi(tokens[1]);
	int hop_number = std::stoi(tokens[2]);
	printf("%d %d %d\n", src_node_id, dest_node_id, hop_number);
	//Node n = p1.node_map[node_id_process];
	
	for (const auto& one_hop: serv.one_hop_neighbors)
	{
		Client c1(serv, one_hop);
		std::cout << "one hop " << one_hop.node_id << std::endl;
		c1.Message(hop_number + 1);
	}
}

void *Server::get_in_addr(struct sockaddr *sa)
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

