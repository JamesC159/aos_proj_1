#include "server.h"
// Help from Beej's Guide to Sockets

void Server::ProcessMessage(const char* buffer)
{
	//std::cout << "RECEIEVED MESSAGE" << std::endl;

	//Reconstruct Message
	std::string b(buffer);
	std::istringstream iss(b);

	std::string kind;
	std::string path;
	std::string visited;

	std::getline(iss, kind);
	std::getline(iss, path);
	std::getline(iss, visited);

	std::istringstream iss_path(path);
	std::istringstream iss_visited(visited);
	std::vector<std::string> path_tokens{std::istream_iterator<std::string>{iss_path},std::istream_iterator<std::string>{}};
	std::vector<std::string> visited_tokens{std::istream_iterator<std::string>{iss_visited},std::istream_iterator<std::string>{}};

	Message msg(kind);

	for (std::vector<std::string>::iterator it = path_tokens.begin() + 1; it != path_tokens.end(); it++)
	{
		msg.path.emplace_back(std::stoi(*it));
	}

	for (std::vector<std::string>::iterator it = visited_tokens.begin() + 1; it != visited_tokens.end(); it++)
	{
		msg.visited.emplace_back(std::stoi(*it));
	}

	//std::cout << msg;
	
//	std::cout << "KIND : " << kind << std::endl;
//	std::cout << "KIND : " << kind[0] << std::endl;

	// IF THE MESSAGE IS AN INBOUND MESSAGE SPECIAL LOGIC
	if (msg.kind == "inbound")
	{
		//std::cout << msg << std::endl;
		// Back to original Sender
		//std::cout << "Serv_node_id " << serv.node_id << " " << "og sender " << msg.path[0] << std::endl;
		if (serv.node_id == msg.path[0])
		{
			// Print one hop neighbors based on information
			//std::cout << msg.path.size() - 1 << " " << msg.path.end()[-1] << std::endl;
			std::cout << msg.path.size() - 1 << " hop neighbor " << msg.path.end()[-1] << std::endl;
			//k_hop.emplace_back(std::to_string(msg.path.size() -1));
			//k_hop.emplace_back(std::to_string(msg.path.end()[-1]));
		}	
		else  // Relay Back to Original 
		{
			// The return node id is one less than the serv_node_id
			int ret_node_id = -1;
			for (std::vector<int>::iterator it = msg.path.begin(); it != msg.path.end(); it++)
			{
				if ((*it) == serv.node_id)
				{
					ret_node_id = *(it - 1);
				}
			}

			//std::cout << "Return Node Id: " << ret_node_id << std::endl;
			for (const auto& one_hop: serv.one_hop_neighbors)
			{
				if(ret_node_id == one_hop.node_id)
				{
					//std::cout << "Relaying" << std::endl;
					Message ret_msg = msg;
					ret_msg.kind = "inbound";
					Client ret(serv, one_hop);
					ret.SendMessage(ret_msg);
				}
			}
		}
	}
	// ELSE IF THE MESSAGE WAS OUTBOUND
	else
	{
		//Add current node to path
		msg.path.emplace_back(serv.node_id);

		// It would be the path
		// Need to check size of path or have some way to know you are back to start
		
		int ret_node_id = msg.path.end()[-2];
		for (const auto& one_hop: serv.one_hop_neighbors)
		{
			if(ret_node_id == one_hop.node_id)
			{
				Message ret_msg = msg;
				ret_msg.kind = "inbound";
				Client ret(serv, one_hop);
				ret.SendMessage(ret_msg);
			}
		}
		
		//Client c1(serv, one_hop);
		//c1.SendMessage(msg);
		
		// Copy nodes visisted before
		std::vector<int> pre_visited = msg.visited;
		// Add current node and one hop neighbors to visited
		msg.visited.emplace_back(serv.node_id);

		for (const auto& one_hop: serv.one_hop_neighbors)
		{
			msg.visited.emplace_back(one_hop.node_id);
			// I'm going to allow duplicates for now
		}
		// Send reply message back to original sender

		// Send message to one hop neighbors
		for (const auto& one_hop: serv.one_hop_neighbors)
		{
			// If the node hasn't been visisted 	
			auto result1 = std::find(std::begin(pre_visited), std::end(pre_visited), one_hop.node_id);
			if (result1 == std::end(pre_visited)) 
			{
				Client c1(serv, one_hop);
				c1.SendMessage(msg);
			}
		}
	}

	//for (const auto& one_hop: serv.one_hop_neighbors)
	//{
	//	if (num_nodes - 1 > hop_number)
	//	{
	//		Client c1(serv, one_hop);
	//		//std::cout << "one hop " << one_hop.node_id << std::endl;
	//		//c1.Message_o(original_sender, hop_number + 1, num_nodes);
	//	}
	//}

//	int original_sender = std::stoi(tokens[0]);
//	int src_node_id = std::stoi(tokens[1]);
//	int dest_node_id = std::stoi(tokens[2]);
//	int hop_number = std::stoi(tokens[3]);
//	int num_nodes = std::stoi(tokens[4]);
	//printf("%d %d %d %d\n", original_sender, src_node_id, dest_node_id, hop_number);
	
	// If the hop_number is equal to the number of nodes -1 then you are going to send a return message in this case lets just have it stop first

}

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

	//printf("server: waiting for connections...\n");

	char buffer[1024];

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
            memset(buffer, 0, 1024);
            int read_rtn = read(newsockfd, buffer, 1023);
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

