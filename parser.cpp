#include "parser.h"

// Valid lines are only ones starting with unsigned integers
// Valid lines can have any number of white space between objects
// Allow for comments (#) on valid lines

// 2n + 1 valid lines
// The first valid line is the number of nodes 
// The next n valid lines have 3 tokens
// The last n valid lines have variable number of tokens

Parser::Parser(const std::string config_file)
{
	config = config_file;
	line_num = 0;
	num_nodes = 0;
}

void Parser::Parse_Config()
{
	std::ifstream in(config);
	//in = std::ifstream(config_file);
	while (std::getline(in, line))
	{
		if (Is_Valid_Line(line))
		{
			// FirstLine
			if (line_num == 0)
			{
				num_nodes = std::stoi(line);	
			}
			// First n
			else if (line_num < num_nodes + 1)
			{
				std::istringstream iss(line);
				std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},std::istream_iterator<std::string>{}};
				node_map[std::stoi(tokens[0])] = Node(std::stoi(tokens[0]), tokens[1], tokens[2]);
			}
			else //Second n  
			{
				std::istringstream iss(line);
				std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},std::istream_iterator<std::string>{}};

				for (std::vector<std::string>::iterator it = tokens.begin() + 1; it != tokens.end(); it++)
				{
					if (((*it).find('#') != std::string::npos))
					{
						break;
					}
					//Node one_hop = node_map[std::stoi(*it)];
					node_map[std::stoi(tokens[0])].Add_One_Hop_Neighbor(node_map[std::stoi(*it)]);
				}
			}
			
			line_num++;
		}
	}
}
bool Parser::Is_Valid_Line(std::string line)
{
	// To verify this you really want to check the first token is an unsigned int
	// Checking the first character won't work if there is whitespace
	// Check if the first non whitespace character is a digit is fairly safe but imagine a situation where after the digit you had other nonsense
	// The safest way is to extract the first token delimited by whitespace Iterate throgh all the characters of the token and verify each one is a digit
	std::istringstream iss(line);
	std::string first;
	iss >> first;
	if (first.empty())
	{
		return false;
	}
	for (auto it: first)
	{
		if (!isdigit(it))
		{
			return false;
		}
	}
	return true;
}


// Print nodes for debug
//for (const auto& n: p1.node_map)
//{
//	std::cout << n.second << std::endl;
//}
