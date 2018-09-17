#include "message.h"

std::ostream &operator<<(std::ostream &os, Message const &m)
{
	std::cout << "PATH ";
	for (const auto& i: m.path)
	{
		std::cout << i << " "; 	
	}
	std::cout << std::endl;
	std::cout << "VISITED ";
	for (const auto& i: m.visited)
	{
		std::cout << i << " "; 	
	}
	std::cout << std::endl;
	
	return os;
}
