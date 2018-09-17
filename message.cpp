#include "message.h"

Message::Message()
{
	this -> kind = "outbound";
}

Message::Message(std::string kind)
{
	this -> kind = kind;
}

std::ostream &operator<<(std::ostream &os, Message const &m)
{
	std::cout << m.kind << std::endl;
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
