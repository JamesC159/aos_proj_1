#ifndef PARSER_H
#define PARSER_H

#include "node.h"
#include <string>
#include <cstddef>
#include <fstream>
#include <unordered_map>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>

class Parser
{
	private:

	public:
		int num_nodes;
		int line_num; 
		//std::ifstream in;
		std::string line;
		std::string config;
		//std::vector<Node> nodes;
		std::unordered_map<int, Node> node_map;
		Parser(const std::string config_file);
        void Parse_Config();
		bool Is_Valid_Line(std::string line);
};

#endif // PARSER_H
