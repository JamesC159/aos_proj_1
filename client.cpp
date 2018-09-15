#include "client.h"

Client::Client(const Node& src, const Node& dest)
//int Client(const Node& n, int sender_node_id)
{
	// Lets split up functionality
	// Send message to one hop neighbors 
	sleep(5); // To let all servers get setup
	this -> dest = dest;
	this -> src = src;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	std::cout << "Attempting to connect" << dest.hostname << " " <<  dest.port << std::endl;

	if ((rv = getaddrinfo(dest.hostname.c_str(), dest.port.c_str(), &hints, &servinfo)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		error_num = 1;
		exit(1);
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
		{
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			perror("client: connect");
			close(sockfd);
			continue;
		}
		break;
	}

	if (p == NULL) 
	{
		fprintf(stderr, "client: failed to connect\n");
		error_num = 2;
		exit(2);
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	//if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) 
	//{
	//	perror("recv");
	//	exit(1);
	//}

}

int Client::Message(int hop_number)
{
		// Let's have it send a message
		std::cout << "Sending Message" << std::endl;

		// What is the message that you want to send?
		// Node_id of sender 
		// Node_id of reciver
		// Hop number
		// It's probably going to make sense to send this as a struct of serialized data and unpack it

		char buffer[256]; 
		sprintf(buffer, "Sending from node %d to node %d hop number %d", src.node_id, dest.node_id, hop_number);

        int msg_rtn = write(sockfd,buffer,strlen(buffer)); // Send the discovery message to neighbors 

		memset(buffer, 0, 256); // reset buffer

		// Return int so you could get error code
		return 0;
}

int Client::Close()
{
		close(sockfd);

		return 0;
}


void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
