#include "node.h"
#include "parser.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>

// Break the problem into pieces
// The first step is to create a parser that parses the input data

// After setting up the topology you need a distributed algorithm that finds the k hop neights (communication thorugh sockets and 

// What data structures are we going to work with?
//using namespace std;

int main(int argc, char** argv)
{
	// Run the program with three arguments
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
}

