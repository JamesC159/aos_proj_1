#include "node.h"
#include "client.h"
#include "message.h"
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
// After setting up the topology you need a distributed algorithm that finds the k hop neights (communication through sockets)

int main(int argc, char** argv)
{
	if(argc != 3)
	{
		std::cerr << "usage: ./main config node_id" << std::endl; 
	 	return -1;
	}

	Parser p1(argv[1]);
	p1.Parse_Config();

	Node process_node = p1.node_map[std::stoi(argv[2])];
	std::cout << "node_id_process " << process_node.node_id << std::endl;

	Server s1(process_node);
	std::thread t1(&Server::Listen, s1);

	// How do you get the information back to the original sender?
	
	// Test with discovering node 0 neighbors first then expand this logic
	// For one hop neighbors
	// Send message to one hop neighbors 
	
	sleep(3); // To let all servers get setup
	
	Message out;
	out.path.emplace_back(process_node.node_id);
	//Visited should really be a hash table
	
	// Add current node and one hop neighbors to visited
	out.visited.emplace_back(process_node.node_id);

	for (const auto& one_hop: process_node.one_hop_neighbors)
	{
		out.visited.emplace_back(one_hop.node_id);
	}

	// Send message to one hop neighbors
	for (const auto& one_hop: process_node.one_hop_neighbors)
	{
		Message out_hop = out;
		Client c1(process_node, one_hop);
		c1.SendMessage(out_hop);

		// Should I close the socket? Should I multi-thread this?
	}

//	if (process_node.node_id == 0)
//	{
//		Message out;
//		out.path.emplace_back(process_node.node_id);
//		//Visited should really be a hash table
//		
//		// Add current node and one hop neighbors to visited
//		out.visited.emplace_back(process_node.node_id);
//
//		for (const auto& one_hop: process_node.one_hop_neighbors)
//		{
//			out.visited.emplace_back(one_hop.node_id);
//		}
//
//		// Send message to one hop neighbors
//		for (const auto& one_hop: process_node.one_hop_neighbors)
//		{
//			Message out_hop = out;
//			Client c1(process_node, one_hop);
//			c1.SendMessage(out_hop);
//
//			// Should I close the socket? Should I multi-thread this?
//		}
//	}
	
	t1.join();
}

