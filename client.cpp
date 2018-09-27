#include "client.h"

//#define MAXLSEEP 128
//
//int connect_retry(int sockfd, const structu sockaddr* addr, socklen_t alen)
//{
//	int numsec;
//
//	for (numsec = 1; numsec <= MAXSLEEP; numsec <<=1)
//	{
//		if (connect(sockfd, addr, alen) == 0)
//		{
//			return (0)
//		}
//		if (numsec <= MAXSLEEP / 2)
//		{
//			sleep(numsec)
//		}
//	}
//	return (-1);
//}

Client::Client(const Node& src, const Node& dest)
{
//int Client(const Node& n, int sender_node_id) {
	// Lets split up functionality
	// Send message to one hop neighbors 
	this -> dest = dest;
	this -> src = src;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	//std::cout << "Attempting to connect" << dest.hostname << " " <<  dest.port << std::endl;

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

		while(connect(sockfd, p->ai_addr, p->ai_addrlen) != 0)
		{
			// Loop for connection 
			// There has to be a better way
		}

		//if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		//{
		//	perror("client: connect");
		//	close(sockfd);
		//	continue;
		//}
		break;
	}

	if (p == NULL) 
	{
		fprintf(stderr, "client: failed to connect\n");
		error_num = 2;
		exit(2);
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
	//printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	//if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) 
	//{
	//	perror("recv");
	//	exit(1);
	//}

}

int Client::SendMessage(Message out)
{
	std::stringstream ss;
    //change the underlying buffer and save the old buffer
    auto old_buf = std::cout.rdbuf(ss.rdbuf()); 
	std::cout << out;
    std::cout.rdbuf(old_buf); //reset

	const std::string& tmp = ss.str();
	const char* msg = tmp.c_str();
	
	// Size of buffer should really be size of msg + 1 
	char buffer[1024]; 
    strcpy(buffer, msg);  

//	std::cout << "Send Message" << std::endl;
//	printf("%s", buffer);

	int msg_rtn = write(sockfd,buffer,strlen(buffer)); // Send the message to neighbors 
	memset(buffer, 0, 1024); // reset buffer

	return 0;  // Return int so you could get error code
	// How do I serialize the structure to pass it through the TCP socket?
}

int Client::Close()
{
		close(sockfd);

		return 0;
}


void* Client::get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
