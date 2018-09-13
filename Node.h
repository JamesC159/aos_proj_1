#ifndef NODE_H
#define NODE_H

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

class Node
{
	private:
		int node_id;
		std::string hostname;
		int port;
		std::vector <int> one_hop_neighbors;	
	public:
		Node();
		Node(int node_id, std::string hostname, int port);
		void Add_One_Hop_Neighbor(int neighbor);
		friend std::ostream &operator<<(std::ostream &os, Node const &n); 
};

#endif // NODE_H
