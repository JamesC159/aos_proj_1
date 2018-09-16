#include "node.h"
#include "client.h"
#include "parser.h"
#include "server.h"

#include <algorithm>
#include <cctype> 
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <thread>
#include <vector>

// Break the problem into pieces
// After setting up the topology you need a distributed algorithm that finds the k hop neights (communication thorugh sockets and 

int main(int argc, char** argv)
{
	if(argc != 3)
	{
		std::cerr << "usage: ./main config node_id" << std::endl; 
	 	return -1;
	}

	Parser p1(argv[1]);
	p1.Parse_Config();
	// Print nodes for debug
	//for (const auto& n: p1.node_map)
	//{
	//	std::cout << n.second << std::endl;
	//}

	Node process_node = p1.node_map[std::stoi(argv[2])];
	std::cout << "node_id_process " << process_node.node_id << std::endl;

	// I think you need this to be a multithreaded program
	// You want the server running at the same time the client is going
	// The server is going to be stuck in that loop and so the client code won't be reached
	// Server
	Server.num_nodes = 
	Server s1(p1.node_map[node_id_process]);
	std::thread t1(&Server::Listen, s1);

	// For one hop neighbors
	// Send message to one hop neighbors 
	
	// You might want to set up all client connections first then message after
	// You need some kind of handler to process receive to send
	// How do you get the information back to the original sender?

	// Test with discovering node 0 neighbors first then expand this logic
	if (process_node.node_id == 0)
	{
		for (const auto& one_hop: process_node.one_hop_neighbors)
		{
			Client c1(process_node, one_hop);
			std::cout << "one hop " << one_hop.node_id << std::endl;
			c1.Message(1);
			//c1.Close();
			//std::thread t2(Client, p1.node_map[one_hop]);
			//t2.detach();
		}
	}
	
	t1.join();
	//t1.detach();
}

