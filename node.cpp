#include "node.h"
// Read about initializer_lists

Node::Node()
{}

Node::Node(int node_id, std::string hostname, std::string port)
{
	this -> node_id = node_id;
	this -> hostname = hostname;
	this -> port = port;
}

void Node::Add_One_Hop_Neighbor(const Node& neighbor)
{
	one_hop_neighbors.emplace_back(neighbor);
}

std::ostream & operator<<(std::ostream &os, Node const &n)
{
	std::cout << "NODE : ID = " << n.node_id << " HOSTNAME = " << n.hostname << " PORT = " << n.port;
	std::cout << " ONE-HOP-NEIGHBORS = ";
	for (const auto& i: n.one_hop_neighbors)
	{
		std::cout << i.node_id  << " ";
	}
	return os;
}
