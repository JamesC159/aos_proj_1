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

	sleep(3); // To let all servers get setup
	
	Message out;
	out.path.emplace_back(process_node.node_id);
	// Add current node and one hop neighbors to visited
	out.visited.emplace_back(process_node.node_id);

	for (const auto& one_hop: process_node.one_hop_neighbors)
	{
		out.visited.emplace_back(one_hop.node_id);
	}

//	if (process_node.node_id == 4)
//	{
//		for (const auto& one_hop: process_node.one_hop_neighbors)
//		{
//			Message out_hop = out;
//			Client c1(process_node, one_hop);
//			c1.SendMessage(out_hop);
//
//			// Should I close the socket? Should I multi-thread this?
//		}
//	}
	//std::stringstream ss;

    //change the underlying buffer and save the old buffer
    //auto old_buf = std::cout.rdbuf(ss.rdbuf()); 

	// Send message to one hop neighbors
	for (const auto& one_hop: process_node.one_hop_neighbors)
	{
		Message out_hop = out;
		Client c1(process_node, one_hop);
		c1.SendMessage(out_hop);

		// Should I close the socket? Should I multi-thread this?
	}

//	std::string line;
//	
//    std::vector<int> k_hop(p1.num_nodes, 0);
//
//	while (std::getline(ss, line))
//	{
//			std::istringstream ss(line);
//			std::vector<std::string> tokens{std::istream_iterator<std::string>{ss},std::istream_iterator<std::string>{}};
//			int hop_number = std::stoi(tokens[0]);
//			int node_id = std::stoi(tokens[1]);
//			int current_hop_num = k_hop[node_id];
//			k_hop[node_id] = std::min(current_hop_num, hop_number);
//	}
//	
//	// I need to parse the file
//    std::cout.rdbuf(old_buf); //reset
//
//	for (int i = 0; i < p1.num_nodes; i++)
//	{
//		if (k_hop[i] > 0)
//		{
//			std::cout << i << " is " << k_hop[i] << " neighbor" << std::endl;
//		}
//
//	}
//

	
	t1.join();
	// How do I know when to stop everything?
}

